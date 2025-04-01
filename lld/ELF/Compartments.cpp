//===- Compartments.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains support for sub-object compartments.
//
//===----------------------------------------------------------------------===//

#include "Compartments.h"
#include "Config.h"
#include "InputFiles.h"
#include "SyntheticSections.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/GlobPattern.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "Arch/Cheri.h"

#include <list>
#include <set>

using namespace llvm;
using namespace llvm::object;
using namespace lld;
using namespace lld::elf;

// ELF relocations describe inter-section dependencies where the input
// section S1 containing the relocation address depends on the input
// section S2 that defines the target symbol.
//
// SectionDepends is a map of each S2 section that holds a logical
// list of unique <S1, RelExpr> tuples that target S2.  This
// list is actually stored as a map where each tuple is mapped to the
// first Symbol targeted by such a tuple.  This permits using the
// Symbol's name in diagnostic messages.
typedef std::pair<InputSectionBase *, RelExpr> SectionKey;
typedef std::unordered_map<SectionKey, Symbol &> SectionDeps;
typedef std::map<InputSectionBase *, SectionDeps> SectionDepends;

// CompartmentDeps is constructed by walking a single SectionDeps
// collapsing entries from multiple <section, relocation, symbol>
// tuples down to a single representative <section, symbol> pair from
// each Compartment.
typedef std::pair<const InputSectionBase *, const Symbol *> CompartmentValue;
typedef std::map<Compartment *, CompartmentValue> CompartmentDeps;

namespace std {
  template <> struct less<Compartment *> {
    bool operator()(const Compartment *lhs, const Compartment *rhs) const {
      if (lhs == nullptr)
        return (true);
      if (rhs == nullptr)
        return (false);

      return (lhs->name.compare(rhs->name) < 0);
    }
  };

  template <> struct less<InputSectionBase *> {
    bool operator()(const InputSectionBase *lhs,
                    const InputSectionBase *rhs) const {
      if (lhs->file != nullptr || rhs->file != nullptr) {
        if (lhs->file == nullptr)
          return (true);
        if (rhs->file == nullptr)
          return (false);

        int rval = lhs->file->getName().compare(rhs->file->getName());
        if (rval < 0)
          return (true);
        if (rval > 0)
          return (false);
      }

      return (lhs->name.compare(rhs->name) < 0);
    }
  };

  template <> struct hash<SectionKey> {
    size_t operator()(SectionKey A) const {
      size_t h1 = std::hash<InputSectionBase *>{}(A.first);
      size_t h2 = std::hash<RelExpr>{}(A.second);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace lld {
namespace elf {

class CompartmentLookup {
public:
  bool addPattern(StringRef glob, Compartment *c)
  {
    Expected<GlobPattern> GlobOrErr = GlobPattern::create(glob);
    if (!GlobOrErr) {
      error(toString(GlobOrErr.takeError()));
      return false;
    }
    matches.emplace_back(std::make_pair(std::move(*GlobOrErr), c));
    return true;
  }

  Expected<Compartment *> findMatch(StringRef name) const
  {
    Compartment *current = nullptr;

    for (const auto &p : matches) {
      if (!p.first.match(name))
        continue;

      if (current == p.second)
        continue;
      if (current != nullptr)
        return make_error<StringError>("assigned to compartment " +
                                       current->name + " and " +
                                       p.second->name, errc::invalid_argument);
      current = p.second;
    }
    return current;
  }

private:
  std::vector<std::pair<GlobPattern, Compartment *> > matches;
};

// A disjoint set union of input sections is implemented by having a
// std::unordered_map<> map input section pointers to a parent input section.  A
// std::map<> maps tree root input sections to a list of all set members.
typedef std::list<InputSectionBase *> SectionList;

class SectionDSU {
  typedef std::map<InputSectionBase *, SectionList> setMap;
public:

  class iterator {
  public:
    explicit iterator(setMap &sets, bool start)
    {
      end = sets.end();
      if (start) {
        it = next(sets.begin());
      } else {
        it = end;
      }
    }

    iterator & operator++() { it = next(++it); return *this; }
    iterator operator++(int)
    { iterator retval = *this; ++(*this); return retval; }
    bool operator==(iterator other) const { return it == other.it; }
    bool operator!=(iterator other) const { return it != other.it; }
    SectionList & operator*() const { return it->second; }

  private:
    setMap::iterator next(setMap::iterator it)
    {
      while (it != end && it->second.empty())
        it++;
      return it;
    }

    setMap::iterator it;
    setMap::iterator end;
  };

  iterator begin() { return iterator(sets, true); }
  iterator end() { return iterator(sets, false); }

  void makeSet(InputSectionBase *s)
  {
    parents.emplace(s, s);
    sets.emplace(s, std::initializer_list<InputSectionBase *>({ s }));
  }

  InputSectionBase *findSet(InputSectionBase *s)
  {
    if (parents[s] == s)
      return s;
    return parents[s] = findSet(parents[s]);
  }

  void unionSets(InputSectionBase *a, InputSectionBase *b)
  {
    a = findSet(a);
    b = findSet(b);
    if (a == b)
      return;

    if (std::less<InputSectionBase *>()(b, a))
      std::swap(a, b);

    SectionList &aList = sets[a];
    SectionList &bList = sets[b];
    aList.splice(aList.end(), bList);
    parents[b] = a;
  }

private:
  std::unordered_map<InputSectionBase *, InputSectionBase *> parents;
  setMap sets;
};

void addCompartment(StringRef name) {
    compartments.emplace_back();
    Compartment &newCompart = compartments.back();
    newCompart.name = name;
    newCompart.index = compartments.size();
}

Compartment *findCompartment(StringRef name) {
  if (name.empty())
    return nullptr;
  for (Compartment &compart : compartments) {
    if (compart.name.compare(name) == 0)
      return &compart;
  }
  return nullptr;
}

static bool fromJSON(const json::Value &E, CompartmentMembers &Out,
                     json::Path P) {
  llvm::json::ObjectMapper O(E, P);
  return O && O.mapOptional("symbols", Out.symbols) &&
    O.mapOptional("files", Out.files);
}

static bool fromJSON(const json::Value &E, CompartmentPolicy &Out,
                     json::Path P) {
  llvm::json::ObjectMapper O(E, P);
  return O && O.map("compartments", Out.compartments);
}

void readCompartmentPolicy(MemoryBufferRef mb) {
  Expected<CompartmentPolicy> policy =
    json::parse<CompartmentPolicy>(mb.getBuffer());
  if (!policy)
    fatal(Twine("failed to parse compartmentalization policy \"") +
          mb.getBufferIdentifier() + Twine("\": ") +
          toString(policy.takeError()));

  for (const auto &kv : policy->compartments) {
    if (findCompartment(kv.first) != nullptr)
      continue;
    addCompartment(kv.first);
  }

  config->compartmentPolicies.emplace_back(std::move(*policy));
}

static std::string compartmentName(const Compartment *c)
{
  if (c == nullptr)
    return "the default compartment";
  else
    return "compartment " + toString(c->name);
}

static std::string isecName(const InputSectionBase *s)
{
  return "input section " + toString(s->file->getName()) + ":" +
      toString(s->name);
}

static std::string symbolName(const Symbol *sym)
{
  if (sym->getName().empty())
    return "local symbol";
  else
    return "symbol " + toString(sym->getName());
}

static Compartment *compartmentForSection(const InputSectionBase *s,
                                          const CompartmentLookup &symbolLookup)
{
  if (s->file == nullptr)
    return nullptr;

  InputFile *f = s->file;
  Symbol *firstSym = nullptr;
  Compartment *current = nullptr;
  bool hasError = false;
  std::vector<Symbol *> implicit_symbols;
  for (Symbol *b : f->getSymbols()) {
    if (Defined *d = dyn_cast<Defined>(b)) {
      if (d->section != s)
        continue;

      if (b->getName().empty())
        continue;

      Expected<Compartment *> symbolC = symbolLookup.findMatch(b->getName());
      if (!symbolC) {
        error("symbol " + b->getName() + " " + toString(symbolC.takeError()));
        hasError = true;
        continue;
      }

      if (*symbolC == current)
        continue;

      // Allow unspecified symbols to follow other rules for their containing
      // section.
      if (*symbolC == nullptr) {
        implicit_symbols.emplace_back(d);
        continue;
      }

      if (current != nullptr) {
        error(isecName(s) + " assigned to " + compartmentName(current) +
              " by " + symbolName(firstSym) + " and " +
              compartmentName(*symbolC) + " by " + symbolName(b));
        hasError = true;
      } else {
        current = *symbolC;
        firstSym = b;
      }
    }
  }
  if (hasError)
    exitLld(1);

  // Finally, if a compartment was assigned by a subset of symbols, note any
  // symbols that inherited this compartment.
  if (config->verboseCompartmentalization && current != nullptr) {
    for (Symbol *b : implicit_symbols) {
      if (!b->includeInDynsym())
        continue;
      message("note: " + symbolName(b) + " implicitly assigned to " +
              compartmentName(current));
    }
  }

  return current;
}

// Returns true for sections that can be assigned to a compartment
static bool canCompartmentalize(const InputSectionBase *s) {
  return s->name.startswith(".text") || s->name.startswith(".rodata") ||
    s->name.startswith(".data") || s->name.startswith(".bss");
}

// Returns a pointer to an exported symbol if a section contains exported
// symbols.  This is not perfect as it is called before relocations are scanned.
static Symbol *firstExportedSymbol(const InputSectionBase *s) {
  InputFile *f = s->file;
  for (Symbol *b : f->getSymbols()) {
    if (Defined *d = dyn_cast<Defined>(b)) {
      if (d->section != s)
        continue;

      if (!b->includeInDynsym())
        continue;

      return b;
    }
  }
  return nullptr;
}

static void checkDefaultCompartment()
{
  bool valid = true;
  if (config->noDefaultCompartment) {
    for (InputSectionBase *s : ctx.inputSections) {
      if (s->file == nullptr || (s->flags & ELF::SHF_ALLOC) == 0)
        continue;

      if (!canCompartmentalize(s))
        continue;

      if (s->compartment == nullptr) {
        error(isecName(s) + " not assigned to a compartment");
        valid = false;
      }
    }
    if (!valid)
      exitLld(1);
  }
}

template <class ELFT, class RelTy>
static InputSectionBase *relocationTargetSection(InputSectionBase *sec,
                                                 const RelTy &rel) {
  uint32_t symIndex = rel.getSymbol(config->isMips64EL);
  Symbol &sym = sec->getFile<ELFT>()->getSymbol(symIndex);

  if (sym.isUndefined())
    return nullptr;

  Defined *d = dyn_cast<Defined>(&sym);
  if (d == nullptr)
    return nullptr;

  return static_cast<InputSectionBase *>(d->section);
}

template <class ELFT, class RelTy>
static void scanRelocations(InputSectionBase *sec, ArrayRef<RelTy> rels,
                            SectionDepends &depends) {
  for (const auto &rel : rels) {
    InputSectionBase *tsec = relocationTargetSection<ELFT>(sec, rel);

    if (tsec == nullptr)
      continue;

    // Ignore self-dependencies.
    if (tsec == sec)
      continue;

    uint64_t offset = rel.r_offset;
    if (offset == uint64_t(-1))
      continue;

    RelType type = rel.getType(config->isMips64EL);
    uint32_t symIndex = rel.getSymbol(config->isMips64EL);
    Symbol &sym = sec->getFile<ELFT>()->getSymbol(symIndex);
    const uint8_t *loc = sec->content().begin() + offset;
    RelExpr expr = target->getRelExpr(type, sym, loc);
    if (expr == R_NONE)
      continue;

    SectionDeps &sdeps = depends[tsec];
    SectionKey key(sec, expr);
    if (sdeps.count(key) == 0) {
      sdeps.emplace(key, sym);
    }
  }
}

template <class ELFT>
static void scanRelocations(InputSectionBase *sec, SectionDepends &depends) {
  const RelsOrRelas<ELFT> rels = sec->template relsOrRelas<ELFT>();
  if (rels.areRelocsRel())
    scanRelocations<ELFT>(sec, rels.rels, depends);
  else
    scanRelocations<ELFT>(sec, rels.relas, depends);
}

void assignSectionsToCompartments() {
  if (compartments.empty()) {
    checkDefaultCompartment();
    return;
  }

  for (Compartment &c : compartments) {
    c.suffix = "." + c.name.str();
  }

  bool valid = true;
  CompartmentLookup fileLookup;
  CompartmentLookup symbolLookup;
  for (const auto &policy : config->compartmentPolicies) {
    for (const auto &kv : policy.compartments) {
      Compartment *c = findCompartment(kv.first);
      for (const auto &glob : kv.second.files) {
        valid &= fileLookup.addPattern(glob, c);
      }
      for (const auto &glob : kv.second.symbols) {
        valid &= symbolLookup.addPattern(glob, c);
      }
    }
  }
  if (!valid)
    exitLld(1);

  // First, bind input object files to compartments using file patterns.  Note
  // that there is no global list of input files, so have to walk input sections
  // instead.
  InputFile *lastFile = nullptr;
  for (InputSectionBase *s : ctx.inputSections) {
    if (s->file == nullptr || s->file == lastFile)
      continue;
    lastFile = s->file;

    // Compartment specified via --compartment=
    if (s->file->compartment != nullptr)
      continue;

    StringRef path = s->file->getName();
    Expected<Compartment *> c = fileLookup.findMatch(path);
    if (!c) {
      error("input file " + path + " " + toString(c.takeError()));
      valid = false;
      continue;
    }

    // If the path is not a basename, try to match on the basename.
    if (*c == nullptr && llvm::sys::path::has_parent_path(path)) {
      c = fileLookup.findMatch(llvm::sys::path::filename(path));
      if (!c) {
        error("input file " + path + " " + toString(c.takeError()));
        valid = false;
        continue;
      }
    }

    if (*c == nullptr)
      continue;

    s->file->compartment = *c;
    if (config->verboseCompartmentalization)
      message("info: input file " + s->file->getName() + " matched to " +
              compartmentName(*c));
  }

  // Second, make a first pass assigning input sections to compartments based on
  // symbol patterns.  This also reports errors for conflicting assignments.
  for (InputSectionBase *s : ctx.inputSections) {
    if (!canCompartmentalize(s)) {
      continue;
    }

    Compartment *c = compartmentForSection(s, symbolLookup);
    if (s->file != nullptr && s->file->compartment != nullptr) {
      if (c == nullptr)
        c = s->file->compartment;
      else if (s->file->compartment != c) {
        error("input file assigned to " +
              compartmentName(s->file->compartment) +
              " but symbols in section " + s->name + " assigned to " +
              compartmentName(c));
        valid = false;
        continue;
      }
    }
    if (c == nullptr)
      continue;

    if (config->verboseCompartmentalization)
      message("info: " + isecName(s) + " assigned to " + compartmentName(c));
    s->compartment = c;
  }
  if (!valid)
    exitLld(1);

  // Third, build table of input section dependencies and add initial sets to
  // DSU used in fifth step.
  SectionDepends depends;
  SectionDSU dsu;
  for (InputSectionBase *s : ctx.inputSections) {
    if (s->file == nullptr || (s->flags & ELF::SHF_ALLOC) == 0)
      continue;

    if (s->name == ".eh_frame")
      continue;

    invokeELFT(scanRelocations, s, depends);
    dsu.makeSet(s);
  }

  // Fourth, for purecap CHERI, look for "hard" dependencies where an
  // input section is required to be contained within the PCC bounds
  // of another compartmentalized section.  These dependencies should
  // always be from .text sections against some other section.  This
  // repeats until no new sections are assigned.
  if (config->isCheriAbi) {
    for (;;) {
      bool assigned = false;
      for (const auto &kv : depends) {
        InputSectionBase *s2 = kv.first;
        const SectionDeps &sdeps = kv.second;

        CompartmentDeps cdeps;
        for (const auto &skv : sdeps) {
          RelExpr expr = skv.first.second;
          switch (expr) {
          case R_PC:
          case R_AARCH64_PAGE_PC:
          case R_RISCV_PC_INDIRECT:
            break;
          default:
            continue;
          }

          const InputSectionBase *s1 = skv.first.first;
          Compartment *c = s1->compartment;
          if (cdeps.count(c) == 0)
            cdeps[c] = std::make_pair(s1, &skv.second);
        }
        if (cdeps.size() == 0)
          continue;

        if (cdeps.size() > 1) {
          std::string clist;
          for (const auto &cdep : cdeps) {
            Compartment *c = cdep.first;
            const auto pair = cdep.second;
            const InputSectionBase *s1 = pair.first;
            const Symbol *sym = pair.second;

            clist += "\n\t" + toString(compartmentName(c)) +
              " directly accesses " + toString(symbolName(sym)) + " from " +
              toString(isecName(s1));
          }
          error(isecName(s2) + " required by multiple compartments:" +
                clist);
          valid = false;
          continue;
        }

        Compartment *c = cdeps.begin()->first;
        if (c == s2->compartment)
          continue;

        const auto pair = cdeps.begin()->second;
        const InputSectionBase *s1 = pair.first;
        const Symbol *sym = pair.second;

        if (s2->compartment != nullptr) {
          error(symbolName(sym) + " assigned to " +
                compartmentName(s2->compartment) +
                " is directly accessed by " + compartmentName(c) + " in " +
                isecName(s1));
          valid = false;
          continue;
        }

        if (!canCompartmentalize(s2)) {
          error(isecName(s2) + " containing " + symbolName(sym) +
                " directly accessed by " + compartmentName(c) + " in " +
                isecName(s1) + " cannot be compartmentalized");
          valid = false;
          continue;
        }

        Symbol *exported = firstExportedSymbol(s2);
        if (exported != nullptr) {
          error(isecName(s2) + " containing " + symbolName(sym) +
                " directly accessed by " + compartmentName(c) + " in " +
                isecName(s1) + " cannot be assigned due to exported " +
                symbolName(exported));
          valid = false;
          continue;
        }

        // Report multiple errors before exiting
        if (!valid)
          continue;

        if (config->verboseCompartmentalization)
          message("info: " + isecName(s2) + " assigned to implied " +
                  compartmentName(c) + " due to direct access of " +
                  symbolName(sym) + " in " + isecName(s1));
        s2->compartment = c;
        assigned = true;
      }
      if (!valid)
        exitLld(1);
      if (!assigned)
        break;
    }
  }

  // Fifth, look for compartmentalizable sections not yet assigned to a
  // compartment that can be compartmentalized.  Start by using a disjoint set
  // union pattern to build sets of related input sections.  Note that edges
  // between sections that are already compartmentalized are ignored.  The goal
  // is to find sets of sections in the default compartment that are only used
  // by a single compartment.
  for (const auto &kv : depends) {
    InputSectionBase *s2 = kv.first;
    const SectionDeps &sdeps = kv.second;
    for (const auto &skv : sdeps) {
      InputSectionBase *s1 = skv.first.first;

      if (s1->compartment != nullptr && s2->compartment != nullptr)
        continue;
      dsu.unionSets(s2, s1);
    }
  }

  for (SectionList &list : dsu) {
    // Ignore singleton sets.
    if (list.size() == 1) {
      if (config->verboseCompartmentalization) {
        InputSectionBase *s = list.front();
        if (s->compartment == nullptr && canCompartmentalize(s)) {
          message("info: potential compartment:");
          message("\t" + isecName(s));
        }
      }
      continue;
    }

    if (config->verboseCompartmentalization) {
      std::set<InputSectionBase *> set;
      for (InputSectionBase *s : list)
        set.insert(s);
      list.clear();
      for (InputSectionBase *s : set)
        list.push_back(s);
    }

    // More than one compartment?
    Compartment *c = nullptr;
    bool multiple = false;
    bool anyDefault = false;
    for (InputSectionBase *s : list) {
      if (!canCompartmentalize(s))
        continue;
      if (s->compartment == nullptr) {
        anyDefault = true;
        continue;
      }
      if (c == nullptr) {
        c = s->compartment;
        continue;
      }
      if (c == s->compartment)
        continue;
      multiple = true;
      break;
    }

    if (multiple) {
      if (config->verboseCompartmentalization) {
        message("info: disjoint set spans multiple compartments:");
        for (InputSectionBase *s : list)
          if (canCompartmentalize(s))
            message("\t" + isecName(s) + " from " +
                    compartmentName(s->compartment));
      }
      continue;
    }

    // Nothing to do if the sole compartment is the default compartment or all
    // of the input sections are already assigned.
    if (!anyDefault)
      continue;
    if (c == nullptr) {
      if (config->verboseCompartmentalization) {
        message("info: potential compartment:");
        for (InputSectionBase *s : list)
          if (canCompartmentalize(s))
            message("\t" + isecName(s));
      }
      continue;
    }

    // Don't assign anything if any of the input sections in the default
    // compartment contain exported symbols.
    bool exportedSym = false;
    for (InputSectionBase *s : list) {
      if (s->compartment == c || !canCompartmentalize(s))
        continue;

      Symbol *b = firstExportedSymbol(s);
      if (b != nullptr) {
        message("info: " + isecName(s) + " not assigned to implied " +
                compartmentName(c) + " due to exported " + symbolName(b));
        exportedSym = true;
      }
    }
    if (exportedSym)
      continue;

    if (config->verboseCompartmentalization)
      message("info: assigning disjoint set to " + compartmentName(c) + ":");
    for (InputSectionBase *s : list) {
      if (s->compartment == c || !canCompartmentalize(s))
        continue;

      s->compartment = c;
      if (config->verboseCompartmentalization)
        message("\t" + isecName(s));
    }
  }

  checkDefaultCompartment();
}

}
}
