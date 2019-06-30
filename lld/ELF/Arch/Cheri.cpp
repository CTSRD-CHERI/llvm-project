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

// See CheriBSD crt_init_globals()
template <class ELFT> struct InMemoryCapRelocEntry {
  using NativeUint = typename ELFT::uint;
  using CapRelocUint = llvm::support::detail::packed_endian_specific_integral<
      NativeUint, ELFT::TargetEndianness, llvm::support::aligned>;
  InMemoryCapRelocEntry(NativeUint Loc, NativeUint Obj, NativeUint Off,
                        NativeUint S, NativeUint Perms)
      : capability_location(Loc), object(Obj), offset(Off), size(S),
        permissions(Perms) {}
  CapRelocUint capability_location;
  CapRelocUint object;
  CapRelocUint offset;
  CapRelocUint size;
  CapRelocUint permissions;
};

template <class ELFT>
CheriCapRelocsSection<ELFT>::CheriCapRelocsSection()
    : SyntheticSection((Config->Pic && !Config->RelativeCapRelocsOnly)
                           ? SHF_ALLOC | SHF_WRITE /* XXX: actually RELRO */
                           : SHF_ALLOC,
                       SHT_PROGBITS, 8, "__cap_relocs") {
  this->Entsize = RelocSize;
}

// TODO: copy MipsABIFlagsSection::create() instead of current impl?
template <class ELFT>
void CheriCapRelocsSection<ELFT>::addSection(InputSectionBase *S) {
  // FIXME: can this happen with ld -r ?
  // error("Compiler should not have generated __cap_relocs section for " + toString(S));
  assert(S->Name == "__cap_relocs");
  assert(S->AreRelocsRela && "__cap_relocs should be RELA");
  // make sure the section is no longer processed
  S->markDead();

  if ((S->getSize() % Entsize) != 0) {
    error("__cap_relocs section size is not a multiple of " + Twine(Entsize) +
          ": " + toString(S));
    return;
  }
  size_t NumCapRelocs = S->getSize() / RelocSize;
  if (NumCapRelocs * 2 != S->NumRelocations) {
    error("expected " + Twine(NumCapRelocs * 2) + " relocations for " +
          toString(S) + " but got " + Twine(S->NumRelocations));
    return;
  }
  if (Config->VerboseCapRelocs)
    message("Adding cap relocs from " + toString(S->File) + "\n");

  LegacyInputs.push_back(S);
}

template <class ELFT> void CheriCapRelocsSection<ELFT>::finalizeContents() {
  for (InputSectionBase *S : LegacyInputs) {
    if (Config->VerboseCapRelocs)
      message("Processing legacy cap relocs from " + toString(S->File) + "\n");
    processSection(S);
  }
}

static inline void nonFatalWarning(const Twine &Str) {
  if (errorHandler().FatalWarnings)
    message("warning: " + Str);
  else
    warn(Str);
}

SymbolAndOffset SymbolAndOffset::fromSectionWithOffset(InputSectionBase *IS,
                                                       uint64_t Offset,
                                                       Symbol *Default) {
  Symbol *FallbackResult = nullptr;
  assert((int64_t)Offset >= 0);
  uint64_t FallbackOffset = Offset;
  // For internal symbols we don't have a matching InputFile, just return
  auto* File = IS->File;
  if (!File)
    return {Default, (int64_t)Offset};
  for (Symbol *B : IS->File->getSymbols()) {
    if (auto *D = dyn_cast<Defined>(B)) {
      if (D->Section != IS)
        continue;
      if (D->Value <= Offset && Offset <= D->Value + D->Size) {
        // XXXAR: should we accept any symbol that encloses or only exact
        // matches?
        if (D->Value == Offset && (D->isFunc() || D->isObject()))
          return {D, 0}; // perfect match
        FallbackResult = D;
        FallbackOffset = Offset - D->Value;
      }
    }
  }
  // When using the legacy __cap_relocs style (where clang emits __cap_relocs
  // instead of R_CHERI_CAPABILITY) the local symbols might not exist so we
  // may have fall back to the section.
  if (!FallbackResult) {
    // worst case we fall back to the section + offset
    // Don't warn if the relocation is against an anonymous string constant
    // since clang won't emit a symbol (and no size) for those
    if (!IS->Name.startswith(".rodata.str"))
      nonFatalWarning("Could not find a real symbol for __cap_reloc against " + IS->Name +
           "+0x" + utohexstr(Offset) + " in " + toString(IS->File));
    FallbackResult = Default;
  }
  // we should have found at least a section symbol
  if (!FallbackResult) {
    // Could not find a symbol -> return section+offset
    return {IS, (int64_t)FallbackOffset};
  }
  return {FallbackResult, (int64_t)FallbackOffset};
}

SymbolAndOffset SymbolAndOffset::findRealSymbol() const {
  if (!SymOrSec.is<Symbol *>())
    return *this;
  auto S = SymOrSec.get<Symbol *>();
  if (!S->isSection())
    return *this;

  if (Defined *DefinedSym = dyn_cast<Defined>(S)) {
    if (auto *IS = dyn_cast<InputSectionBase>(DefinedSym->Section)) {
      return SymbolAndOffset::fromSectionWithOffset(IS, Offset, S);
    }
  }
  return *this;
}

std::string CheriCapRelocLocation::toString() const {
  SymbolAndOffset Resolved =
      SymbolAndOffset::fromSectionWithOffset(Section, Offset);
 return Resolved.verboseToString();
}

template <class ELFT>
void CheriCapRelocsSection<ELFT>::processSection(InputSectionBase *S) {
  // TODO: sort by offset (or is that always true?
  const auto Rels = S->relas<ELFT>();
  for (auto I = Rels.begin(), End = Rels.end(); I != End; ++I) {
    const auto &LocationRel = *I;
    ++I;
    const auto &TargetRel = *I;
    if ((LocationRel.r_offset % Entsize) != 0) {
      error("corrupted __cap_relocs:  expected Relocation offset to be a "
            "multiple of " +
            Twine(Entsize) + " but got " + Twine(LocationRel.r_offset));
      return;
    }
    if (TargetRel.r_offset != LocationRel.r_offset + FieldSize) {
      error("corrupted __cap_relocs: expected target relocation (" +
            Twine(TargetRel.r_offset) +
            " to directly follow location relocation (" +
            Twine(LocationRel.r_offset) + ")");
      return;
    }
    if (LocationRel.r_addend < 0) {
      error("corrupted __cap_relocs: addend is less than zero in" +
            toString(S) + ": " + Twine(LocationRel.r_addend));
      return;
    }
    uint64_t CapRelocsOffset = LocationRel.r_offset;
    assert(CapRelocsOffset + Entsize <= S->getSize());
    if (Config->EMachine == EM_MIPS) {
      if (LocationRel.getType(Config->IsMips64EL) != R_MIPS_64) {
        error("Exptected a R_MIPS_64 relocation in __cap_relocs but got " +
              toString(LocationRel.getType(Config->IsMips64EL)));
        continue;
      }
      if (TargetRel.getType(Config->IsMips64EL) != R_MIPS_64) {
        error("Exptected a R_MIPS_64 relocation in __cap_relocs but got " +
              toString(TargetRel.getType(Config->IsMips64EL)));
        continue;
      }
    } else {
      if (LocationRel.getType(Config->IsMips64EL) != *Target->AbsPointerRel) {
        error("Exptected an absolute pointer relocation in __cap_relocs "
              "but got " + toString(LocationRel.getType(Config->IsMips64EL)));
        continue;
      }
      if (TargetRel.getType(Config->IsMips64EL) != *Target->AbsPointerRel) {
        error("Exptected an absolute pointer relocation in __cap_relocs "
              "but got " + toString(TargetRel.getType(Config->IsMips64EL)));
        continue;
      }
    }
    Symbol *LocationSym =
        &S->getFile<ELFT>()->getRelocTargetSym(LocationRel);
    Symbol &TargetSym = S->getFile<ELFT>()->getRelocTargetSym(TargetRel);

    if (LocationSym->File != S->File) {
      error("Expected capability relocation to point to " + toString(S->File) +
            " but got " + toString(LocationSym->File));
      continue;
    }
    //    errs() << "Adding cap reloc at " << toString(LocationSym) << " type "
    //           << Twine((int)LocationSym.Type) << " against "
    //           << toString(TargetSym) << "\n";
    auto *RawInput = reinterpret_cast<const InMemoryCapRelocEntry<ELFT> *>(
        S->data().begin() + CapRelocsOffset);
    int64_t TargetCapabilityOffset = (int64_t)RawInput->offset;
    assert(RawInput->size == 0 && "Clang should not have set size in __cap_relocs");
    if (!isa<Defined>(LocationSym)) {
      error("Unhandled symbol kind for cap_reloc: " +
            Twine(LocationSym->kind()));
      continue;
    }

    const SymbolAndOffset RelocLocation{LocationSym, LocationRel.r_addend};
    const SymbolAndOffset RelocTarget{&TargetSym, TargetRel.r_addend};
    SymbolAndOffset RealLocation = RelocLocation.findRealSymbol();
    SymbolAndOffset RealTarget = RelocTarget.findRealSymbol();
    if (Config->VerboseCapRelocs) {
      message("Adding capability relocation at " +
              RealLocation.verboseToString() + "\nagainst " +
              RealTarget.verboseToString());
    }

    bool TargetNeedsDynReloc = false;
    if (TargetSym.IsPreemptible) {
      // Do we need this?
      // TargetNeedsDynReloc = true;
    }
    switch (TargetSym.kind()) {
    case Symbol::DefinedKind:
      break;
    case Symbol::SharedKind:
      if (!hasDynamicLinker()) {
        error("cannot create a capability relocation against a shared symbol"
              " when linking statically");
        continue;
      }
      TargetNeedsDynReloc = true;
      break;
    case Symbol::UndefinedKind:
      // addCapReloc() will add an error if we are building an executable
      // instead of a shlib
      // TODO: we really should add a dynamic SIZE relocation as well
      TargetNeedsDynReloc = true;
      break;
    default:
      error("Unhandled symbol kind for cap_reloc target: " +
            Twine(TargetSym.kind()));
      continue;
    }
    assert(LocationSym->isSection());
    auto *LocationDef = cast<Defined>(LocationSym);
    auto *LocationSec = cast<InputSectionBase>(LocationDef->Section);
    addCapReloc({LocationSec, (uint64_t)LocationRel.r_addend, false},
                RealTarget, TargetNeedsDynReloc, TargetCapabilityOffset,
                RealLocation.sym());
  }
}

template <class ELFT>
void CheriCapRelocsSection<ELFT>::addCapReloc(CheriCapRelocLocation Loc,
                                              const SymbolAndOffset &Target,
                                              bool TargetNeedsDynReloc,
                                              int64_t CapabilityOffset,
                                              Symbol *SourceSymbol) {
  if (Config->RelativeCapRelocsOnly) {
    assert(!Loc.NeedsDynReloc);
    if (TargetNeedsDynReloc) {
      error("Cannot add __cap_reloc against target that needs a dynamic "
            "relocation when --relative-cap-relocs is enabled: " +
            Target.verboseToString());
      return;
    }
  } else {
    // Allow relocations in __cap_relocs section for legacy mode
    Loc.NeedsDynReloc = Loc.NeedsDynReloc || Config->Pic || Config->Pie;
    TargetNeedsDynReloc = TargetNeedsDynReloc || Config->Pic || Config->Pie;
  }
  uint64_t CurrentEntryOffset = RelocsMap.size() * RelocSize;

  std::string SourceMsg =
      SourceSymbol ? verboseToString(SourceSymbol) : Loc.toString();
  if (Target.sym()->isUndefined() && !Target.sym()->isUndefWeak()) {
    std::string Msg =
        "cap_reloc against undefined symbol: " + toString(*Target.sym()) +
        "\n>>> referenced by " + SourceMsg;
    if (Config->UnresolvedSymbols == UnresolvedPolicy::ReportError)
      error(Msg);
    else
      nonFatalWarning(Msg);
  }

  // assert(CapabilityOffset >= 0 && "Negative offsets not supported");
  if (errorHandler().Verbose && CapabilityOffset < 0)
    message("global capability offset " + Twine(CapabilityOffset) +
            " is less than 0:\n>>> Location: " + Loc.toString() +
            "\n>>> Target: " + Target.verboseToString());

  bool CanWriteLoc = (Loc.Section->Flags & SHF_WRITE) || !Config->ZText;
  if (!CanWriteLoc) {
    readOnlyCapRelocsError(*Target.sym(), "\n>>> referenced by " + SourceMsg);
    return;
  }

  if (!addEntry(Loc, {Target, CapabilityOffset, TargetNeedsDynReloc})) {
    return; // Maybe happens with vtables?
  }
  if (Loc.NeedsDynReloc) {
    // XXXAR: We don't need to create a symbol here since if we pass nullptr
    // to the dynamic reloc it will add a relocation against the load address
#ifdef DEBUG_CAP_RELOCS
    llvm::sys::path::filename(Loc.Section->File->getName());
    StringRef Filename = llvm::sys::path::filename(Loc.Section->File->getName());
    std::string SymbolHackName = ("__caprelocs_hack_" + Loc.Section->Name + "_" +
                                  Filename).str();
    auto LocationSym = Symtab->find(SymbolHackName);
    if (!LocationSym) {
      Symtab->addDefined(Saver.save(SymbolHackName), STV_DEFAULT, STT_OBJECT,
                         Loc.Offset, Config->CapabilitySize, STB_GLOBAL,
                         Loc.Section, Loc.Section->File);
      LocationSym = Symtab->find(SymbolHackName);
      assert(LocationSym);
    }

    // Needed because local symbols cannot be used in dynamic relocations
    // TODO: do this better
    // message("Adding dyn reloc at " + toString(this) + "+0x" +
    // utohexstr(CurrentEntryOffset))
#endif
    assert(CurrentEntryOffset < getSize());
    // Add a dynamic relocation so that RTLD fills in the right base address
    // We only have the offset relative to the load address...
    // Ideally RTLD/crt_init_globals would just add the load address to all
    // cap_relocs entries that have a RELATIVE flag set instead of requiring a
    // full Elf_Rel/Elf_Rela
    // The addend is zero here since it will be written in writeTo()
    assert(!Config->IsRela);
    Main->RelaDyn->addReloc({elf::Target->RelativeRel, this, CurrentEntryOffset,
                            true, nullptr, 0});
    ContainsDynamicRelocations = true;
  }
  if (TargetNeedsDynReloc) {
#ifdef DEBUG_CAP_RELOCS
    message("Adding dyn reloc at " + toString(this) + "+0x" +
            utohexstr(CurrentEntryOffset) + " against " +
            Target.verboseToString());
    message("Symbol preemptible:" + Twine(Target.Sym->IsPreemptible));
#endif

    bool RelativeToLoadAddress = false;
    RelType RelocKind;
    if (Target.sym()->IsPreemptible) {
      RelocKind = *elf::Target->AbsPointerRel;
    } else {
      // If the target is not preemptible we can optimize this to a relative
      // relocation agaist the image base
      RelativeToLoadAddress = true;
      RelocKind = elf::Target->RelativeRel;
    }
    // The addend is not used as the offset into the capability here, as we
    // have the offset field in the __cap_relocs for that. The Addend
    // will be zero unless we are targetting a string constant as these
    // don't have a symbol and will be like .rodata.str+0x1234
    int64_t Addend = Target.Offset;
    // Capability target is the second field
    assert((CurrentEntryOffset + FieldSize) < getSize());
    Main->RelaDyn->addReloc({RelocKind, this, CurrentEntryOffset + FieldSize,
                            RelativeToLoadAddress, Target.sym(), Addend});
    ContainsDynamicRelocations = true;
    if (!RelativeToLoadAddress) {
      // We also add a size relocation for the size field here
      RelType SizeRel = *elf::Target->SizeRel;
      // Capability size is the fourth field
      assert((CurrentEntryOffset + 3*FieldSize) < getSize());
      Main->RelaDyn->addReloc(SizeRel, this, CurrentEntryOffset + 3*FieldSize,
                             Target.sym());
    }
  }
}

template<typename ELFT>
static uint64_t getTargetSize(const CheriCapRelocLocation &Location,
                              const CheriCapReloc &Reloc, bool Strict) {
  uint64_t TargetSize = Reloc.Target.sym()->getSize();
  if (TargetSize > INT_MAX) {
    error("Insanely large symbol size for " + Reloc.Target.verboseToString() +
          "for cap_reloc at" + Location.toString());
    return 0;
  }
  auto TargetSym = Reloc.Target.sym();
  if (TargetSize == 0 && !TargetSym->IsPreemptible) {
    StringRef Name = TargetSym->getName();
    // Section end symbols like __preinit_array_end, etc. should actually be
    // zero size symbol since they are just markers for the end of a section
    // and not usable as a valid pointer
    if (isSectionEndSymbol(Name) || isSectionStartSymbol(Name))
      return TargetSize;

    bool IsAbsoluteSym = TargetSym->getOutputSection() == nullptr;
    // Symbols previously declared as weak can have size 0 (if they then resolve
    // to NULL). For example __preinit_array_start, etc. are generated by the
    // linker as ABS symbols with value 0.
    // A symbol is linker-synthesized/linker script generated if File == nullptr
    if (IsAbsoluteSym && TargetSym->File == nullptr)
      return TargetSize;

    if (TargetSym->isUndefWeak() && TargetSym->getVA(0) == 0)
      // Weak symbol resolved to NULL -> zero size is fine
      return 0;

    // Absolute value provided by -defsym or assignment in .o file is fine
    if (IsAbsoluteSym)
      return TargetSize;

    // Otherwise warn about missing sizes for symbols
    bool WarnAboutUnknownSize = true;
    // currently clang doesn't emit the necessary symbol information for local
    // string constants such as: struct config_opt opts[] = { { ..., "foo" },
    // { ..., "bar" } }; As this pattern is quite common don't warn if the
    // target section is .rodata.str
    if (Defined *DefinedSym =
      dyn_cast<Defined>(TargetSym)) {
      if (DefinedSym->isSection() &&
          DefinedSym->Section->Name.startswith(".rodata.str")) {
        WarnAboutUnknownSize = false;
      }
    }
    // TODO: are there any other cases that can be ignored?

    if (WarnAboutUnknownSize || errorHandler().Verbose) {
      std::string Msg = "could not determine size of cap reloc against " +
                        Reloc.Target.verboseToString() +
                        "\n>>> referenced by " + Location.toString();
      if (Strict)
        warn(Msg);
      else
        nonFatalWarning(Msg);
    }
    if (OutputSection *OS = TargetSym->getOutputSection()) {
      // For negative offsets use 0 instead (we wan the range of the full symbol in that case)
      int64_t Offset = std::max((int64_t)0, Reloc.Target.Offset);
      uint64_t TargetVA = TargetSym->getVA(Offset);
      assert(TargetVA >= OS->Addr);
      uint64_t OffsetInOS = TargetVA - OS->Addr;
      // Use less-or-equal here to account for __end_foo symbols which point 1 past the section
      assert(OffsetInOS <= OS->Size);
      TargetSize = OS->Size - OffsetInOS;
#ifdef DEBUG_CAP_RELOCS
      if (Config->VerboseCapRelocs)
          errs() << " OS OFFSET 0x" << utohexstr(OS->Addr) << "SYM OFFSET 0x"
                 << utohexstr(OffsetInOS) << " SECLEN 0x" << utohexstr(OS->Size)
                 << " -> target size 0x" << utohexstr(TargetSize) << "\n";
#endif
    } else {
      warn("Could not find size for symbol " + Reloc.Target.verboseToString() +
           " and could not determine section size. Using 0.");
      // TargetSize = std::numeric_limits<uint64_t>::max();
      return 0;
    }
  }
  return TargetSize;
}

template <class ELFT>
struct CaptablePermissions {
  static const uint64_t Function =
      UINT64_C(1) << ((sizeof(typename ELFT::uint) * 8) - 1);
  static const uint64_t ReadOnly =
      UINT64_C(1) << ((sizeof(typename ELFT::uint) * 8) - 2);
};

template <class ELFT> void CheriCapRelocsSection<ELFT>::writeTo(uint8_t *Buf) {
  static_assert(RelocSize == sizeof(InMemoryCapRelocEntry<ELFT>),
                "cap relocs size mismatch");
  uint64_t Offset = 0;
  for (const auto &I : RelocsMap) {
    const CheriCapRelocLocation &Location = I.first;
    const CheriCapReloc &Reloc = I.second;
    assert(Location.Offset <= Location.Section->getSize());
    // We write the virtual address of the location in in both static and the
    // shared library case:
    // In the static case we can compute the final virtual address and write it
    // In the dynamic case we write the virtual address relative to the load
    // address and the runtime linker will add the load address to that
    uint64_t OutSecOffset = Location.Section->getOffset(Location.Offset);
    uint64_t LocationVA =
        Location.Section->getOutputSection()->Addr + OutSecOffset;

    // For the target the virtual address the addend is always zero so
    // if we need a dynamic reloc we write zero
    // TODO: would it be more efficient for local symbols to write the DSO VA
    // and add a relocation against the load address?
    // Also this would make llvm-objdump --cap-relocs more useful because it
    // would actually display the symbol that the relocation is against
    uint64_t TargetVA = Reloc.Target.sym()->getVA(Reloc.Target.Offset);
    bool PreemptibleDynReloc =
        Reloc.NeedsDynReloc && Reloc.Target.sym()->IsPreemptible;
    uint64_t TargetSize = 0;
    if (PreemptibleDynReloc) {
      // If we have a relocation against a preemptible symbol (even in the
      // current DSO) we can't compute the virtual address here so we only write
      // the addend
      if (Reloc.Target.Offset != 0)
        error("Dyn Reloc Target offset was nonzero: " +
              Twine(Reloc.Target.Offset) + " - " +
              Reloc.Target.verboseToString());
      TargetVA = Reloc.Target.Offset;
    } else {
      // For non-preemptible symbols we can write the target size:
      TargetSize = getTargetSize<ELFT>(Location, Reloc,
                                       /*Strict=*/!containsLegacyCapRelocs());
    }
    uint64_t TargetOffset = Reloc.CapabilityOffset;
    uint64_t Permissions = 0;
    // Fow now Function implies ReadOnly so don't add the flag
    if (Reloc.Target.sym()->isFunc()) {
      Permissions |= CaptablePermissions<ELFT>::Function;
    } else if (auto OS = Reloc.Target.sym()->getOutputSection()) {
      assert(!Reloc.Target.sym()->isTls());
      assert((OS->Flags & SHF_TLS) == 0);
      // if ((OS->getPhdrFlags() & PF_W) == 0) {
      if (((OS->Flags & SHF_WRITE) == 0) || isRelroSection(OS)) {
        Permissions |= CaptablePermissions<ELFT>::ReadOnly;
      } else if (OS->Flags & SHF_EXECINSTR) {
        warn("Non-function __cap_reloc against symbol in section with "
             "SHF_EXECINSTR (" + toString(OS->Name) + ") for symbol " +
             Reloc.Target.verboseToString());
      }
    }

    // TODO: should we warn about symbols that are out-of-bounds?
    // mandoc seems to do it so I guess we need it
    // if (TargetOffset < 0 || TargetOffset > TargetSize) warn(...);

    InMemoryCapRelocEntry<ELFT> Entry(LocationVA, TargetVA, TargetOffset,
                                      TargetSize, Permissions);
    memcpy(Buf + Offset, &Entry, sizeof(Entry));
    //     if (errorHandler().Verbose) {
    //       errs() << "Added capability reloc: loc=" << utohexstr(LocationVA)
    //              << ", object=" << utohexstr(TargetVA)
    //              << ", offset=" << utohexstr(TargetOffset)
    //              << ", size=" << utohexstr(TargetSize)
    //              << ", permissions=" << utohexstr(Permissions) << "\n";
    //     }
    Offset += RelocSize;
  }

  // FIXME: this totally breaks dynamic relocs!!! need to do in finalize()

  // Sort the cap_relocs by target address for better cache and TLB locality
  // It also makes it much easier to read the llvm-objdump -C output since it
  // is sorted in a sensible order
  // However, we can't do this if we added any dynamic relocations since it
  // will mean the dynamic relocation offset refers to a different location
  // FIXME: do the sorting in finalizeSection instead
  if (Config->SortCapRelocs && !ContainsDynamicRelocations)
    std::stable_sort(reinterpret_cast<InMemoryCapRelocEntry<ELFT> *>(Buf),
                     reinterpret_cast<InMemoryCapRelocEntry<ELFT> *>(Buf + Offset),
                     [](const InMemoryCapRelocEntry<ELFT> &a,
                        const InMemoryCapRelocEntry<ELFT> &b) {
                       return a.capability_location < b.capability_location;
                     });
  assert(Offset == getSize() && "Not all data written?");
}


CheriCapTableSection::CheriCapTableSection()
  : SyntheticSection(SHF_ALLOC | SHF_WRITE, /* XXX: actually RELRO for BIND_NOW*/
                     SHT_PROGBITS, Config->CapabilitySize, ".captable") {
  assert(Config->CapabilitySize > 0);
  this->Entsize = Config->CapabilitySize;
}

void CheriCapTableSection::writeTo(uint8_t* Buf) {
  // Should be filled with all zeros and crt_init_globals fills it in
  // TODO: fill in the raw bits and use csettag

  auto Write = [&](size_t I, const Symbol *S, int64_t A) {
    uint64_t VA = A;
    if (S)
      VA = S->getVA(A);

    if (Config->Is64)
      write64(Buf + I * 8, VA);
    else
      write32(Buf + I * 4, VA);
  };
  // If TLS entry has a corresponding dynamic relocations, leave it
  // initialized by zero. Write down adjusted TLS symbol's values otherwise.
  // To calculate the adjustments use offsets for thread-local storage.
  for (auto &it : DynTlsEntries.Map) {
    if (it.first == nullptr && !Config->Pic)
      Write(it.second.Index.getValue(), nullptr, 1);
    else if (it.first && !it.first->IsPreemptible) {
      // If we are emitting PIC code with relocations we mustn't write
      // anything to the GOT here. When using Elf_Rel relocations the value
      // one will be treated as an addend and will cause crashes at runtime
      if (!Config->Pic)
        Write(it.second.Index.getValue(), nullptr, 1);
      Write(it.second.Index.getValue() + 1, it.first, 0);
    }
  }

  for (auto &it : TlsEntries.Map)
    Write(it.second.Index.getValue(), it.first, 0);
}

static Defined *findMatchingFunction(InputSectionBase *IS, uint64_t SymOffset) {
  switch (Config->EKind) {
  default:
    llvm_unreachable("Invalid kind");
  case ELF32LEKind:
    return IS->getEnclosingFunction<ELF32LE>(SymOffset);
  case ELF32BEKind:
    return IS->getEnclosingFunction<ELF32BE>(SymOffset);
  case ELF64LEKind:
    return IS->getEnclosingFunction<ELF64LE>(SymOffset);
  case ELF64BEKind:
    return IS->getEnclosingFunction<ELF64BE>(SymOffset);
  }
}

CheriCapTableSection::CaptableMap &
CheriCapTableSection::getCaptableMapForFileAndOffset(InputSectionBase *IS,
                                                     uint64_t Offset) {
  if (LLVM_LIKELY(Config->CapTableScope == CapTableScopePolicy::All))
    return GlobalEntries;
  if (Config->CapTableScope == CapTableScopePolicy::File) {
    // operator[] will insert if missing
    return PerFileEntries[IS->File];
  }
  if (Config->CapTableScope == CapTableScopePolicy::Function) {
    Symbol *Func = findMatchingFunction(IS, Offset);
    if (!Func) {
      warn(
          "Could not find corresponding function with per-function captable: " +
          IS->getObjMsg(Offset));
    }
    // operator[] will insert if missing
    return PerFunctionEntries[Func];
  }
  llvm_unreachable("INVALID CONFIG OPTION");
  return GlobalEntries;
}

void CheriCapTableSection::addEntry(Symbol &Sym, RelExpr Expr,
                                    InputSectionBase *IS, uint64_t Offset) {
  // FIXME: can this be called from multiple threads?
  CapTableIndex Idx;
  Idx.NeedsSmallImm = false;
  Idx.UsedInCallExpr = false;
  Idx.FirstUse = SymbolAndOffset::fromSectionWithOffset(IS, Offset);
  switch (Expr) {
  case R_CHERI_CAPABILITY_TABLE_INDEX_SMALL_IMMEDIATE:
  case R_CHERI_CAPABILITY_TABLE_INDEX_CALL_SMALL_IMMEDIATE:
    Idx.NeedsSmallImm = true;
    break;
  default:
    break;
  }
  // If the symbol is only ever referenced by the captable call relocations we
  // can emit a capability call relocation instead of a normal capability
  // relocation. This indicates to the runtime linker that the capability is
  // not used as a function pointer and therefore does not need a unique
  // address (plt stub) across all DSOs.
  switch (Expr) {
  case R_CHERI_CAPABILITY_TABLE_INDEX_CALL:
  case R_CHERI_CAPABILITY_TABLE_INDEX_CALL_SMALL_IMMEDIATE:
    if (!Sym.isFunc() && !Sym.isUndefWeak()) {
      CheriCapRelocLocation Loc{IS, Offset, false};
      std::string Msg = "call relocation against non-function symbol " + verboseToString(&Sym, 0) +
      "\n>>> referenced by " + Loc.toString();
      if (Sym.isUndefined() && Config->AllowShlibUndefined) {
        // Don't fail the build for shared libraries unless
        nonFatalWarning(Msg);
      } else {
        warn(Msg);
      }
    }
    Idx.UsedInCallExpr = true;
    break;
  default:
    break;
  }
  CaptableMap &Entries = getCaptableMapForFileAndOffset(IS, Offset);
  if (Config->ZCapTableDebug) {
    // Add a local helper symbol to improve disassembly:
    StringRef HelperSymName = Saver.save("$captable_load_" + (Sym.getName().empty() ? "$anonymous_symbol" : Sym.getName()));
    addSyntheticLocal(HelperSymName, STT_NOTYPE, Offset, 0, *IS);
  }

  auto it = Entries.Map.insert(std::make_pair(&Sym, Idx));
  if (!it.second) {
    // If it is references by a small immediate relocation we need to update
    // the small immediate flag
    if (Idx.NeedsSmallImm)
      it.first->second.NeedsSmallImm = true;
    // If one of the uses is not a call expression, we need to emit a unique
    // function pointer and reuse that for the call expression, too.
    // TODO: should we emit two relocations instead?
    if (!Idx.UsedInCallExpr)
      it.first->second.UsedInCallExpr = false;
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

void CheriCapTableSection::addDynTlsEntry(Symbol &Sym) {
  DynTlsEntries.Map.insert(std::make_pair(&Sym, CapTableIndex()));
}

void CheriCapTableSection::addTlsIndex() {
  DynTlsEntries.Map.insert(std::make_pair(nullptr, CapTableIndex()));
}

void CheriCapTableSection::addTlsEntry(Symbol &Sym) {
  TlsEntries.Map.insert(std::make_pair(&Sym, CapTableIndex()));
}

uint32_t CheriCapTableSection::getIndex(const Symbol &Sym, InputSectionBase *IS,
                                        uint64_t Offset) const {
  assert(ValuesAssigned && "getIndex called before index assignment");
  const CaptableMap &Entries = getCaptableMapForFileAndOffset(IS, Offset);
  auto it = Entries.Map.find(const_cast<Symbol *>(&Sym));
  assert(Entries.FirstIndex != std::numeric_limits<uint64_t>::max() &&
         "First index not set yet?");
  assert(it != Entries.Map.end());
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
  return it->second.Index.getValue() - Entries.FirstIndex;
}

uint32_t CheriCapTableSection::getDynTlsOffset(const Symbol &Sym) const {
  assert(ValuesAssigned && "getDynTlsOffset called before index assignment");
  auto it = DynTlsEntries.Map.find(const_cast<Symbol *>(&Sym));
  assert(it != DynTlsEntries.Map.end());
  return it->second.Index.getValue() * Config->Wordsize;
}

uint32_t CheriCapTableSection::getTlsIndexOffset() const {
  assert(ValuesAssigned && "getTlsIndexOffset called before index assignment");
  auto it = DynTlsEntries.Map.find(nullptr);
  assert(it != DynTlsEntries.Map.end());
  return it->second.Index.getValue() * Config->Wordsize;
}

uint32_t CheriCapTableSection::getTlsOffset(const Symbol &Sym) const {
  assert(ValuesAssigned && "getTlsOffset called before index assignment");
  auto it = TlsEntries.Map.find(const_cast<Symbol *>(&Sym));
  assert(it != TlsEntries.Map.end());
  return it->second.Index.getValue() * Config->Wordsize;
}

template <class ELFT>
uint64_t CheriCapTableSection::assignIndices(uint64_t StartIndex,
                                             CaptableMap &Entries,
                                             const Twine &SymContext) {
  // Usually StartIndex will be zero (one global captable) but if we are
  // compiling with per-file/per-function
  uint64_t SmallEntryCount = 0;
  assert(Entries.FirstIndex == std::numeric_limits<uint64_t>::max() &&
         "Should not be initialized yet!");
  Entries.FirstIndex = StartIndex;
  for (auto &it : Entries.Map) {
    // TODO: looping twice is inefficient, we could keep track of the number of
    // small entries during insertion
    if (it.second.NeedsSmallImm) {
      SmallEntryCount++;
    }
  }

  if (Config->EMachine == EM_MIPS) {
    unsigned MaxSmallEntries = (1 << 19) / Config->CapabilitySize;
    if (SmallEntryCount > MaxSmallEntries) {
      // Use warn here since the calculation may be wrong if the 11 bit clc is
      // used. We will error when writing the relocation values later anyway
      // so this will help find the error
      warn("added " + Twine(SmallEntryCount) + " entries to .captable but "
          "current maximum is " + Twine(MaxSmallEntries) + "; try recompiling "
          "non-performance critical source files with -mxcaptable");
    }
    if (errorHandler().Verbose) {
      message("Total " + Twine(Entries.size()) + " .captable entries: " +
          Twine(SmallEntryCount) + " use a small immediate and " +
          Twine(Entries.size() - SmallEntryCount) + " use -mxcaptable. ");
    }
  }

  // Only add the @CAPTABLE symbols when running the LLD unit tests
  // errorHandler().ExitEarly is set to false if LLD_IN_TEST=1 so just reuse
  // that instead of calling getenv on every iteration
  const bool ShouldAddAtCaptableSymbols = !errorHandler().ExitEarly;
  uint32_t AssignedSmallIndexes = 0;
  uint32_t AssignedLargeIndexes = 0;
  for (auto &it : Entries.Map) {
    CapTableIndex &CTI = it.second;
    if (CTI.NeedsSmallImm) {
      assert(AssignedSmallIndexes < SmallEntryCount);
      CTI.Index = StartIndex + AssignedSmallIndexes;
      AssignedSmallIndexes++;
    } else {
      CTI.Index = StartIndex + SmallEntryCount + AssignedLargeIndexes;
      AssignedLargeIndexes++;
    }

    uint32_t Index = *CTI.Index;
    assert(Index >= StartIndex && Index < StartIndex + Entries.size());
    Symbol *TargetSym = it.first;

    StringRef Name = TargetSym->getName();
    // Avoid duplicate symbol name errors for unnamed string constants:
    StringRef RefName;
    // For now always append .INDEX to local symbols @CAPTABLE names since they
    // might not be unique. If there is a global with the same name we always
    // want the global to have the plain @CAPTABLE name
    if (Name.empty() /* || Name.startswith(".L") */ || TargetSym->isLocal())
      RefName = Saver.save(Name + "@CAPTABLE" + SymContext + "." + Twine(Index));
    else
      RefName = Saver.save(Name + "@CAPTABLE" + SymContext);
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
    uint64_t Off = Index * Config->CapabilitySize;
    if (ShouldAddAtCaptableSymbols) {
      addSyntheticLocal(RefName, STT_OBJECT, Off, Config->CapabilitySize, *this);
    }
    // If the symbol is used as a function pointer the runtime linker has to
    // ensure that all pointers to that function compare equal. This is done
    // by ensuring that they all point to the same PLT stub.
    // If it is not used as a function pointer we can use a capability call
    // relocation instead which allows the runtime linker to create non-unique
    // plt stubs.
    RelType ElfCapabilityReloc = it.second.UsedInCallExpr
                                     ? *Target->CheriCapCallRel
                                     : *Target->CheriCapRel;
    // All capability call relocations should end up in the pltrel section
    // rather than the normal relocation section to make processing of PLT
    // relocations in RTLD more efficient.
    RelocationBaseSection *DynRelSec =
        it.second.UsedInCallExpr ? In.RelaPlt : Main->RelaDyn;
    addCapabilityRelocation<ELFT>(
        TargetSym, ElfCapabilityReloc, In.CheriCapTable, Off,
        R_CHERI_CAPABILITY, 0, it.second.UsedInCallExpr,
        [&]() {
          return ("\n>>> referenced by " + RefName + "\n>>> first used in " +
                  it.second.FirstUse->verboseToString())
              .str();
        },
        DynRelSec);
  }
  assert(AssignedSmallIndexes + AssignedLargeIndexes == Entries.size());
  return AssignedSmallIndexes + AssignedLargeIndexes;
}

template <class ELFT>
void CheriCapTableSection::assignValuesAndAddCapTableSymbols() {
  // First assign the global indices (which will usually be the only ones)
  uint64_t AssignedEntries = assignIndices<ELFT>(0, GlobalEntries, "");
  if (LLVM_UNLIKELY(Config->CapTableScope != CapTableScopePolicy::All)) {
    assert(AssignedEntries == 0 && "Should not have any global entries in"
                                   " per-file/per-function captable mode");
    for (auto &it : PerFileEntries) {
      std::string FullContext = toString(it.first);
      auto LastSlash = StringRef(FullContext).find_last_of("/\\") + 1;
      StringRef Context = StringRef(FullContext).substr(LastSlash);
      AssignedEntries += assignIndices<ELFT>(AssignedEntries, it.second,
                                             "@" + Context);
    }
    for (auto &it : PerFunctionEntries)
      AssignedEntries += assignIndices<ELFT>(AssignedEntries, it.second,
                                             "@" + toString(*it.first));
  }
  assert(AssignedEntries == nonTlsEntryCount());

  uint32_t AssignedTlsIndexes = 0;
  uint32_t TlsBaseIndex =
      AssignedEntries * (Config->CapabilitySize / Config->Wordsize);

  // TODO: support TLS for per-function captable
  if (Config->CapTableScope != CapTableScopePolicy::All &&
      (!DynTlsEntries.empty() || !TlsEntries.empty())) {
    error("TLS is not supported yet with per-file or per-function captable");
    return;
  }

  for (auto &it : DynTlsEntries.Map) {
    CapTableIndex &CTI = it.second;
    assert(!CTI.NeedsSmallImm);
    CTI.Index = TlsBaseIndex + AssignedTlsIndexes;
    AssignedTlsIndexes += 2;
    Symbol *S = it.first;
    uint64_t Offset = CTI.Index.getValue() * Config->Wordsize;
    if (S == nullptr) {
      if (!Config->Pic)
        continue;
      Main->RelaDyn->addReloc(Target->TlsModuleIndexRel, this, Offset, S);
    } else {
      // When building a shared library we still need a dynamic relocation
      // for the module index. Therefore only checking for
      // S->IsPreemptible is not sufficient (this happens e.g. for
      // thread-locals that have been marked as local through a linker script)
      if (!S->IsPreemptible && !Config->Pic)
        continue;
      Main->RelaDyn->addReloc(Target->TlsModuleIndexRel, this, Offset, S);
      // However, we can skip writing the TLS offset reloc for non-preemptible
      // symbols since it is known even in shared libraries
      if (!S->IsPreemptible)
        continue;
      Offset += Config->Wordsize;
      Main->RelaDyn->addReloc(Target->TlsOffsetRel, this, Offset, S);
    }
  }

  for (auto &it : TlsEntries.Map) {
    CapTableIndex &CTI = it.second;
    assert(!CTI.NeedsSmallImm);
    CTI.Index = TlsBaseIndex + AssignedTlsIndexes++;
    Symbol *S = it.first;
    uint64_t Offset = CTI.Index.getValue() * Config->Wordsize;
    if (S->IsPreemptible)
      Main->RelaDyn->addReloc(Target->TlsGotRel, this, Offset, S);
  }

  ValuesAssigned = true;
}

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
  assert(Config->CapabilitySize > 0);
  this->Entsize = sizeof(CaptableMappingEntry);
  static_assert(sizeof(CaptableMappingEntry) == 24, "");
}

size_t CheriCapTableMappingSection::getSize() const {
  assert(Config->CapTableScope != CapTableScopePolicy::All);
  if (!isNeeded())
    return 0;
  size_t Count = 0;
  if (!In.SymTab) {
    error("Cannot use " + this->Name + " without .symtab section!");
    return 0;
  }
  for (const SymbolTableEntry &STE : In.SymTab->getSymbols()) {
    if (!STE.Sym->isDefined() || !STE.Sym->isFunc())
      continue;
    Count++;
  }
  return Count * sizeof(CaptableMappingEntry);
}

void CheriCapTableMappingSection::writeTo(uint8_t *Buf) {
  assert(Config->CapTableScope != CapTableScopePolicy::All);
  if (!In.CheriCapTable)
    return;
  if (!In.SymTab) {
    error("Cannot write " + this->Name + " without .symtab section!");
    return;
  }

  // Write the mapping from function vaddr -> captable subset for RTLD
  std::vector<CaptableMappingEntry> Entries;
  // Note: Symtab->getSymbols() only returns the symbols in .dynsym. We need
  // to use In.sym()tab instead since we also want to add all local functions!
  for (const SymbolTableEntry &STE : In.SymTab->getSymbols()) {
    Symbol* Sym = STE.Sym;
    if (!Sym->isDefined() || !Sym->isFunc())
      continue;
    const CheriCapTableSection::CaptableMap *CapTableMap = nullptr;
    if (Config->CapTableScope == CapTableScopePolicy::Function) {
      auto it = In.CheriCapTable->PerFunctionEntries.find(Sym);
      if (it != In.CheriCapTable->PerFunctionEntries.end())
        CapTableMap = &it->second;
    } else if (Config->CapTableScope == CapTableScopePolicy::File) {
      auto it = In.CheriCapTable->PerFileEntries.find(Sym->File);
      if (it != In.CheriCapTable->PerFileEntries.end())
        CapTableMap = &it->second;
    } else {
      llvm_unreachable("Invalid mode!");
    }
    CaptableMappingEntry Entry;
    Entry.FuncStart = Sym->getVA(0);
    Entry.FuncEnd = Entry.FuncStart + Sym->getSize();
    if (CapTableMap) {
      assert(CapTableMap->FirstIndex != std::numeric_limits<uint64_t>::max());
      Entry.CapTableOffset = CapTableMap->FirstIndex * Config->CapabilitySize;
      Entry.SubTableSize = CapTableMap->size() * Config->CapabilitySize;
    } else {
      // TODO: don't write an entry for functions that don't use the captable
      Entry.CapTableOffset = 0;
      Entry.SubTableSize = 0;
    }
    Entries.push_back(Entry);
  }
  // Sort all the entries so that RTLD can do a binary search to find the
  // correct entry instead of having to scan all of them.
  // Do this before swapping to target endianess to simplify the comparisons.
  llvm::sort(Entries, [](const CaptableMappingEntry &E1,
                         const CaptableMappingEntry &E2) {
    if (E1.FuncStart == E2.FuncStart)
      return E1.FuncEnd < E2.FuncEnd;
    return E1.FuncStart < E2.FuncStart;
  });
  // Byte-swap all the values so that we can memcpy the sorted buffer
  for (CaptableMappingEntry &E : Entries) {
    E.FuncStart = support::endian::byte_swap(E.FuncStart, Config->Endianness);
    E.FuncEnd = support::endian::byte_swap(E.FuncEnd, Config->Endianness);
    E.CapTableOffset =
        support::endian::byte_swap(E.CapTableOffset, Config->Endianness);
    E.SubTableSize =
        support::endian::byte_swap(E.SubTableSize, Config->Endianness);
  }
  assert(Entries.size() * sizeof(CaptableMappingEntry) == getSize());
  memcpy(Buf, Entries.data(), Entries.size() * sizeof(CaptableMappingEntry));
}

template <typename ELFT>
void addCapabilityRelocation(Symbol *Sym, RelType Type, InputSectionBase *Sec,
                             uint64_t Offset, RelExpr Expr, int64_t Addend,
                             bool IsCallExpr,
                             llvm::function_ref<std::string()> ReferencedBy,
                             RelocationBaseSection *DynRelSec) {
  // Emit either the legacy __cap_relocs section or a R_CHERI_CAPABILITY reloc
  // For local symbols we can also emit the untagged capability bits and
  // instruct csu/rtld to run CBuildCap
  assert(Config->ProcessCapRelocs);
  CapRelocsMode CapRelocMode = Sym->IsPreemptible
                                   ? Config->PreemptibleCapRelocsMode
                                   : Config->LocalCapRelocsMode;
  bool NeedTrampoline = false;
  // In the PLT ABI (and fndesc?) we have to use an elf relocation for function
  // pointers to ensure that the runtime linker adds the required trampolines
  // that sets $cgp:
  if (!IsCallExpr && Config->EMachine == llvm::ELF::EM_MIPS && Sym->isFunc()) {
    if (!lld::elf::hasDynamicLinker()) {
      // In static binaries we do not need PLT stubs for function pointers since
      // all functions share the same $cgp
      // TODO: this is no longer true if we were to support dlopen() in static
      // binaries
      if (Config->VerboseCapRelocs)
        message("Do not need function pointer trampoline for " +
                toString(*Sym) + " in static binary");
      NeedTrampoline = false;
    } else if (auto ABI = InX<ELFT>::MipsAbiFlags->getCheriAbiVariant()) {
      if (*ABI == llvm::ELF::DF_MIPS_CHERI_ABI_PLT ||
          *ABI == llvm::ELF::DF_MIPS_CHERI_ABI_FNDESC)
        NeedTrampoline = true;
    }
  }
  if (NeedTrampoline) {
    CapRelocMode = CapRelocsMode::ElfReloc;
    assert(CapRelocMode == Config->PreemptibleCapRelocsMode);
    if (Config->VerboseCapRelocs)
      message("Using trampoline for function pointer against " +
              verboseToString(Sym));
  }

  // local cap relocs don't need a Elf relocation with a full symbol lookup:
  if (CapRelocMode == CapRelocsMode::ElfReloc) {
    assert((Sym->IsPreemptible || NeedTrampoline) &&
           "ELF relocs should not be used for non-preemptible symbols");
    assert((!Sym->isLocal() || NeedTrampoline) &&
           "ELF relocs should not be used for local symbols");
    if (Config->EMachine == llvm::ELF::EM_MIPS && Config->BuildingFreeBSDRtld) {
      error("relocation " + toString(Type) + " against " +
            verboseToString(Sym) +
            " cannot be using when building FreeBSD RTLD" + ReferencedBy());
      return;
    }
    if (!lld::elf::hasDynamicLinker()) {
      error("attempting to emit a R_CAPABILITY relocation against " +
            (Sym->getName().empty() ? "local symbol"
                                    : "symbol " + toString(*Sym)) +
            " in binary without a dynamic linker; try removing -Wl,-" +
            (Sym->IsPreemptible ? "preemptible" : "local") + "-caprelocs=elf" +
            ReferencedBy());
      return;
    }
    assert(Config->HasDynSymTab && "Should have been checked in Driver.cpp");
    // We don't use a R_MIPS_CHERI_CAPABILITY relocation for the input but
    // instead need to use an absolute pointer size relocation to write
    // the offset addend
    if (!DynRelSec)
      DynRelSec = Main->RelaDyn;
    // in the case that -local-caprelocs=elf is passed we need to ensure that
    // the target symbol is included in the dynamic symbol table
    if (!Main->DynSymTab) {
      error("R_CHERI_CAPABILITY relocations need a dynamic symbol table");
      return;
    }
    if (!Sym->includeInDynsym()) {
      if (!NeedTrampoline) {
        error("added a R_CHERI_CAPABILITY relocation but symbol not included "
              "in dynamic symbol: " +
              verboseToString(Sym));
        return;
      }
      // Hack: Add a new global symbol with a unique name so that we can use
      // a dynamic relocation against it.
      // TODO: should it be possible to add STB_LOCAL symbols to .dynsymtab?



      std::string UniqueName = ("__cheri_fnptr_" + Sym->getName()).str();
      for (int i = 2; Symtab->find(UniqueName); i++) {
        UniqueName = ("__cheri_fnptr" + Twine(i) + "_" + Sym->getName()).str();
      }
      Defined* NewSym = Symtab->ensureSymbolWillBeInDynsym(Sym);
      assert(NewSym->isFunc() && "This should only be used for functions");
      assert(NewSym->includeInDynsym());
      assert(NewSym->Binding == llvm::ELF::STB_GLOBAL);
      assert(NewSym->Visibility == llvm::ELF::STV_HIDDEN);
      Sym = NewSym; // Make the relocation point to the newly added symbol
    }
    DynRelSec->addReloc(
        Type, Sec, Offset, Sym, Addend, Expr,
        /* Relocation type for the addend = */ *Target->AbsPointerRel);
  } else if (CapRelocMode == CapRelocsMode::Legacy) {
    bool NeedsDynReloc = Config->Pic;
    if (Config->RelativeCapRelocsOnly) {
      assert(!Sym->IsPreemptible);
      NeedsDynReloc = false;
    }
    InX<ELFT>::CapRelocs->addCapReloc({Sec, Offset, NeedsDynReloc}, {Sym, 0u},
                                      Sym->IsPreemptible, Addend);
  } else {
    assert(Config->LocalCapRelocsMode == CapRelocsMode::CBuildCap);
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
