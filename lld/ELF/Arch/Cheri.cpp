#include "Cheri.h"
#include "../OutputSections.h"
#include "../SymbolTable.h"
#include "../SyntheticSections.h"
#include "../Target.h"
#include "../Writer.h"
#include "lld/Common/Memory.h"
#include "llvm/Support/Path.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::ELF;


// Change these to #define for extremely verbose debug output
#undef DEBUG_CAP_RELOCS
#undef DEBUG_CAP_TABLE

namespace lld {
namespace elf {

static uint8_t GetEncodedIndexForSegment(PhdrEntry *Segment) {
  // CheriOS encodes no segment as index 0 and other indices with a plus 1
  if (Segment == nullptr)
    return (uint8_t)0;

  assert(Segment->ndx < 0xFE);
  return (uint8_t)(Segment->ndx + 1);
}

// See CheriBSD crt_init_globals()
template <class ELFT> struct InMemoryCapRelocEntry {
  using NativeUint = typename ELFT::uint;
  using CapRelocUint = llvm::support::detail::packed_endian_specific_integral<
      NativeUint, ELFT::TargetEndianness, llvm::support::aligned>;

  using CapRelocUint64 = llvm::support::detail::packed_endian_specific_integral<
      uint64_t, ELFT::TargetEndianness, llvm::support::aligned>;
  using CapRelocUint32 = llvm::support::detail::packed_endian_specific_integral<
      uint32_t, ELFT::TargetEndianness, llvm::support::aligned>;
  using CapRelocUint16 = llvm::support::detail::packed_endian_specific_integral<
      uint16_t, ELFT::TargetEndianness, llvm::support::aligned>;
  using CapRelocUint8 = llvm::support::detail::packed_endian_specific_integral<
      uint8_t, ELFT::TargetEndianness, llvm::support::aligned>;

  InMemoryCapRelocEntry(uint64_t loc, uint64_t obj, uint64_t off, uint64_t s,
                        uint64_t perms)
      : capability_location(loc), object(obj), offset(off), size(s),
        targetSpecific(perms) {}

  InMemoryCapRelocEntry(uint64_t loc, uint64_t obj, uint64_t off, uint64_t s,
                        uint64_t perms, uint16_t flags, uint8_t locSeg,
                        uint8_t obSeg)
      : capability_location(loc), object(obj), offset(off), size(s),
        targetSpecific(perms, flags, locSeg, obSeg) {}

  CapRelocUint capability_location;
  CapRelocUint object;
  CapRelocUint offset;
  CapRelocUint size;

  // See CheriOS crt_init_globals
  struct CheriOSStyle64 {
    CapRelocUint32 permissions;
    CapRelocUint16 flags;
    CapRelocUint8 location_seg_ndx;
    CapRelocUint8 object_seg_ndx;
    CheriOSStyle64(uint64_t perms, uint16_t flags, uint8_t LocSeg,
                   uint8_t obSeg)
        : permissions(perms >> 32), flags(flags), location_seg_ndx(LocSeg),
          object_seg_ndx(obSeg) {}
  };

  struct CheriOSStyle32 {
    CapRelocUint8 permissions;
    CapRelocUint8 flags;
    CapRelocUint8 location_seg_ndx;
    CapRelocUint8 object_seg_ndx;
    CheriOSStyle32(uint64_t perms, uint8_t flags, uint8_t LocSeg, uint8_t obSeg)
        : permissions(perms >> 24), flags(flags), location_seg_ndx(LocSeg),
          object_seg_ndx(obSeg) {}
  };

  using cheriOSStyleT =
      typename std::conditional<sizeof(NativeUint) == 8, CheriOSStyle64,
                                CheriOSStyle32>::type;

  union TargetSpecific {

    struct BSDStyle {
      CapRelocUint permissions;
      BSDStyle(uint64_t perms) : permissions(perms) {}
    } bsdStyle;

    cheriOSStyleT cheriOSStyle;

    TargetSpecific(uint64_t perms) : bsdStyle(perms) {}

    TargetSpecific(uint64_t perms, uint16_t flags, uint8_t locSeg,
                   uint8_t obSeg)
        : cheriOSStyle(perms, flags, locSeg, obSeg){};
  } targetSpecific;
};

template <class ELFT>
CheriCapRelocsSection<ELFT>::CheriCapRelocsSection()
    : SyntheticSection((config->isPic && !config->relativeCapRelocsOnly)
                           ? SHF_ALLOC | SHF_WRITE /* XXX: actually RELRO */
                           : SHF_ALLOC,
                       SHT_PROGBITS, config->wordsize, "__cap_relocs") {
  this->entsize = relocSize;
}

// TODO: copy MipsABIFlagsSection::create() instead of current impl?
template <class ELFT>
void CheriCapRelocsSection<ELFT>::addSection(InputSectionBase *s) {
  // FIXME: can this happen with ld -r ?
  // error("Compiler should not have generated __cap_relocs section for " + toString(S));
  assert(s->name == "__cap_relocs");
  assert(s->areRelocsRela && "__cap_relocs should be RELA");
  // make sure the section is no longer processed
  s->markDead();

  if ((s->getSize() % entsize) != 0) {
    error("__cap_relocs section size is not a multiple of " + Twine(entsize) +
          ": " + toString(s));
    return;
  }
  size_t numCapRelocs = s->getSize() / relocSize;
  if (numCapRelocs * 2 != s->numRelocations) {
    error("expected " + Twine(numCapRelocs * 2) + " relocations for " +
          toString(s) + " but got " + Twine(s->numRelocations));
    return;
  }
  if (config->verboseCapRelocs)
    message("Adding cap relocs from " + toString(s->file) + "\n");

  legacyInputs.push_back(s);
}

template <class ELFT> void CheriCapRelocsSection<ELFT>::finalizeContents() {
  for (InputSectionBase *s : legacyInputs) {
    if (config->verboseCapRelocs)
      message("Processing legacy cap relocs from " + toString(s->file) + "\n");
    processSection(s);
  }
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
          offset <= (int64_t)d->value + (int64_t)d->size) {
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
  // instead of R_CHERI_CAPABILITY) the local symbols might not exist so we
  // may have fall back to the section.
  if (!fallbackResult) {
    // worst case we fall back to the section + offset
    // Don't warn if the relocation is against an anonymous string constant
    // since clang won't emit a symbol (and no size) for those
    // CheriOS hits this warning a LOT. Is it a bit over-zelous?
    if (!isec->name.startswith(".rodata.str") && !config->isCheriOS())
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

std::string CheriCapRelocLocation::toString() const {
  SymbolAndOffset resolved =
      SymbolAndOffset::fromSectionWithOffset(section, offset);
 return resolved.verboseToString();
}

template <class ELFT>
void CheriCapRelocsSection<ELFT>::processSection(InputSectionBase *s) {
  // TODO: sort by offset (or is that always true?
  const auto rels = s->relas<ELFT>();
  for (auto i = rels.begin(), end = rels.end(); i != end; ++i) {
    const auto &locationRel = *i;
    ++i;
    const auto &targetRel = *i;
    if ((locationRel.r_offset % entsize) != 0) {
      error("corrupted __cap_relocs:  expected Relocation offset to be a "
            "multiple of " +
            Twine(entsize) + " but got " + Twine(locationRel.r_offset));
      return;
    }
    if (targetRel.r_offset != locationRel.r_offset + fieldSize) {
      error("corrupted __cap_relocs: expected target relocation (" +
            Twine(targetRel.r_offset) +
            " to directly follow location relocation (" +
            Twine(locationRel.r_offset) + ")");
      return;
    }
    if (locationRel.r_addend < 0) {
      error("corrupted __cap_relocs: addend is less than zero in" +
            toString(s) + ": " + Twine(locationRel.r_addend));
      return;
    }
    uint64_t capRelocsOffset = locationRel.r_offset;
    assert(capRelocsOffset + entsize <= s->getSize());
    if (config->emachine == EM_MIPS) {
      if (locationRel.getType(config->isMips64EL) != R_MIPS_64) {
        error("Exptected a R_MIPS_64 relocation in __cap_relocs but got " +
              toString(locationRel.getType(config->isMips64EL)));
        continue;
      }
      if (targetRel.getType(config->isMips64EL) != R_MIPS_64) {
        error("Exptected a R_MIPS_64 relocation in __cap_relocs but got " +
              toString(targetRel.getType(config->isMips64EL)));
        continue;
      }
    } else {
      if (locationRel.getType(config->isMips64EL) != *target->absPointerRel) {
        error("Exptected an absolute pointer relocation in __cap_relocs "
              "but got " + toString(locationRel.getType(config->isMips64EL)));
        continue;
      }
      if (targetRel.getType(config->isMips64EL) != *target->absPointerRel) {
        error("Exptected an absolute pointer relocation in __cap_relocs "
              "but got " + toString(targetRel.getType(config->isMips64EL)));
        continue;
      }
    }
    Symbol *locationSym =
        &s->getFile<ELFT>()->getRelocTargetSym(locationRel);
    Symbol &targetSym = s->getFile<ELFT>()->getRelocTargetSym(targetRel);

    if (locationSym->file != s->file) {
      error("Expected capability relocation to point to " + toString(s->file) +
            " but got " + toString(locationSym->file));
      continue;
    }
    //    errs() << "Adding cap reloc at " << toString(LocationSym) << " type "
    //           << Twine((int)LocationSym.Type) << " against "
    //           << toString(TargetSym) << "\n";
    auto *rawInput = reinterpret_cast<const InMemoryCapRelocEntry<ELFT> *>(
        s->data().begin() + capRelocsOffset);
    int64_t targetCapabilityOffset = (int64_t)rawInput->offset;
    assert(rawInput->size == 0 && "Clang should not have set size in __cap_relocs");
    if (!isa<Defined>(locationSym)) {
      error("Unhandled symbol kind for cap_reloc: " +
            Twine(locationSym->kind()));
      continue;
    }

    const SymbolAndOffset relocLocation{locationSym, locationRel.r_addend};
    const SymbolAndOffset relocTarget{&targetSym, targetRel.r_addend};
    SymbolAndOffset realLocation = relocLocation.findRealSymbol();
    SymbolAndOffset realTarget = relocTarget.findRealSymbol();
    if (config->verboseCapRelocs) {
      message("Adding capability relocation at " +
              realLocation.verboseToString() + "\nagainst " +
              realTarget.verboseToString());
    }

    bool targetNeedsDynReloc = false;
    if (targetSym.isPreemptible) {
      // Do we need this?
      // TargetNeedsDynReloc = true;
    }
    switch (targetSym.kind()) {
    case Symbol::DefinedKind:
      break;
    case Symbol::SharedKind:
      if (!hasDynamicLinker()) {
        error("cannot create a capability relocation against a shared symbol"
              " when linking statically");
        continue;
      }
      targetNeedsDynReloc = true;
      break;
    case Symbol::UndefinedKind:
      // addCapReloc() will add an error if we are building an executable
      // instead of a shlib
      // TODO: we really should add a dynamic SIZE relocation as well
      targetNeedsDynReloc = true;
      break;
    default:
      error("Unhandled symbol kind for cap_reloc target: " +
            Twine(targetSym.kind()));
      continue;
    }
    assert(locationSym->isSection());
    auto *locationDef = cast<Defined>(locationSym);
    auto *locationSec = cast<InputSectionBase>(locationDef->section);
    addCapReloc({locationSec, (uint64_t)locationRel.r_addend}, realTarget,
                targetNeedsDynReloc, targetCapabilityOffset,
                realLocation.sym());
  }
}

template <class ELFT>
void CheriCapRelocsSection<ELFT>::addCapReloc(CheriCapRelocLocation loc,
                                              const SymbolAndOffset &target,
                                              bool targetNeedsDynReloc,
                                              int64_t capabilityOffset,
                                              Symbol *sourceSymbol) {
  if (config->relativeCapRelocsOnly) {
    if (targetNeedsDynReloc) {
      error("Cannot add __cap_reloc against target that needs a dynamic "
            "relocation when --relative-cap-relocs is enabled: " +
            target.verboseToString());
      return;
    }
  } else {
    // Allow relocations in __cap_relocs section for legacy mode
    targetNeedsDynReloc = targetNeedsDynReloc || config->isPic || config->pie;
  }
  uint64_t currentEntryOffset = relocsMap.size() * relocSize;

  auto sourceMsg = [&]() -> std::string {
    return sourceSymbol ? verboseToString(sourceSymbol) : loc.toString();
  };
  if (target.sym()->isUndefined() && !target.sym()->isUndefWeak()) {
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

  if (!addEntry(loc, {target, capabilityOffset, targetNeedsDynReloc})) {
    return; // Maybe happens with vtables?
  }
  if (targetNeedsDynReloc) {
#ifdef DEBUG_CAP_RELOCS
    message("Adding dyn reloc at " + toString(this) + "+0x" +
            utohexstr(CurrentEntryOffset) + " against " +
            Target.verboseToString());
    message("Symbol preemptible:" + Twine(Target.Sym->IsPreemptible));
#endif

    bool relativeToLoadAddress = false;
    // The addend is not used as the offset into the capability here, as we
    // have the offset field in the __cap_relocs for that. The Addend
    // will be zero unless we are targeting a string constant as these
    // don't have a symbol and will be like .rodata.str+0x1234
    int64_t addend = target.offset;
    // Capability target is the second field
    if (target.sym()->isPreemptible) {
      mainPart->relaDyn->addSymbolReloc(
          *elf::target->absPointerRel, this, currentEntryOffset + fieldSize,
          *target.sym(), addend, lld::elf::target->symbolicRel);
    } else {
      // If the target is not preemptible we can optimize this to a relative
      // relocation against the image base.
      relativeToLoadAddress = true;
      mainPart->relaDyn->addRelativeReloc(
          elf::target->relativeRel, this, currentEntryOffset + fieldSize,
          *target.sym(), addend, lld::elf::target->symbolicRel, R_ABS);
    }
    assert((currentEntryOffset + fieldSize) < getSize());
    containsDynamicRelocations = true;
    if (!relativeToLoadAddress) {
      // We also add a size relocation for the size field here
      RelType sizeRel = *elf::target->sizeRel;
      // Capability size is the fourth field
      assert((currentEntryOffset + 3 * fieldSize) < getSize());
      mainPart->relaDyn->addSymbolReloc(
          sizeRel, this, currentEntryOffset + 3 * fieldSize, *target.sym());
    }
  }
}

template<typename ELFT>
static uint64_t getTargetSize(const CheriCapRelocLocation &location,
                              const CheriCapReloc &reloc, bool strict) {
  uint64_t targetSize = reloc.target.sym()->getSize();
  if (targetSize > INT_MAX) {
    error("Insanely large symbol size for " + reloc.target.verboseToString() +
          "for cap_reloc at" + location.toString());
    return 0;
  }
  auto targetSym = reloc.target.sym();
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
      // warn("Could not find size for symbol " + reloc.target.verboseToString() +
      //    " and could not determine section size. Using 0.");
      if ((int64_t)def->value < 0 || def->value > os->size) {
        // Note: we allow def->value == os->size for pointers to the end
        warn("Symbol " + reloc.target.verboseToString() +
             " is defined as being in section " + os->name +
             " but the value (0x" + utohexstr(targetSym->getVA()) +
             ") is outside this section. Will create a zero-size capability."
             "\n>>> referenced by " + location.toString());
        return 0;
      }
      // For negative offsets use 0 instead (we want the range of the full symbol in that case)
      int64_t offset = std::max((int64_t)0, reloc.target.offset);
      uint64_t targetVA = targetSym->getVA(offset);
      assert(targetVA >= os->addr);
      uint64_t offsetInOS = targetVA - os->addr;
      // Check this isn't a symbol defined outside a section in a linker script.
      // Use less-or-equal here to account for __end_foo symbols which point 1 past the section
      if (offsetInOS <= os->size) {
        targetSize = os->size - offsetInOS;
#ifdef DEBUG_CAP_RELOCS
        if (Config->verboseCapRelocs)
            errs() << " OS OFFSET 0x" << utohexstr(OS->Addr) << "SYM OFFSET 0x"
                   << utohexstr(OffsetInOS) << " SECLEN 0x" << utohexstr(OS->Size)
                   << " -> target size 0x" << utohexstr(TargetSize) << "\n";
#endif
        UnknownSectionSize = false;
      }
    }
    if (warnAboutUnknownSize || errorHandler().verbose) {
      std::string msg = "could not determine size of cap reloc against " +
          reloc.target.verboseToString() +
          "\n>>> referenced by " + location.toString();
      if (strict)
        warn(msg);
      else
        nonFatalWarning(msg);
    }
    if (UnknownSectionSize) {
      warn("Could not find size for symbol " + reloc.target.verboseToString() +
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

template <class ELFT> void CheriCapRelocsSection<ELFT>::writeTo(uint8_t *buf) {
  static_assert(relocSize == sizeof(InMemoryCapRelocEntry<ELFT>),
                "cap relocs size mismatch");
  uint64_t offset = 0;
  for (const auto &i : relocsMap) {
    const CheriCapRelocLocation &location = i.first;
    const CheriCapReloc &reloc = i.second;
    assert(location.offset <= location.section->getSize());
    // We write the virtual address of the location in in both static and the
    // shared library case:
    // In the static case we can compute the final virtual address and write it
    // In the dynamic case we write the virtual address relative to the load
    // address and the runtime linker will add the load address to that

    // CheriOS has a more complicated segment relative encoding that subtracts a
    // segment base and also outputs a segment index

    Symbol *targetSym = reloc.target.sym();

    bool targetsTLS = targetSym->isTls();
    bool targetNoOutput = targetSym->getOutputSection() == nullptr;

    OutputSection *locOutputSec = location.section->getOutputSection();
    OutputSection *targetOutputSec = targetSym->getOutputSection();
    // FIXME this assert gets hit if you delete the captable in your linker
    // script but then define symbols that go there.
    assert(locOutputSec != nullptr);

    PhdrEntry *locationPHDR, *targetPHDR;

    uint64_t outSecOffset = location.section->getOffset(location.offset);
    uint64_t locationVA = locOutputSec->addr + outSecOffset;

    if (config->isCheriOS()) {
      locationPHDR = targetsTLS ? locOutputSec->ptTLS : locOutputSec->ptLoad;
      targetPHDR = targetNoOutput ? nullptr
                                  : (targetsTLS ? targetOutputSec->ptTLS
                                                : targetOutputSec->ptLoad);
      assert(locationPHDR != nullptr &&
             "Could not find a program header for relocation location");

      uint64_t OutSegOffset = locationPHDR->p_vaddr;
      locationVA -= OutSegOffset;
    }

    // For the target the virtual address the addend is always zero so
    // if we need a dynamic reloc we write zero
    // TODO: would it be more efficient for local symbols to write the DSO VA
    // and add a relocation against the load address?
    // Also this would make llvm-objdump --cap-relocs more useful because it
    // would actually display the symbol that the relocation is against
    uint64_t targetVA = targetSym->getVA(reloc.target.offset);
    bool preemptibleDynReloc = reloc.needsDynReloc && targetSym->isPreemptible;
    uint64_t targetSize = 0;
    if (preemptibleDynReloc) {
      // If we have a relocation against a preemptible symbol (even in the
      // current DSO) we can't compute the virtual address here so we only write
      // the addend
      if (reloc.target.offset != 0)
        error("Dyn Reloc Target offset was nonzero: " +
              Twine(reloc.target.offset) + " - " +
              reloc.target.verboseToString());
      targetVA = reloc.target.offset;
    } else {
      // For non-preemptible symbols we can write the target size:
      targetSize = getTargetSize<ELFT>(location, reloc,
                                       /*strict=*/!containsLegacyCapRelocs());
    }

    uint64_t permissions = 0;
    // Fow now Function implies ReadOnly so don't add the flag
    if (targetSym->isFunc()) {
      permissions |= CaptablePermissions<ELFT>::function;
    } else if (!config->isCheriOS() && targetOutputSec) {
      assert(!targetSym->isTls());
      // if ((OS->getPhdrFlags() & PF_W) == 0) {
      if (((targetOutputSec->flags & SHF_WRITE) == 0) ||
          isRelroSection(targetOutputSec)) {
        permissions |= CaptablePermissions<ELFT>::readOnly;
      } else if (targetOutputSec->flags & SHF_EXECINSTR) {
        warn("Non-function __cap_reloc against symbol in section with "
             "SHF_EXECINSTR (" +
             toString(targetOutputSec->name) + ") for symbol " +
             reloc.target.verboseToString());
      }
    }

    // Having no output segment is allowed to happen.
    // Sometimes symbols are not really VAs are used as a way to communicate
    // between a linker script and a program. This means they might not be in
    // any output segment.

    if (!targetsTLS && !targetNoOutput && config->isCheriOS()) {

      assert(targetPHDR != nullptr &&
             "Could not find a program header for relocation target");

      targetVA -= targetPHDR->p_vaddr;
    }

    uint64_t targetOffset = reloc.capabilityOffset;

    // TODO: should we warn about symbols that are out-of-bounds?
    // mandoc seems to do it so I guess we need it

    uint16_t flags = 0;
    if (targetsTLS)
      flags |= 1;
    if (targetNoOutput)
      flags |= 2;

    // TODO more accurate permission masks
    InMemoryCapRelocEntry<ELFT> entry =
        config->isCheriOS()
            ? InMemoryCapRelocEntry<ELFT>{locationVA,
                                          targetVA,
                                          targetOffset,
                                          targetSize,
                                          permissions,
                                          flags,
                                          GetEncodedIndexForSegment(
                                              locationPHDR),
                                          GetEncodedIndexForSegment(targetPHDR)}
            : InMemoryCapRelocEntry<ELFT>{locationVA, targetVA, targetOffset,
                                          targetSize, permissions};

    memcpy(buf + offset, &entry, sizeof(entry));
    //     if (errorHandler().Verbose) {
    //       errs() << "Added capability reloc: loc=" << utohexstr(locationVA)
    //              << ", object=" << utohexstr(TargetVA)
    //              << ", offset=" << utohexstr(TargetOffset)
    //              << ", size=" << utohexstr(TargetSize)
    //              << ", permissions=" << utohexstr(Permissions) << "\n";
    //     }
    offset += relocSize;
  }

  // FIXME: this totally breaks dynamic relocs!!! need to do in finalize()

  // Sort the cap_relocs by target address for better cache and TLB locality
  // It also makes it much easier to read the llvm-objdump -C output since it
  // is sorted in a sensible order
  // However, we can't do this if we added any dynamic relocations since it
  // will mean the dynamic relocation offset refers to a different location
  // FIXME: do the sorting in finalizeSection instead
  if (config->sortCapRelocs && !containsDynamicRelocations)
    std::stable_sort(reinterpret_cast<InMemoryCapRelocEntry<ELFT> *>(buf),
                     reinterpret_cast<InMemoryCapRelocEntry<ELFT> *>(buf + offset),
                     [](const InMemoryCapRelocEntry<ELFT> &a,
                        const InMemoryCapRelocEntry<ELFT> &b) {
                       return a.capability_location < b.capability_location;
                     });
  assert(offset == getSize() && "Not all data written?");
}

CheriCapTableSection::CheriCapTableSection(bool local)
    : SyntheticSection(SHF_ALLOC |
                           SHF_WRITE, /* XXX: actually RELRO for BIND_NOW*/
                       SHT_PROGBITS, config->capabilitySize,
                       local ? ".cap_table_local" : ".captable") {
  assert(config->capabilitySize > 0);
  this->entsize = config->capabilitySize;

  isLocal = local;

  if (local) {
    flags |= SHF_TLS;
  }

  if (local && config->isCheriOS()) {
    // CheriOS adds a few fixed offset entries to the thread local captable
    for (uint64_t i = 1; i <= 9; i++) {
      addFixedEntry(*(Symbol *)i, i - 1);
    }
  }
}

void CheriCapTableSection::writeTo(uint8_t* buf) {
  // Capability part should be filled with all zeros and crt_init_globals fills
  // it in. For the TLS part, assignValuesAndAddCapTableSymbols adds any static
  // relocations needed, and should be procesed by relocateAlloc.
  // TODO: Fill in the raw capability bits and use CBuildCap
  relocateAlloc(buf, buf + getSize());
}

static Defined *findMatchingFunction(const InputSectionBase *isec,
                                     uint64_t symOffset) {
  switch (config->ekind) {
  default:
    llvm_unreachable("Invalid kind");
  case ELF32LEKind:
    return isec->getEnclosingFunction<ELF32LE>(symOffset);
  case ELF32BEKind:
    return isec->getEnclosingFunction<ELF32BE>(symOffset);
  case ELF64LEKind:
    return isec->getEnclosingFunction<ELF64LE>(symOffset);
  case ELF64BEKind:
    return isec->getEnclosingFunction<ELF64BE>(symOffset);
  }
}

CheriCapTableSection::CaptableMap &
CheriCapTableSection::getCaptableMapForFileAndOffset(
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

void CheriCapTableSection::addEntry(Symbol &sym, RelExpr expr,
                                    InputSectionBase *isec, uint64_t offset) {
  // FIXME: can this be called from multiple threads?
  CapTableIndex idx;
  idx.needsSmallImm = false;
  idx.usedInCallExpr = false;
  idx.isFixed = false;
  idx.priority = -1;
  idx.firstUse = SymbolAndOffset::fromSectionWithOffset(isec, offset);
  assert(!idx.firstUse->symOrSec.isNull());
  switch (expr) {
  case R_CHERI_CAPABILITY_TABLE_INDEX_SMALL_IMMEDIATE:
  case R_CHERI_CAPABILITY_TABLE_INDEX_CALL_SMALL_IMMEDIATE:
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
  case R_CHERI_CAPABILITY_TABLE_INDEX_CALL:
  case R_CHERI_CAPABILITY_TABLE_INDEX_CALL_SMALL_IMMEDIATE:
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
    StringRef helperSymName = saver.save("$captable_load_" + (sym.getName().empty() ? "$anonymous_symbol" : sym.getName()));
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
  } else {
    entries.unfixedEntries++;
    if (idx.needsSmallImm) {
      entries.smallEntries++;
    }
  }
#if defined(DEBUG_CAP_TABLE)
  std::string DbgContext;
  if (Config->CapTableScope == CapTableScopePolicy::File) {
    DbgContext = " for file '" + toString(IS->File) + "'";
  } else if (Config->CapTableScope == CapTableScopePolicy::Function) {
    DbgContext =  " for function '" + toString(*findMatchingFunction(IS, Offset)) + "'";
  }
  llvm::errs() << "Added symbol " << toString(Sym) << " to .captable"
               << DbgContext << ". Total count " << Entries.size() << "\n";
#endif
}

void CheriCapTableSection::addFixedEntry(Symbol &sym, uint64_t index) {
  assert((config->capTableScope == CapTableScopePolicy::All));
  CaptableMap &entries = globalEntries;
  CapTableIndex idx;
  idx.isFixed = true;
  idx.index = index;
  idx.priority = -1;
  idx.usedInCallExpr = false;
  idx.needsSmallImm = true;
  bool added = entries.map.insert(std::make_pair(&sym, idx)).second;
  assert(added);
  entries.fixedEntries++;
  entries.smallEntries++;
}

void CheriCapTableSection::addDynTlsEntry(Symbol &sym) {
  dynTlsEntries.map.insert(std::make_pair(&sym, CapTableIndex()));
}

void CheriCapTableSection::addTlsIndex() {
  dynTlsEntries.map.insert(std::make_pair(nullptr, CapTableIndex()));
}

void CheriCapTableSection::addTlsEntry(Symbol &sym) {
  tlsEntries.map.insert(std::make_pair(&sym, CapTableIndex()));
}

uint32_t CheriCapTableSection::getIndex(const Symbol &sym,
                                        const InputSectionBase *isec,
                                        uint64_t offset) const {
  assert(valuesAssigned && "getIndex called before index assignment");
  const CaptableMap &entries =
      const_cast<CheriCapTableSection *>(this)->getCaptableMapForFileAndOffset(
          isec, offset);
  auto it = entries.map.find(const_cast<Symbol *>(&sym));
  assert(entries.firstIndex != std::numeric_limits<uint64_t>::max() &&
         "First index not set yet?");
  assert(it != entries.map.end());
  // The index that is written as part of the relocation is relative to the
  // start of the current captable subset (or the global table in the default
  // case). When using per-function tables the first index in every function
  // will always be zero.
#if defined(DEBUG_CAP_TABLE)
  message("captable index for " + toString(Sym) + " is " +
          Twine(it->second.Index.getValue()) + " - " +
          Twine(Entries.FirstIndex) + ": " +
          Twine(it->second.Index.getValue() - Entries.FirstIndex));
#endif
  return it->second.index.getValue() - entries.firstIndex;
}

uint32_t CheriCapTableSection::getDynTlsOffset(const Symbol &sym) const {
  assert(valuesAssigned && "getDynTlsOffset called before index assignment");
  auto it = dynTlsEntries.map.find(const_cast<Symbol *>(&sym));
  assert(it != dynTlsEntries.map.end());
  return it->second.index.getValue() * config->wordsize;
}

uint32_t CheriCapTableSection::getTlsIndexOffset() const {
  assert(valuesAssigned && "getTlsIndexOffset called before index assignment");
  auto it = dynTlsEntries.map.find(nullptr);
  assert(it != dynTlsEntries.map.end());
  return it->second.index.getValue() * config->wordsize;
}

uint32_t CheriCapTableSection::getTlsOffset(const Symbol &sym) const {
  assert(valuesAssigned && "getTlsOffset called before index assignment");
  auto it = tlsEntries.map.find(const_cast<Symbol *>(&sym));
  assert(it != tlsEntries.map.end());
  return it->second.index.getValue() * config->wordsize;
}

struct sort_pair {
  uint32_t vector_index;
  int32_t priority; // priority of being placed in the cap table (larger more
                    // priority)
};

void CheriCapTableSection::assignCheriOSIndices(uint64_t startIndex,
                                                CaptableMap &entries) {
  uint64_t fixedEntries = entries.fixedEntries;
  uint64_t unfixedEntries = entries.unfixedEntries;
  // uint64_t smallEntries = entries.smallEntries;

  // TODO allow per function cap tables as well
  assert(startIndex == 0);

  uint64_t totalEntries = unfixedEntries + fixedEntries;

  assert(totalEntries == entries.size());

  // Hopefully we have done all lazy resolution now.
  // Add the remaining symbols to a dummy table to not cause any output, but so
  // we can still inspect them.
  if (!CheriCapTableSection::dummySymTab) {
    SymbolTable *mainTable = symtab;
    CheriCapTableSection::dummySymTab = make<SymbolTable>();
    symtab = CheriCapTableSection::dummySymTab; // And THIS is why we don't use
                                                // globals kids
    // Process all of the archives
    for (ArchiveFile *arc : mainTable->getArchives()) {
      arc->fetchRemaining(CheriCapTableSection::dummySymTab);
    }
    symtab = mainTable;
  }

  // To avoid any double counting
  llvm::DenseMap<Symbol *, ArchiveFile *> symArcMap;

  // Process each archive and assign preferred indices to symbols found within
  // based on order within the file
  for (ArchiveFile *arc : symtab->getArchives()) {

    uint32_t archiveIndex = 0;
    int32_t prio = (int32_t)arc->groupId;

    for (auto child : arc->getChildren()) {
      InputFile *file = child.second;
      for (Symbol *s : file->getSymbols()) {

        Symbol *dummyS = dummySymTab->find(s->getName());
        Symbol *mainS = symtab->find(s->getName());

        if (dummyS != nullptr && mainS != nullptr) {
          // Due to the way we add symbols to the aux table, they may be
          // duplicated If we find two we need to choose a canonical one We
          // prefer to take the used symbol, unless it is lazy, then we take the
          // dummy
          s = mainS->isLazy() ? dummyS : mainS;
        }

        if (s->isTls() != isLocal)
          continue; // Only handle symbols from the correct table kind

        // Don't double count usage of the same symbol in the same archive
        auto it = symArcMap.insert(std::make_pair(s, arc));
        if (!it.second) {
          if (it.first->second == arc)
            continue;
          it.first->second = arc;
        }

        // Only give symbols that are likely to be used a preferred index
        bool likely =
            (s->isUndefWeak() || s->isTls() || s->isObject() ||
             (s->isFunc() && !s->getName().startswith("__cross_domain")) ||
             (s->isDefined() && s->isLocal() &&
              s->getName().startswith(".LJTI"))) &&
            !(s->isFile() || s->isSection());
        uint32_t pref = likely ? (archiveIndex++) : (uint32_t)(~0);

        auto sEnt = entries.map.find(s);
        if (sEnt != entries.map.end()) {
          if (mainS) {
            assert(s == mainS); // We should only see symbols from the main
                                // symtab need entries.
          }
          if (prio > (*sEnt).second.priority) {
            (*sEnt).second.index = pref;
            (*sEnt).second.priority = prio;
          }
        }
      }
    }
  }

  // A map if which slots are taken.
  llvm::SmallVector<bool, 0x1000> taken_map(totalEntries, false);

  // Every slot is taken until this one
  size_t taken_till = 0;

  // First create a map that we can sort efficiently
  llvm::SmallVector<sort_pair, 0x1000> sortMap(unfixedEntries, {0, 0});

  uint32_t assigned = 0;
  uint32_t vector_index = 0;
  uint32_t map_index = 0;

  for (auto &it : entries.map) {
    if (it.second.isFixed) {
      // Is fixed
      assert(it.second.index.hasValue());
      assert(it.second.index.getValue() < totalEntries);
      assert(!taken_map[it.second.index.getValue()]);
      taken_map[it.second.index.getValue()] = true;
      assigned++;
    } else {
      // Has a preferred index
      if (it.second.index.hasValue()) {
        assert(map_index < unfixedEntries);
        sortMap[map_index].vector_index = vector_index;
        sortMap[map_index].priority = it.second.priority;
        map_index++;
      }
    }
    vector_index++;
  }

  // TODO: Currently sort of ignoring need small immediate thing and just
  // assuming everything can use one.

  // Then sort the map. Give priority for placement first.
  llvm::sort(sortMap.begin(), sortMap.end(),
             [](sort_pair a, sort_pair b) -> bool {
               return a.priority > b.priority;
             }); // Items that are seen last come first

  // Now assign the indices.

  // First give preferred positions.
  for (uint32_t i = 0; i != map_index; i++) {
    auto &entry = (entries.map.begin() + sortMap[i].vector_index)->second;
    assert(entry.index.hasValue());
    uint32_t position = entry.index.getValue();
    if (position == (uint32_t)(~0))
      continue;
    position += fixedEntries; // Fixed entries tend to go first and be of
                              // constant size. Bump everything up.

    if (position < totalEntries && !taken_map[position]) {
      assigned++;
      taken_map[position] = true;
      entry.index = position;
    } else {
      entry.index.reset();
    }
  }

  // Then by filling in any gaps.
  for (auto &it : entries.map) {
    auto &entry = it.second;
    if (!entry.index.hasValue() || (entry.index.getValue() == (uint32_t)(~0))) {
      while (taken_map[taken_till]) {
        taken_till++;
        assert(taken_till < totalEntries);
      }
      assigned++;
      taken_map[taken_till] = true;
      entry.index = taken_till;
      taken_till++;
    }
    assert(entry.index.hasValue() && entry.index.getValue() < totalEntries);
    assert(taken_map[entry.index.getValue()]);
  }

  assert(assigned == totalEntries);
}

template <class ELFT>
uint64_t CheriCapTableSection::assignIndices(uint64_t startIndex,
                                             CaptableMap &entries,
                                             const Twine &symContext) {
  // Usually StartIndex will be zero (one global captable) but if we are
  // compiling with per-file/per-function
  uint64_t smallEntryCount = 0;
  assert(entries.firstIndex == std::numeric_limits<uint64_t>::max() &&
         "Should not be initialized yet!");
  entries.firstIndex = startIndex;

  smallEntryCount = entries.smallEntries;

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

  // CheriOS tries to assign indices in a deterministic way based on the order
  // symbols appear in the input files This results in more deterministic
  // compilation, but is just an optimisation.
  if (config->isCheriOS()) {
    assignCheriOSIndices(startIndex, entries);
  }

  // Only add the @CAPTABLE symbols when running the LLD unit tests
  // errorHandler().exitEarly is set to false if LLD_IN_TEST=1 so just reuse
  // that instead of calling getenv on every iteration
  const bool shouldAddAtCaptableSymbols = !errorHandler().exitEarly;
  uint32_t assignedSmallIndexes = 0;
  uint32_t assignedLargeIndexes = 0;
  for (auto &it : entries.map) {
    CapTableIndex &cti = it.second;

    if (!config->isCheriOS()) {
      if (cti.needsSmallImm) {
        assert(assignedSmallIndexes < smallEntryCount);
        cti.index = startIndex + assignedSmallIndexes;
        assignedSmallIndexes++;
      } else {
        cti.index = startIndex + smallEntryCount + assignedLargeIndexes;
        assignedLargeIndexes++;
      }
    } else {
      assert(cti.index.hasValue());
      assignedSmallIndexes++;
      if (cti.isFixed)
        continue;
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
      refName = saver.save(name + "@CAPTABLE" + symContext + "." + Twine(index));
    else
      refName = saver.save(name + "@CAPTABLE" + symContext);
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
    RelocationBaseSection *dynRelSec =
        it.second.usedInCallExpr ? in.relaPlt : mainPart->relaDyn;
    addCapabilityRelocation<ELFT>(
        targetSym, elfCapabilityReloc, this, off, R_CHERI_CAPABILITY, 0,
        it.second.usedInCallExpr,
        [&]() {
          return ("\n>>> referenced by " + refName + "\n>>> first used in " +
                  it.second.firstUse->verboseToString())
              .str();
        },
        dynRelSec);
  }
  assert(assignedSmallIndexes + assignedLargeIndexes == entries.size());
  return assignedSmallIndexes + assignedLargeIndexes;
}

template <class ELFT>
void CheriCapTableSection::assignValuesAndAddCapTableSymbols() {
  // First assign the global indices (which will usually be the only ones)
  uint64_t assignedEntries = assignIndices<ELFT>(0, globalEntries, "");
  if (LLVM_UNLIKELY(config->capTableScope != CapTableScopePolicy::All)) {
    assert(assignedEntries == 0 && "Should not have any global entries in"
                                   " per-file/per-function captable mode");
    for (auto &it : perFileEntries) {
      std::string fullContext = toString(it.first);
      auto lastSlash = StringRef(fullContext).find_last_of("/\\") + 1;
      StringRef context = StringRef(fullContext).substr(lastSlash);
      assignedEntries += assignIndices<ELFT>(assignedEntries, it.second,
                                             "@" + context);
    }
    for (auto &it : perFunctionEntries)
      assignedEntries += assignIndices<ELFT>(assignedEntries, it.second,
                                             "@" + toString(*it.first));
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
    uint64_t offset = cti.index.getValue() * config->wordsize;
    if (s == nullptr) {
      if (!config->shared)
        this->relocations.push_back(
            {R_ADDEND, target->symbolicRel, offset, 1, s});
      else
        mainPart->relaDyn->addReloc({target->tlsModuleIndexRel, this, offset});
    } else {
      // When building a shared library we still need a dynamic relocation
      // for the module index. Therefore only checking for
      // s->isPreemptible is not sufficient (this happens e.g. for
      // thread-locals that have been marked as local through a linker script)
      if (!s->isPreemptible && !config->shared)
        this->relocations.push_back(
            {R_ADDEND, target->symbolicRel, offset, 1, s});
      else
        mainPart->relaDyn->addSymbolReloc(target->tlsModuleIndexRel, this,
                                          offset, *s);

      offset += config->wordsize;

      // However, we can skip writing the TLS offset reloc for non-preemptible
      // symbols since it is known even in shared libraries
      if (!s->isPreemptible)
        this->relocations.push_back(
            {R_ABS, target->tlsOffsetRel, offset, 0, s});
      else
        mainPart->relaDyn->addSymbolReloc(target->tlsOffsetRel, this, offset,
                                          *s);
    }
  }

  for (auto &it : tlsEntries.map) {
    CapTableIndex &cti = it.second;
    assert(!cti.needsSmallImm);
    cti.index = tlsBaseIndex + assignedTlsIndexes++;
    Symbol *s = it.first;
    uint64_t offset = cti.index.getValue() * config->wordsize;
    // When building a shared library we still need a dynamic relocation
    // for the TP-relative offset as we don't know how much other data will
    // be allocated before us in the static TLS block.
    if (!s->isPreemptible && !config->shared)
      this->relocations.push_back({R_TPREL, target->symbolicRel, offset, 0, s});
    else
      mainPart->relaDyn->addAddendOnlyRelocIfNonPreemptible(
          target->tlsGotRel, this, offset, *s, target->symbolicRel);
  }

  valuesAssigned = true;
}

SymbolTable *CheriCapTableSection::dummySymTab = nullptr;

template class elf::CheriCapRelocsSection<ELF32LE>;
template class elf::CheriCapRelocsSection<ELF32BE>;
template class elf::CheriCapRelocsSection<ELF64LE>;
template class elf::CheriCapRelocsSection<ELF64BE>;

template void
CheriCapTableSection::assignValuesAndAddCapTableSymbols<ELF32LE>();
template void
CheriCapTableSection::assignValuesAndAddCapTableSymbols<ELF32BE>();
template void
CheriCapTableSection::assignValuesAndAddCapTableSymbols<ELF64LE>();
template void
CheriCapTableSection::assignValuesAndAddCapTableSymbols<ELF64BE>();

CheriCapTableMappingSection::CheriCapTableMappingSection()
    : SyntheticSection(SHF_ALLOC, SHT_PROGBITS, 8, ".captable_mapping") {
  assert(config->capabilitySize > 0);
  this->entsize = sizeof(CaptableMappingEntry);
  static_assert(sizeof(CaptableMappingEntry) == 24, "");
}

size_t CheriCapTableMappingSection::getSize() const {
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

void CheriCapTableMappingSection::writeTo(uint8_t *buf) {
  assert(config->capTableScope != CapTableScopePolicy::All);
  if (!in.cheriCapTable)
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
    const CheriCapTableSection::CaptableMap *capTableMap = nullptr;
    if (config->capTableScope == CapTableScopePolicy::Function) {
      auto it = in.cheriCapTable->perFunctionEntries.find(sym);
      if (it != in.cheriCapTable->perFunctionEntries.end())
        capTableMap = &it->second;
    } else if (config->capTableScope == CapTableScopePolicy::File) {
      auto it = in.cheriCapTable->perFileEntries.find(sym->file);
      if (it != in.cheriCapTable->perFileEntries.end())
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
void addCapabilityRelocation(Symbol *sym, RelType type, InputSectionBase *sec,
                             uint64_t offset, RelExpr expr, int64_t addend,
                             bool isCallExpr,
                             llvm::function_ref<std::string()> referencedBy,
                             RelocationBaseSection *dynRelSec) {
  assert(expr == R_CHERI_CAPABILITY);
  if (sec->name == ".gcc_except_table" && sym->isPreemptible) {
    // We previously had an ugly workaround here to create a hidden alias for
    // relocations in the exception table, but this has since been fixed in
    // the compiler. Add an explicit error here in case someone tries to
    // link against object files/static libraries from an old toolchain.
    errorOrWarn("got relocation against preemptible symbol " + toString(*sym) +
                " in exception handling table. Please recompile this file!\n"
                ">>> referenced by " +
                sec->getObjMsg(offset));
  }

  // Emit either the legacy __cap_relocs section or a R_CHERI_CAPABILITY reloc
  // For local symbols we can also emit the untagged capability bits and
  // instruct csu/rtld to run CBuildCap
  CapRelocsMode capRelocMode = sym->isPreemptible
                                   ? config->preemptibleCapRelocsMode
                                   : config->localCapRelocsMode;
  bool needTrampoline = false;
  // In the PLT ABI (and fndesc?) we have to use an elf relocation for function
  // pointers to ensure that the runtime linker adds the required trampolines
  // that sets $cgp:
  if (!config->isCheriOS() && !isCallExpr &&
      config->emachine == llvm::ELF::EM_MIPS && sym->isFunc()) {
    if (!lld::elf::hasDynamicLinker()) {
      // In static binaries we do not need PLT stubs for function pointers since
      // all functions share the same $cgp
      // TODO: this is no longer true if we were to support dlopen() in static
      // binaries
      if (config->verboseCapRelocs)
        message("Do not need function pointer trampoline for " +
                toString(*sym) + " in static binary");
      needTrampoline = false;
    } else if (auto abi = InX<ELFT>::mipsAbiFlags->getCheriAbiVariant()) {
      if (*abi == llvm::ELF::DF_MIPS_CHERI_ABI_PLT ||
          *abi == llvm::ELF::DF_MIPS_CHERI_ABI_FNDESC)
        needTrampoline = true;
    }
  }

  if (needTrampoline) {
    capRelocMode = CapRelocsMode::ElfReloc;
    assert(capRelocMode == config->preemptibleCapRelocsMode);
    if (config->verboseCapRelocs)
      message("Using trampoline for function pointer against " +
              verboseToString(sym));
  }

  // local cap relocs don't need a Elf relocation with a full symbol lookup:
  if (capRelocMode == CapRelocsMode::ElfReloc) {
    assert((sym->isPreemptible || needTrampoline) &&
           "ELF relocs should not be used for non-preemptible symbols");
    assert((!sym->isLocal() || needTrampoline) &&
           "ELF relocs should not be used for local symbols");
    if (config->emachine == llvm::ELF::EM_MIPS && config->buildingFreeBSDRtld) {
      error("relocation " + toString(type) + " against " +
            verboseToString(sym) +
            " cannot be using when building FreeBSD RTLD" + referencedBy());
      return;
    }
    if (!lld::elf::hasDynamicLinker()) {
      error("attempting to emit a R_CAPABILITY relocation against " +
            (sym->getName().empty() ? "local symbol"
                                    : "symbol " + toString(*sym)) +
            " in binary without a dynamic linker; try removing -Wl,-" +
            (sym->isPreemptible ? "preemptible" : "local") + "-caprelocs=elf" +
            referencedBy());
      return;
    }
    assert(config->hasDynSymTab && "Should have been checked in Driver.cpp");
    // We don't use a R_MIPS_CHERI_CAPABILITY relocation for the input but
    // instead need to use an absolute pointer size relocation to write
    // the offset addend
    if (!dynRelSec)
      dynRelSec = mainPart->relaDyn;
    // in the case that -local-caprelocs=elf is passed we need to ensure that
    // the target symbol is included in the dynamic symbol table
    if (!mainPart->dynSymTab) {
      error("R_CHERI_CAPABILITY relocations need a dynamic symbol table");
      return;
    }
    if (!sym->includeInDynsym()) {
      if (!needTrampoline) {
        error("added a R_CHERI_CAPABILITY relocation but symbol not included "
              "in dynamic symbol: " +
              verboseToString(sym));
        return;
      }
      // Hack: Add a new global symbol with a unique name so that we can use
      // a dynamic relocation against it.
      // TODO: should it be possible to add STB_LOCAL symbols to .dynsymtab?
      Defined* newSym = symtab->ensureSymbolWillBeInDynsym(sym);
      assert(newSym->isFunc() && "This should only be used for functions");
      assert(newSym->includeInDynsym());
      assert(newSym->binding == llvm::ELF::STB_GLOBAL);
      assert(newSym->visibility == llvm::ELF::STV_HIDDEN);
      sym = newSym; // Make the relocation point to the newly added symbol
    }
    dynRelSec->addReloc(
        DynamicReloc::AgainstSymbol, type, sec, offset, *sym, addend, expr,
        /* Relocation type for the addend = */ target->symbolicRel);

  } else if (capRelocMode == CapRelocsMode::Legacy) {
    if (config->relativeCapRelocsOnly) {
      assert(!sym->isPreemptible);
    }
    InX<ELFT>::capRelocs->addCapReloc({sec, offset}, {sym, 0u},
                                      sym->isPreemptible, addend);
  } else {
    assert(config->localCapRelocsMode == CapRelocsMode::CBuildCap);
    error("CBuildCap method not implemented yet!");
  }
}

} // namespace elf
} // namespace lld

template void lld::elf::addCapabilityRelocation<ELF32LE>(
    Symbol *, RelType, InputSectionBase *, uint64_t, RelExpr, int64_t, bool,
    llvm::function_ref<std::string()>, RelocationBaseSection *);
template void lld::elf::addCapabilityRelocation<ELF32BE>(
    Symbol *, RelType, InputSectionBase *, uint64_t, RelExpr, int64_t, bool,
    llvm::function_ref<std::string()>, RelocationBaseSection *);
template void lld::elf::addCapabilityRelocation<ELF64LE>(
    Symbol *, RelType, InputSectionBase *, uint64_t, RelExpr, int64_t, bool,
    llvm::function_ref<std::string()>, RelocationBaseSection *);
template void lld::elf::addCapabilityRelocation<ELF64BE>(
    Symbol *, RelType, InputSectionBase *, uint64_t, RelExpr, int64_t, bool,
    llvm::function_ref<std::string()>, RelocationBaseSection *);
