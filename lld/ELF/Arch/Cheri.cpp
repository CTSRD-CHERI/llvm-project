#include "Cheri.h"
#include "../InputFiles.h"
#include "../OutputSections.h"
#include "../SymbolTable.h"
#include "../SyntheticSections.h"
#include "../Target.h"
#include "../Writer.h"
#include "lld/Common/CommonLinkerContext.h"
#include "lld/Common/Memory.h"
#include "llvm/Support/Path.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::ELF;

namespace lld {
namespace elf {

bool isCheriAbi(const InputFile *f) {
  switch (f->emachine) {
  case EM_MIPS:
    return (f->eflags & EF_MIPS_ABI) == EF_MIPS_ABI_CHERIABI;
  case EM_RISCV:
    return f->eflags & EF_RISCV_CHERIABI;
  default:
    return false;
  }
}

// See CheriBSD crt_init_globals()
template <class ELFT> struct InMemoryCapRelocEntry {
  static constexpr size_t fieldSize = ELFT::Is64Bits ? 8 : 4;
  static constexpr size_t relocSize = fieldSize * 5;
  using NativeUint = typename ELFT::uint;
  using CapRelocUint = llvm::support::detail::packed_endian_specific_integral<
      NativeUint, ELFT::TargetEndianness, llvm::support::aligned>;
  InMemoryCapRelocEntry(NativeUint loc, NativeUint obj, NativeUint off,
                        NativeUint s, NativeUint perms)
      : capability_location(loc), object(obj), offset(off), size(s),
        permissions(perms) {}
  CapRelocUint capability_location;
  CapRelocUint object;
  CapRelocUint offset;
  CapRelocUint size;
  CapRelocUint permissions;
};

CheriCapRelocsSection::CheriCapRelocsSection(StringRef name)
    : SyntheticSection(SHF_ALLOC, SHT_PROGBITS, config->wordsize, name) {
  this->entsize = config->wordsize * 5;
}

SymbolAndOffset
SymbolAndOffset::fromSectionWithOffset(InputSectionBase *isec, int64_t offset,
                                       const SymbolAndOffset *Default) {
  Symbol *fallbackResult = nullptr;
  assert((int64_t)offset >= 0);
  int64_t fallbackOffset = offset;
  // For internal symbols we don't have a matching InputFile, just return
  auto* file = isec->file;
  if (!file) {
    if (Default)
      return *Default;
    return {isec, offset};
  }
  for (Symbol *b : isec->file->getSymbols()) {
    if (auto *d = dyn_cast<Defined>(b)) {
      if (d->section != isec)
        continue;
      if ((int64_t)d->value <= offset &&
          offset <= (int64_t)d->value + (int64_t)d->getSize()) {
        // XXXAR: should we accept any symbol that encloses or only exact
        // matches?
        if ((int64_t)d->value == offset && (d->isFunc() || d->isObject()))
          return {d, 0}; // perfect match
        fallbackResult = d;
        fallbackOffset = offset - d->value;
      }
    }
  }
  // When using the legacy __cap_relocs style (where clang emits __cap_relocs
  // instead of R_*_CHERI_CAPABILITY) the local symbols might not exist so we
  // may have fall back to the section.
  // TODO: Revisit this now we no longer support __cap_relocs input.
  if (!fallbackResult) {
    // worst case we fall back to the section + offset
    // Don't warn if the relocation is against an anonymous string constant
    // since clang won't emit a symbol (and no size) for those
    if (!isec->name.startswith(".rodata.str"))
      nonFatalWarning("Could not find a real symbol for " + isec->name +
           "+0x" + utohexstr(offset) + " in " + toString(isec->file));
    // Could not find a symbol -> return section+offset
    assert(offset == fallbackOffset);
    if (Default)
      return *Default;
    return {isec, offset};
  }
  return {fallbackResult, fallbackOffset};
}

SymbolAndOffset SymbolAndOffset::findRealSymbol() const {
  // If we only have a section, see if we can resolve section+offset to a
  // symbol (that may have been added later).
  if (symOrSec.is<InputSectionBase *>()) {
    return SymbolAndOffset::fromSectionWithOffset(
        symOrSec.get<InputSectionBase *>(), offset, this);
  }
  auto s = symOrSec.get<Symbol *>();
  if (!s->isSection())
    return *this;
  if (Defined *definedSym = dyn_cast<Defined>(s)) {
    if (auto *isec = dyn_cast<InputSectionBase>(definedSym->section)) {
      return SymbolAndOffset::fromSectionWithOffset(isec, offset, this);
    }
  }
  return *this;
}

SymbolAndOffset SymbolAndOffset::findSymbolForCapabilityRelocation() const {
  // In some cases we can end up with a symbol+offset that points to an
  // assembler local symbol without a type or size.
  // This happens e.g. for exception landing pads that are a relocation
  // against "fnstart+(lpad-fnstart)", but when building with RISC-V -mrelax,
  // the addend value may not be a constant, so the compiler must emit a
  // relocation against `lpad` instead which does not have a type or size
  // information. For such STT_NOTYPE symbols, we identify the surrounding one.
  // NOTE: This must only be called after linker relaxations are processed.
  Defined *self = dyn_cast<Defined>(sym());
  if (!self)
    return *this;
  assert(self->getSize() == 0);
  assert(self->type == STT_NOTYPE);
  Defined *bestMatch = self;
  uint64_t bestSize = bestMatch->getSize();
  const uint64_t targetValue = self->value + offset;
  auto foundBetterMatch = [&](Defined *newMatch) {
    int64_t oldOffset = targetValue - bestMatch->value;
    int64_t newOffset = targetValue - newMatch->value;
    log("Found better match for capability relocation against " +
        lld::toString(*bestMatch) + "+" + Twine(oldOffset) + ": " +
        lld::toString(*newMatch) + "+" + Twine(newOffset));
    bestMatch = newMatch;
    bestSize = bestMatch->getSize();
  };
  if (auto *isec = dyn_cast_or_null<InputSectionBase>(self->section)) {
    if (!isec->file)
      return *this;
    for (Symbol *b : isec->file->getSymbols()) {
      if (auto *d = dyn_cast<Defined>(b)) {
        if (d->section != isec)
          continue;
        uint64_t dsize = d->getSize();
        if (d->value <= targetValue && targetValue < d->value + dsize) {
          // Try to find a better match (with a size/type). The match is
          // considered better if
          //  - The current match has no size, and the new one does
          //  - The current match is preemptible, and the new one isn't
          //  - The current match has a smaller (non-zero) size
          //  - The current match has no type, and the new one is func/object
          if (d->type != STT_FUNC && d->type != STT_OBJECT)
            continue;
          if (bestSize == 0 && dsize != 0)
            foundBetterMatch(d);
          else if (bestMatch->isPreemptible && !d->isPreemptible)
            foundBetterMatch(d);
          else if (dsize != 0 && dsize < bestSize)
            foundBetterMatch(d);
          else if (bestMatch->type == STT_NOTYPE)
            foundBetterMatch(d);
        }
      }
    }
  }
  return {bestMatch, (int64_t)(targetValue - bestMatch->value)};
}

std::string CheriCapRelocLocation::toString() const {
 return SymbolAndOffset(section, offset).verboseToString();
}

void CheriCapRelocsSection::addCapReloc(CheriCapRelocLocation loc,
                                        const SymbolAndOffset &target,
                                        int64_t capabilityOffset,
                                        Symbol *sourceSymbol) {
  assert(!isa<Symbol *>(target.symOrSec) || !target.sym()->isPreemptible);

  auto sourceMsg = [&]() -> std::string {
    return sourceSymbol ? verboseToString(sourceSymbol) : loc.toString();
  };
  if (isa<Symbol *>(target.symOrSec) && target.sym()->isUndefined() &&
      !target.sym()->isUndefWeak()) {
    std::string msg =
        "cap_reloc against undefined symbol: " + toString(*target.sym()) +
        "\n>>> referenced by " + sourceMsg();
    if (config->unresolvedSymbols == UnresolvedPolicy::ReportError)
      error(msg);
    else
      nonFatalWarning(msg);
  }

  // assert(CapabilityOffset >= 0 && "Negative offsets not supported");
  if (errorHandler().verbose && capabilityOffset < 0)
    message("global capability offset " + Twine(capabilityOffset) +
            " is less than 0:\n>>> Location: " + loc.toString() +
            "\n>>> Target: " + target.verboseToString());

  bool canWriteLoc = (loc.section->flags & SHF_WRITE) || !config->zText;
  if (!canWriteLoc) {
    readOnlyCapRelocsError(*target.sym(), "\n>>> referenced by " + sourceMsg());
    return;
  }

  addEntry(loc, {target, capabilityOffset});
}

static uint64_t getTargetSize(const CheriCapRelocLocation &location,
                              const SymbolAndOffset &target) {
  if (InputSectionBase *isec = dyn_cast<InputSectionBase *>(target.symOrSec))
    return isec->getSize();

  uint64_t targetSize = target.sym()->getSize();
  if (targetSize > INT_MAX) {
    error("Insanely large symbol size for " + target.verboseToString() +
          "for cap_reloc at" + location.toString());
    return 0;
  }
  auto targetSym = target.sym();
  if (targetSize == 0 && !targetSym->isPreemptible) {
    StringRef name = targetSym->getName();
    // Section end symbols like __preinit_array_end, etc. should actually be
    // zero size symbol since they are just markers for the end of a section
    // and not usable as a valid pointer
    if (isSectionEndSymbol(name) || isSectionStartSymbol(name))
      return targetSize;

    bool isAbsoluteSym = targetSym->getOutputSection() == nullptr;
    // Symbols previously declared as weak can have size 0 (if they then resolve
    // to NULL). For example __preinit_array_start, etc. are generated by the
    // linker as ABS symbols with value 0.
    // A symbol is linker-synthesized/linker script generated if File == nullptr
    if (isAbsoluteSym && targetSym->file == nullptr)
      return targetSize;

    if (targetSym->isUndefWeak() && targetSym->getVA(0) == 0)
      // Weak symbol resolved to NULL -> zero size is fine
      return 0;

    // Absolute value provided by -defsym or assignment in .o file is fine
    if (isAbsoluteSym)
      return targetSize;

    // Otherwise warn about missing sizes for symbols
    bool warnAboutUnknownSize = true;
    // currently clang doesn't emit the necessary symbol information for local
    // string constants such as: struct config_opt opts[] = { { ..., "foo" },
    // { ..., "bar" } }; As this pattern is quite common don't warn if the
    // target section is .rodata.str
    if (Defined *definedSym =
      dyn_cast<Defined>(targetSym)) {
      if (definedSym->isSection() &&
          definedSym->section->name.startswith(".rodata.str")) {
        warnAboutUnknownSize = false;
      }
    }
    // TODO: are there any other cases that can be ignored?

    bool UnknownSectionSize = true;
    if (OutputSection *os = targetSym->getOutputSection()) {
      // Cast must succeed since getOutputSection() returned non-NULL
      Defined* def = cast<Defined>(targetSym);
      // warn("Could not find size for symbol " + target.verboseToString() +
      //    " and could not determine section size. Using 0.");
      if ((int64_t)def->value < 0 || def->value > os->size) {
        // Note: we allow def->value == os->size for pointers to the end
        warn("Symbol " + target.verboseToString() +
             " is defined as being in section " + os->name +
             " but the value (0x" + utohexstr(targetSym->getVA()) +
             ") is outside this section. Will create a zero-size capability."
             "\n>>> referenced by " +
             location.toString());
        return 0;
      }
      // For negative offsets use 0 instead (we want the range of the full symbol in that case)
      int64_t offset = std::max((int64_t)0, target.offset);
      uint64_t targetVA = targetSym->getVA(offset);
      assert(targetVA >= os->addr);
      uint64_t offsetInOS = targetVA - os->addr;
      // Check this isn't a symbol defined outside a section in a linker script.
      // Use less-or-equal here to account for __end_foo symbols which point 1 past the section
      if (offsetInOS <= os->size) {
        targetSize = os->size - offsetInOS;
        UnknownSectionSize = false;
      }
    }
    if (warnAboutUnknownSize || errorHandler().verbose) {
      std::string msg = "could not determine size of cap reloc against " +
                        target.verboseToString() + "\n>>> referenced by " +
                        location.toString();
      warn(msg);
    }
    if (UnknownSectionSize) {
      warn("Could not find size for symbol " + target.verboseToString() +
           " and could not determine section size. Using 0.");
      // TargetSize = std::numeric_limits<uint64_t>::max();
      return 0;
    }
  }
  return targetSize;
}

template <class ELFT>
struct CaptablePermissions {
  static const uint64_t function =
      UINT64_C(1) << ((sizeof(typename ELFT::uint) * 8) - 1);
  static const uint64_t readOnly =
      UINT64_C(1) << ((sizeof(typename ELFT::uint) * 8) - 2);
};

template <class ELFT>
void CheriCapRelocsSection::writeToImpl(uint8_t *buf) {
  assert(entsize == sizeof(InMemoryCapRelocEntry<ELFT>) &&
         "cap relocs size mismatch");
  uint64_t offset = 0;
  for (const auto &i : relocsMap) {
    const CheriCapRelocLocation &location = i.first;
    const CheriCapReloc &reloc = i.second;
    assert(location.offset <= location.section->getSize());

    // If we are targeting a symbol that does not have type and size
    // information, try to find the intended target symbol. This can happen for
    // relocations against a label inside a function or a subobject.
    SymbolAndOffset realTarget = reloc.target;
    if (Symbol *s = reloc.target.symOrSec.dyn_cast<Symbol *>())
      if (s->type == STT_NOTYPE && s->getSize() == 0)
        realTarget = reloc.target.findSymbolForCapabilityRelocation();

    // We write the virtual address of the location in both static and the
    // shared library case:
    // In the static case we can compute the final virtual address and write it
    // In the dynamic case we write the virtual address relative to the load
    // address and the runtime linker will add the load address to that
    uint64_t outSecOffset = location.section->getOffset(location.offset);
    uint64_t locationVA =
        location.section->getOutputSection()->addr + outSecOffset;

    // The target VA is the base address of the capability, so symbol + 0
    uint64_t targetVA;
    bool isFunc, isTls;
    OutputSection *os;
    if (Symbol *s = dyn_cast<Symbol *>(realTarget.symOrSec)) {
      if (s->isGnuIFunc())
        error("cannot reference non-preemptible IFUNC as a capability, "
              "needed for symbol " +
              realTarget.verboseToString() + "\n>>> referenced by " +
              location.toString());

      targetVA = realTarget.sym()->getVA(0);
      isFunc = s->isFunc();
      isTls = s->isTls();
      os = s->getOutputSection();
    } else {
      InputSectionBase *isec = cast<InputSectionBase *>(realTarget.symOrSec);
      targetVA = isec->getVA(0);
      isFunc = (isec->flags & SHF_EXECINSTR) != 0;
      isTls = isec->type == STT_TLS;
      os = isec->getOutputSection();
    }
    uint64_t targetSize = getTargetSize(location, realTarget);
    uint64_t targetOffset = reloc.capabilityOffset + realTarget.offset;
    uint64_t permissions = 0;
    // Fow now Function implies ReadOnly so don't add the flag
    if (isFunc) {
      permissions |= CaptablePermissions<ELFT>::function;
    } else if (os) {
      assert(!isTls);
      // if ((OS->getPhdrFlags() & PF_W) == 0) {
      if (((os->flags & SHF_WRITE) == 0) || isRelroSection(os)) {
        permissions |= CaptablePermissions<ELFT>::readOnly;
      } else if (os->flags & SHF_EXECINSTR) {
        warn("Non-function __cap_reloc against symbol in section with "
             "SHF_EXECINSTR (" +
             toString(os->name) + ") for symbol " +
             realTarget.verboseToString());
      }
    }

    // TODO: should we warn about symbols that are out-of-bounds?
    // mandoc seems to do it so I guess we need it
    // if (TargetOffset < 0 || TargetOffset > TargetSize) warn(...);

    InMemoryCapRelocEntry<ELFT> entry(locationVA, targetVA, targetOffset,
                                      targetSize, permissions);
    memcpy(buf + offset, &entry, sizeof(entry));
    //     if (errorHandler().verbose) {
    //       errs() << "Added capability reloc: loc=" << utohexstr(LocationVA)
    //              << ", object=" << utohexstr(TargetVA)
    //              << ", offset=" << utohexstr(TargetOffset)
    //              << ", size=" << utohexstr(TargetSize)
    //              << ", permissions=" << utohexstr(Permissions) << "\n";
    //     }
    offset += InMemoryCapRelocEntry<ELFT>::relocSize;
  }

  // Sort the cap_relocs by target address for better cache and TLB locality
  // It also makes it much easier to read the llvm-objdump -C output since it
  // is sorted in a sensible order
  // FIXME: do the sorting in finalizeSection instead
  if (config->sortCapRelocs)
    std::stable_sort(reinterpret_cast<InMemoryCapRelocEntry<ELFT> *>(buf),
                     reinterpret_cast<InMemoryCapRelocEntry<ELFT> *>(buf + offset),
                     [](const InMemoryCapRelocEntry<ELFT> &a,
                        const InMemoryCapRelocEntry<ELFT> &b) {
                       return a.capability_location < b.capability_location;
                     });
  assert(offset == getSize() && "Not all data written?");
}

void CheriCapRelocsSection::writeTo(uint8_t *buf) {
  invokeELFT(writeToImpl, buf);
}

MipsCheriCapTableSection::MipsCheriCapTableSection()
    : SyntheticSection(SHF_ALLOC | SHF_WRITE, SHT_PROGBITS,
                       config->capabilitySize, ".captable") {
  assert(config->capabilitySize > 0);
  this->entsize = config->capabilitySize;
}

void MipsCheriCapTableSection::writeTo(uint8_t *buf) {
  // Capability part should be filled with all zeros and crt_init_globals fills
  // it in. For the TLS part, assignValuesAndAddCapTableSymbols adds any static
  // relocations needed, and should be procesed by relocateAlloc.
  // TODO: Fill in the raw capability bits and use CBuildCap
  target->relocateAlloc(*this, buf);
}

static Defined *findMatchingFunction(const InputSectionBase *isec,
                                     uint64_t symOffset) {
  return isec->getEnclosingFunction(symOffset);
}

MipsCheriCapTableSection::CaptableMap &
MipsCheriCapTableSection::getCaptableMapForFileAndOffset(
    const InputSectionBase *isec, uint64_t offset) {
  if (LLVM_LIKELY(config->capTableScope == CapTableScopePolicy::All))
    return globalEntries;
  if (config->capTableScope == CapTableScopePolicy::File) {
    // operator[] will insert if missing
    return perFileEntries[isec->file];
  }
  if (config->capTableScope == CapTableScopePolicy::Function) {
    Symbol *func = findMatchingFunction(isec, offset);
    if (!func) {
      warn(
          "Could not find corresponding function with per-function captable: " +
          isec->getObjMsg(offset));
    }
    // operator[] will insert if missing
    return perFunctionEntries[func];
  }
  llvm_unreachable("INVALID CONFIG OPTION");
  return globalEntries;
}

void MipsCheriCapTableSection::addEntry(Symbol &sym, RelExpr expr,
                                        InputSectionBase *isec,
                                        uint64_t offset) {
  // FIXME: can this be called from multiple threads?
  CapTableIndex idx;
  idx.needsSmallImm = false;
  idx.usedInCallExpr = false;
  idx.firstUse = SymbolAndOffset(isec, offset);
  assert(!idx.firstUse->symOrSec.isNull());
  switch (expr) {
  case R_MIPS_CHERI_CAPTAB_INDEX_SMALL_IMMEDIATE:
  case R_MIPS_CHERI_CAPTAB_INDEX_CALL_SMALL_IMMEDIATE:
    idx.needsSmallImm = true;
    break;
  default:
    break;
  }
  // If the symbol is only ever referenced by the captable call relocations we
  // can emit a capability call relocation instead of a normal capability
  // relocation. This indicates to the runtime linker that the capability is
  // not used as a function pointer and therefore does not need a unique
  // address (plt stub) across all DSOs.
  switch (expr) {
  case R_MIPS_CHERI_CAPTAB_INDEX_CALL:
  case R_MIPS_CHERI_CAPTAB_INDEX_CALL_SMALL_IMMEDIATE:
    if (!sym.isFunc() && !sym.isUndefWeak()) {
      CheriCapRelocLocation loc{isec, offset};
      std::string msg = "call relocation against non-function symbol " + verboseToString(&sym, 0) +
      "\n>>> referenced by " + loc.toString();
      if (sym.isUndefined() && config->unresolvedSymbolsInShlib == UnresolvedPolicy::Ignore) {
        // Don't fail the build for shared libraries unless
        nonFatalWarning(msg);
      } else {
        warn(msg);
      }
    }
    idx.usedInCallExpr = true;
    break;
  default:
    break;
  }
  CaptableMap &entries = getCaptableMapForFileAndOffset(isec, offset);
  if (config->zCapTableDebug) {
    // Add a local helper symbol to improve disassembly:
    StringRef helperSymName = saver().save(
        "$captable_load_" +
        (sym.getName().empty() ? "$anonymous_symbol" : sym.getName()));
    addSyntheticLocal(helperSymName, STT_NOTYPE, offset, 0, *isec);
  }

  auto it = entries.map.insert(std::make_pair(&sym, idx));
  if (!it.second) {
    // If it is references by a small immediate relocation we need to update
    // the small immediate flag
    if (idx.needsSmallImm)
      it.first->second.needsSmallImm = true;
    // If one of the uses is not a call expression, we need to emit a unique
    // function pointer and reuse that for the call expression, too.
    // TODO: should we emit two relocations instead?
    if (!idx.usedInCallExpr)
      it.first->second.usedInCallExpr = false;
  }
}

void MipsCheriCapTableSection::addDynTlsEntry(Symbol &sym) {
  dynTlsEntries.map.insert(std::make_pair(&sym, CapTableIndex()));
}

void MipsCheriCapTableSection::addTlsIndex() {
  dynTlsEntries.map.insert(std::make_pair(nullptr, CapTableIndex()));
}

void MipsCheriCapTableSection::addTlsEntry(Symbol &sym) {
  tlsEntries.map.insert(std::make_pair(&sym, CapTableIndex()));
}

uint32_t MipsCheriCapTableSection::getIndex(const Symbol &sym,
                                            const InputSectionBase *isec,
                                            uint64_t offset) const {
  assert(valuesAssigned && "getIndex called before index assignment");
  const CaptableMap &entries =
      const_cast<MipsCheriCapTableSection *>(this)
          ->getCaptableMapForFileAndOffset(isec, offset);
  auto it = entries.map.find(const_cast<Symbol *>(&sym));
  assert(entries.firstIndex != std::numeric_limits<uint64_t>::max() &&
         "First index not set yet?");
  assert(it != entries.map.end());
  // The index that is written as part of the relocation is relative to the
  // start of the current captable subset (or the global table in the default
  // case). When using per-function tables the first index in every function
  // will always be zero.
  return *it->second.index - entries.firstIndex;
}

uint32_t MipsCheriCapTableSection::getDynTlsOffset(const Symbol &sym) const {
  assert(valuesAssigned && "getDynTlsOffset called before index assignment");
  auto it = dynTlsEntries.map.find(const_cast<Symbol *>(&sym));
  assert(it != dynTlsEntries.map.end());
  return *it->second.index * config->wordsize;
}

uint32_t MipsCheriCapTableSection::getTlsIndexOffset() const {
  assert(valuesAssigned && "getTlsIndexOffset called before index assignment");
  auto it = dynTlsEntries.map.find(nullptr);
  assert(it != dynTlsEntries.map.end());
  return *it->second.index * config->wordsize;
}

uint32_t MipsCheriCapTableSection::getTlsOffset(const Symbol &sym) const {
  assert(valuesAssigned && "getTlsOffset called before index assignment");
  auto it = tlsEntries.map.find(const_cast<Symbol *>(&sym));
  assert(it != tlsEntries.map.end());
  return *it->second.index * config->wordsize;
}

uint64_t MipsCheriCapTableSection::assignIndices(uint64_t startIndex,
                                                 CaptableMap &entries,
                                                 const Twine &symContext) {
  // Usually StartIndex will be zero (one global captable) but if we are
  // compiling with per-file/per-function
  uint64_t smallEntryCount = 0;
  assert(entries.firstIndex == std::numeric_limits<uint64_t>::max() &&
         "Should not be initialized yet!");
  entries.firstIndex = startIndex;
  for (auto &it : entries.map) {
    // TODO: looping twice is inefficient, we could keep track of the number of
    // small entries during insertion
    if (it.second.needsSmallImm) {
      smallEntryCount++;
    }
  }

  if (config->emachine == EM_MIPS) {
    unsigned maxSmallEntries = (1 << 19) / config->capabilitySize;
    if (smallEntryCount > maxSmallEntries) {
      // Use warn here since the calculation may be wrong if the 11 bit clc is
      // used. We will error when writing the relocation values later anyway
      // so this will help find the error
      warn("added " + Twine(smallEntryCount) + " entries to .captable but "
          "current maximum is " + Twine(maxSmallEntries) + "; try recompiling "
          "non-performance critical source files with -mxcaptable");
    }
    if (errorHandler().verbose) {
      message("Total " + Twine(entries.size()) + " .captable entries: " +
          Twine(smallEntryCount) + " use a small immediate and " +
          Twine(entries.size() - smallEntryCount) + " use -mxcaptable. ");
    }
  }

  // Only add the @CAPTABLE symbols when running the LLD unit tests
  // errorHandler().exitEarly is set to false if LLD_IN_TEST=1 so just reuse
  // that instead of calling getenv on every iteration
  const bool shouldAddAtCaptableSymbols = !errorHandler().exitEarly;
  uint32_t assignedSmallIndexes = 0;
  uint32_t assignedLargeIndexes = 0;
  for (auto &it : entries.map) {
    CapTableIndex &cti = it.second;
    if (cti.needsSmallImm) {
      assert(assignedSmallIndexes < smallEntryCount);
      cti.index = startIndex + assignedSmallIndexes;
      assignedSmallIndexes++;
    } else {
      cti.index = startIndex + smallEntryCount + assignedLargeIndexes;
      assignedLargeIndexes++;
    }

    uint32_t index = *cti.index;
    assert(index >= startIndex && index < startIndex + entries.size());
    Symbol *targetSym = it.first;

    StringRef name = targetSym->getName();
    // Avoid duplicate symbol name errors for unnamed string constants:
    StringRef refName;
    // For now always append .INDEX to local symbols @CAPTABLE names since they
    // might not be unique. If there is a global with the same name we always
    // want the global to have the plain @CAPTABLE name
    if (name.empty() /* || Name.startswith(".L") */ || targetSym->isLocal())
      refName = saver().save(name + "@CAPTABLE" + symContext + "." + Twine(index));
    else
      refName = saver().save(name + "@CAPTABLE" + symContext);
    // XXXAR: This should no longer be necessary now that I am using addSyntheticLocal?
#if 0
    if (Symtab->find(RefName)) {
      std::string NewRefName =
          (Name + "@CAPTABLE" + SymContext + "." + Twine(Index)).str();
      // XXXAR: for some reason we sometimes create more than one cap table entry
      // for a given global name, for now just rename the symbol
      // assert(TargetSym->isLocal());
      if (!TargetSym->isLocal()) {
        error("Found duplicate global captable ref name " + RefName +
              " but referenced symbol was not local\n>>> " +
              verboseToString(TargetSym));
      } else {
        // TODO: make this a warning
        message("Found duplicate captable name " + RefName +
                "\n>>> Replacing with " + NewRefName);
      }
      RefName = std::move(NewRefName);
      assert(!Symtab->find(RefName) && "RefName should be unique");
    }
#endif
    uint64_t off = index * config->capabilitySize;
    if (shouldAddAtCaptableSymbols) {
      addSyntheticLocal(refName, STT_OBJECT, off, config->capabilitySize, *this);
    }
    // If the symbol is used as a function pointer the runtime linker has to
    // ensure that all pointers to that function compare equal. This is done
    // by ensuring that they all point to the same PLT stub.
    // If it is not used as a function pointer we can use a capability call
    // relocation instead which allows the runtime linker to create non-unique
    // plt stubs.
    RelType elfCapabilityReloc = it.second.usedInCallExpr
                                     ? *target->cheriCapCallRel
                                     : *target->cheriCapRel;
    // All capability call relocations should end up in the pltrel section
    // rather than the normal relocation section to make processing of PLT
    // relocations in RTLD more efficient.
    RelocationBaseSection &dynRelSec =
        it.second.usedInCallExpr ? *in.relaPlt : *mainPart->relaDyn;
    if (targetSym->isPreemptible)
      dynRelSec.addSymbolReloc(elfCapabilityReloc, *this, off, *targetSym);
    else if (targetSym->isUndefWeak())
      addConstant({R_ABS_CAP, elfCapabilityReloc, off, 0, targetSym});
    else
      addRelativeCapabilityRelocation(*this, off, targetSym, 0, R_ABS_CAP,
                                      elfCapabilityReloc);
  }
  assert(assignedSmallIndexes + assignedLargeIndexes == entries.size());
  return assignedSmallIndexes + assignedLargeIndexes;
}

void MipsCheriCapTableSection::assignValuesAndAddCapTableSymbols() {
  // First assign the global indices (which will usually be the only ones)
  uint64_t assignedEntries = assignIndices(0, globalEntries, "");
  if (LLVM_UNLIKELY(config->capTableScope != CapTableScopePolicy::All)) {
    assert(assignedEntries == 0 && "Should not have any global entries in"
                                   " per-file/per-function captable mode");
    for (auto &it : perFileEntries) {
      std::string fullContext = toString(it.first);
      auto lastSlash = StringRef(fullContext).find_last_of("/\\") + 1;
      StringRef context = StringRef(fullContext).substr(lastSlash);
      assignedEntries +=
          assignIndices(assignedEntries, it.second, "@" + context);
    }
    for (auto &it : perFunctionEntries)
      assignedEntries +=
          assignIndices(assignedEntries, it.second, "@" + toString(*it.first));
  }
  assert(assignedEntries == nonTlsEntryCount());

  uint32_t assignedTlsIndexes = 0;
  uint32_t tlsBaseIndex =
      assignedEntries * (config->capabilitySize / config->wordsize);

  // TODO: support TLS for per-function captable
  if (config->capTableScope != CapTableScopePolicy::All &&
      (!dynTlsEntries.empty() || !tlsEntries.empty())) {
    error("TLS is not supported yet with per-file or per-function captable");
    return;
  }

  for (auto &it : dynTlsEntries.map) {
    CapTableIndex &cti = it.second;
    assert(!cti.needsSmallImm);
    cti.index = tlsBaseIndex + assignedTlsIndexes;
    assignedTlsIndexes += 2;
    Symbol *s = it.first;
    uint64_t offset = *cti.index * config->wordsize;
    if (s == nullptr) {
      if (!config->shared)
        addConstant({R_ADDEND, target->symbolicRel, offset, 1, s});
      else
        mainPart->relaDyn->addReloc({target->tlsModuleIndexRel, this, offset});
    } else {
      // When building a shared library we still need a dynamic relocation
      // for the module index. Therefore only checking for
      // s->isPreemptible is not sufficient (this happens e.g. for
      // thread-locals that have been marked as local through a linker script)
      if (!s->isPreemptible && !config->shared)
        addConstant({R_ADDEND, target->symbolicRel, offset, 1, s});
      else
        mainPart->relaDyn->addSymbolReloc(target->tlsModuleIndexRel, *this,
                                          offset, *s);

      offset += config->wordsize;

      // However, we can skip writing the TLS offset reloc for non-preemptible
      // symbols since it is known even in shared libraries
      if (!s->isPreemptible)
        addConstant({R_ABS, target->tlsOffsetRel, offset, 0, s});
      else
        mainPart->relaDyn->addSymbolReloc(target->tlsOffsetRel, *this, offset,
                                          *s);
    }
  }

  for (auto &it : tlsEntries.map) {
    CapTableIndex &cti = it.second;
    assert(!cti.needsSmallImm);
    cti.index = tlsBaseIndex + assignedTlsIndexes++;
    Symbol *s = it.first;
    uint64_t offset = *cti.index * config->wordsize;
    // When building a shared library we still need a dynamic relocation
    // for the TP-relative offset as we don't know how much other data will
    // be allocated before us in the static TLS block.
    if (!s->isPreemptible && !config->shared)
      addConstant({R_TPREL, target->symbolicRel, offset, 0, s});
    else
      mainPart->relaDyn->addAddendOnlyRelocIfNonPreemptible(
          target->tlsGotRel, *this, offset, *s, target->symbolicRel);
  }

  valuesAssigned = true;
}

MipsCheriCapTableMappingSection::MipsCheriCapTableMappingSection()
    : SyntheticSection(SHF_ALLOC, SHT_PROGBITS, 8, ".captable_mapping") {
  assert(config->capabilitySize > 0);
  this->entsize = sizeof(CaptableMappingEntry);
  static_assert(sizeof(CaptableMappingEntry) == 24, "");
}

size_t MipsCheriCapTableMappingSection::getSize() const {
  assert(config->capTableScope != CapTableScopePolicy::All);
  if (!isNeeded())
    return 0;
  size_t count = 0;
  if (!in.symTab) {
    error("Cannot use " + this->name + " without .symtab section!");
    return 0;
  }
  for (const SymbolTableEntry &ste : in.symTab->getSymbols()) {
    if (!ste.sym->isDefined() || !ste.sym->isFunc())
      continue;
    count++;
  }
  return count * sizeof(CaptableMappingEntry);
}

void MipsCheriCapTableMappingSection::writeTo(uint8_t *buf) {
  assert(config->capTableScope != CapTableScopePolicy::All);
  if (!in.mipsCheriCapTable)
    return;
  if (!in.symTab) {
    error("Cannot write " + this->name + " without .symtab section!");
    return;
  }

  // Write the mapping from function vaddr -> captable subset for RTLD
  std::vector<CaptableMappingEntry> entries;
  // Note: Symtab->getSymbols() only returns the symbols in .dynsym. We need
  // to use In.sym()tab instead since we also want to add all local functions!
  for (const SymbolTableEntry &ste : in.symTab->getSymbols()) {
    Symbol* sym = ste.sym;
    if (!sym->isDefined() || !sym->isFunc())
      continue;
    const MipsCheriCapTableSection::CaptableMap *capTableMap = nullptr;
    if (config->capTableScope == CapTableScopePolicy::Function) {
      auto it = in.mipsCheriCapTable->perFunctionEntries.find(sym);
      if (it != in.mipsCheriCapTable->perFunctionEntries.end())
        capTableMap = &it->second;
    } else if (config->capTableScope == CapTableScopePolicy::File) {
      auto it = in.mipsCheriCapTable->perFileEntries.find(sym->file);
      if (it != in.mipsCheriCapTable->perFileEntries.end())
        capTableMap = &it->second;
    } else {
      llvm_unreachable("Invalid mode!");
    }
    CaptableMappingEntry entry;
    entry.funcStart = sym->getVA(0);
    entry.funcEnd = entry.funcStart + sym->getSize();
    if (capTableMap) {
      assert(capTableMap->firstIndex != std::numeric_limits<uint64_t>::max());
      entry.capTableOffset = capTableMap->firstIndex * config->capabilitySize;
      entry.subTableSize = capTableMap->size() * config->capabilitySize;
    } else {
      // TODO: don't write an entry for functions that don't use the captable
      entry.capTableOffset = 0;
      entry.subTableSize = 0;
    }
    entries.push_back(entry);
  }
  // Sort all the entries so that RTLD can do a binary search to find the
  // correct entry instead of having to scan all of them.
  // Do this before swapping to target endianess to simplify the comparisons.
  llvm::sort(entries, [](const CaptableMappingEntry &e1,
                         const CaptableMappingEntry &e2) {
    if (e1.funcStart == e2.funcStart)
      return e1.funcEnd < e2.funcEnd;
    return e1.funcStart < e2.funcStart;
  });
  // Byte-swap all the values so that we can memcpy the sorted buffer
  for (CaptableMappingEntry &e : entries) {
    e.funcStart = support::endian::byte_swap(e.funcStart, config->endianness);
    e.funcEnd = support::endian::byte_swap(e.funcEnd, config->endianness);
    e.capTableOffset =
        support::endian::byte_swap(e.capTableOffset, config->endianness);
    e.subTableSize =
        support::endian::byte_swap(e.subTableSize, config->endianness);
  }
  assert(entries.size() * sizeof(CaptableMappingEntry) == getSize());
  memcpy(buf, entries.data(), entries.size() * sizeof(CaptableMappingEntry));
}

template <typename ELFT>
static void getMipsCheriAbiVariant(std::optional<unsigned> &abi,
                                   SyntheticSection &sec) {
  abi = static_cast<MipsAbiFlagsSection<ELFT> &>(sec).getCheriAbiVariant();
}

static bool needsCheriMipsTrampoline(RelType type, const Symbol &sym) {
  // In the PLT ABI (and fndesc?) we have to use an elf relocation for function
  // pointers to ensure that the runtime linker adds the required trampolines
  // that sets $cgp:

  if (config->emachine != EM_MIPS)
    return false;

  if (!sym.isFunc() || type == *target->cheriCapCallRel)
    return false;

  // In static binaries we do not need PLT stubs for function pointers since
  // all functions share the same $cgp
  // TODO: this is no longer true if we were to support dlopen() in static
  // binaries
  if (!hasDynamicLinker())
    return false;

  if (!in.mipsAbiFlags)
    return false;

  std::optional<unsigned> abi;
  invokeELFT(getMipsCheriAbiVariant, abi, *in.mipsAbiFlags);
  if (!abi)
    return false;

  if (*abi != DF_MIPS_CHERI_ABI_PLT && *abi != DF_MIPS_CHERI_ABI_FNDESC)
    return false;

  return true;
}

static Symbol &getCheriMipsTrampolineSym(RelType type, Symbol &sym) {
  assert(needsCheriMipsTrampoline(type, sym));

  if (sym.includeInDynsym())
    return sym;

  Defined &newSym = *symtab.ensureSymbolWillBeInDynsym(&sym);
  assert(newSym.isFunc() && "This should only be used for functions");
  assert(newSym.includeInDynsym());
  assert(newSym.binding == llvm::ELF::STB_GLOBAL);
  assert(newSym.visibility() == llvm::ELF::STV_HIDDEN);
  return newSym;
}

void addRelativeCapabilityRelocation(
    InputSectionBase &isec, uint64_t offsetInSec,
    llvm::PointerUnion<Symbol *, InputSectionBase *> symOrSec, int64_t addend,
    RelExpr expr, RelType type) {
  Symbol *sym = dyn_cast<Symbol *>(symOrSec);
  assert(expr == R_ABS_CAP);
  if (sym && needsCheriMipsTrampoline(type, *sym)) {
    if (config->verboseCapRelocs)
      message("Forcing symbolic relocation for non-preemptible "
              "trampoline-using function pointer against " +
              verboseToString(sym));

    sym = &getCheriMipsTrampolineSym(type, *sym);
    mainPart->relaDyn->addSymbolReloc(type, isec, offsetInSec, *sym, addend,
                                      type);
    return;
  }
  assert(!sym || !sym->isPreemptible);
  assert(!config->useRelativeElfCheriRelocs &&
         "relative ELF capability relocations not currently implemented");
  in.capRelocs->addCapReloc({&isec, offsetInSec}, {symOrSec, 0u}, addend);
}

} // namespace elf
} // namespace lld
