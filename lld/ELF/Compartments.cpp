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

using namespace llvm;
using namespace llvm::object;

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

typedef std::pair <InputSectionBase *, Symbol *> SectionDep;
typedef std::unordered_map<Compartment *, SectionDep> CompartmentDeps;
typedef std::unordered_map<InputSectionBase *,
                           CompartmentDeps> SectionDepends;

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
    InputSectionBase *target = relocationTargetSection<ELFT>(sec, rel);

    if (target == nullptr)
      continue;

    // Ignore self-dependencies.  If all other references are from another
    // compartment, then the self-dependencies would also end up being from that
    // other compartment if the section is assigned.  However, tracking
    // self-dependencies means that the default compartment would always depend
    // on the section preventing implicit assignment.
    if (target == sec)
      continue;

    if (target->compartment != nullptr)
      continue;

    CompartmentDeps &cdeps = depends[target];
    if (cdeps.count(sec->compartment) == 0) {
      uint32_t symIndex = rel.getSymbol(config->isMips64EL);
      Symbol &sym = sec->getFile<ELFT>()->getSymbol(symIndex);
      cdeps[sec->compartment] = std::make_pair(sec, &sym);
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

  // Third, look for compartmentalizable sections not yet assigned to a
  // compartment that are depended on solely by another compartmentalized
  // section and assign those.  This process repeats until no new sections are
  // assigned.
  for (;;) {
    SectionDepends depends;

    for (InputSectionBase *s : ctx.inputSections) {
      if (s->file == nullptr || (s->flags & ELF::SHF_ALLOC) == 0)
        continue;

      if (s->name == ".eh_frame")
        continue;

      if (s->compartment == nullptr)
	continue;

      invokeELFT(scanRelocations, s, depends);
    }

    bool assigned = false;
    for (const auto &kv : depends) {
      const CompartmentDeps &cdeps = kv.second;
      if (cdeps.size() > 1)
        continue;

      InputSectionBase *s = kv.first;
      if (!canCompartmentalize(s) || firstExportedSymbol(s) != nullptr)
        continue;

      const auto &cdep = *cdeps.begin();
      Compartment *c = cdep.first;

      if (config->verboseCompartmentalization) {
        const auto pair = cdep.second;
        InputSectionBase *s2 = pair.first;
        Symbol *sym = pair.second;

        message("info: " + isecName(s) + " assigned to implied " +
                compartmentName(c) + " due to " + symbolName(sym) +
                " needed by " + isecName(s2));
      }

      s->compartment = c;
      assigned = true;
    }
    if (assigned)
      continue;

    if (config->verboseCompartmentalization) {
      for (const auto &kv : depends) {
        InputSectionBase *s = kv.first;
        const CompartmentDeps &cdeps = kv.second;

        if (cdeps.size() == 1) {
          Compartment *c = cdeps.begin()->first;
          if (c != nullptr) {
            if (!canCompartmentalize(s)) {
              message("info: " + isecName(s) + " not assigned to implied " +
                      compartmentName(c));
            } else {
              Symbol *b = firstExportedSymbol(s);
              if (b != nullptr) {
                message("info: " + isecName(s) + " not assigned to implied " +
                        compartmentName(c) + " due to exported " +
                        symbolName(b));
              }
            }
          }
          continue;
        }

        if (!canCompartmentalize(s) || firstExportedSymbol(s) != nullptr)
          continue;

        message("info: " + isecName(s) + " depended on by:");
        for (const auto &cdep : cdeps) {
          Compartment *c = cdep.first;
          const auto pair = cdep.second;
          InputSectionBase *s2 = pair.first;
          Symbol *sym = pair.second;

	  message("\t" + isecName(s2) + " from " + compartmentName(c) +
                  " due to " + symbolName(sym));
        }
      }
    }

    break;
  }

  checkDefaultCompartment();
}

}
}
