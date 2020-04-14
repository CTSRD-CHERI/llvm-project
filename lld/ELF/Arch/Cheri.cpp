#include "Cheri.h"
#include "../Bits.h"
#include "../OutputSections.h"
#include "../SymbolTable.h"
#include "../SyntheticSections.h"
#include "../Target.h"
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

static uint8_t GetMagicIndexForSegment(PhdrEntry* Segment) {
  if(Segment == nullptr) return (uint8_t)0;

  assert(Segment->ndx < 0xFE);
  return (uint8_t)(Segment->ndx + 1);
}

template <class ELFT>
CheriCapRelocsSection<ELFT>::CheriCapRelocsSection()
    : SyntheticSection(SHF_ALLOC | (Config->Pic ? SHF_WRITE : 0), /* XXX: actually RELRO */
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
  S->Live = false;

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
  assert((Default == nullptr || FallbackResult) && "SHOULD HAVE FOUND A SYMBOL!");
  return {FallbackResult, (int64_t)FallbackOffset};
}

SymbolAndOffset SymbolAndOffset::findRealSymbol() const {
  if (!Sym->isSection())
    return *this;

  if (Defined *DefinedSym = dyn_cast<Defined>(Sym)) {
    if (auto *IS = dyn_cast<InputSectionBase>(DefinedSym->Section)) {
      return SymbolAndOffset::fromSectionWithOffset(IS, Offset, Sym);
    }
  }
  return *this;
}

template<typename ELFT>
std::string CheriCapRelocLocation::toString() const {
  SymbolAndOffset Resolved =
      SymbolAndOffset::fromSectionWithOffset(Section, Offset);
  if (Resolved.Sym)
    return Resolved.verboseToString();
  return Section->getObjMsg(Offset);
}

template <class ELFT>
void CheriCapRelocsSection<ELFT>::processSection(InputSectionBase *S) {
  constexpr endianness E = ELFT::TargetEndianness;
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
    if (TargetRel.r_offset != LocationRel.r_offset + 8) {
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
    if (LocationRel.getType(Config->IsMips64EL) != R_MIPS_64) {
      error("Exptected a R_MIPS_64 relocation in __cap_relocs but got " +
            toString(LocationRel.getType(Config->IsMips64EL)));
      continue;
    }
    if (TargetRel.getType(Config->IsMips64EL) != R_MIPS_64) {
      error("Exptected a R_MIPS_64 relocation in __cap_relocs but got " +
            toString(LocationRel.getType(Config->IsMips64EL)));
      continue;
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
    auto *RawInput = reinterpret_cast<const InMemoryCapRelocEntry<E> *>(
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
      TargetNeedsDynReloc = true;
    }
    switch (TargetSym.kind()) {
    case Symbol::DefinedKind:
      break;
    case Symbol::SharedKind:
      if (!Config->Shared && !In.Interp) {
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
                RealLocation.Sym);
  }
}

template <class ELFT>
void CheriCapRelocsSection<ELFT>::addCapReloc(CheriCapRelocLocation Loc,
                                              const SymbolAndOffset &Target,
                                              bool TargetNeedsDynReloc,
                                              int64_t CapabilityOffset,
                                              Symbol *SourceSymbol) {
  Loc.NeedsDynReloc = Loc.NeedsDynReloc || Config->Pic || Config->Pie;
  TargetNeedsDynReloc = TargetNeedsDynReloc || Config->Pic || Config->Pie;
  uint64_t CurrentEntryOffset = RelocsMap.size() * RelocSize;

  std::string SourceMsg =
      SourceSymbol ? verboseToString(SourceSymbol) : Loc.toString<ELFT>();
  if (Target.Sym->isUndefined() && !Target.Sym->isUndefWeak()) {
    std::string Msg =
        "cap_reloc against undefined symbol: " + toString(*Target.Sym) +
        "\n>>> referenced by " + SourceMsg;
    if (Config->UnresolvedSymbols == UnresolvedPolicy::ReportError)
      error(Msg);
    else
      nonFatalWarning(Msg);
  }

  // assert(CapabilityOffset >= 0 && "Negative offsets not supported");
  if (errorHandler().Verbose && CapabilityOffset < 0)
    message("global capability offset " + Twine(CapabilityOffset) +
            " is less than 0:\n>>> Location: " + Loc.toString<ELFT>() +
            "\n>>> Target: " + Target.verboseToString());

  bool CanWriteLoc = (Loc.Section->Flags & SHF_WRITE) || !Config->ZText;
  if (!CanWriteLoc) {
    readOnlyCapRelocsError(*Target.Sym, "\n>>> referenced by " + SourceMsg);
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
        Symtab->addDefined<ELFT>(Saver.save(SymbolHackName), STV_DEFAULT,
                                 STT_OBJECT, Loc.Offset, Config->CapabilitySize,
                                 STB_GLOBAL, Loc.Section, Loc.Section->File);
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
    In.RelaDyn->addReloc({elf::Target->RelativeRel, this, CurrentEntryOffset,
                            true, nullptr, 0});
    ContainsDynamicRelocations = true;
  }
  if (TargetNeedsDynReloc) {
#ifdef DEBUG_CAP_RELOCS
    message("Adding dyn reloc at " + toString(this) + "+0x" +
            utohexstr(OffsetInOutSec) + " against " + Target.verboseToString());
    message("Symbol preemptible:" + Twine(Target.Sym->IsPreemptible));
#endif

    bool RelativeToLoadAddress = false;
    RelType RelocKind;
    if (Target.Sym->IsPreemptible) {
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
    // Capability target is the second field -> offset + 8
    assert((CurrentEntryOffset + 8) < getSize());
    In.RelaDyn->addReloc({RelocKind, this, CurrentEntryOffset + 8,
                            RelativeToLoadAddress, Target.Sym, Addend});
    ContainsDynamicRelocations = true;
    if (!RelativeToLoadAddress) {
      // We also add a size relocation for the size field here
      assert(Config->EMachine == EM_MIPS);
      RelType Size64Rel = R_MIPS_CHERI_SIZE | (R_MIPS_64 << 8);
      // Capability size is the fourth field -> offset + 24
      assert((CurrentEntryOffset + 24) < getSize());
      In.RelaDyn->addReloc(Size64Rel, this, CurrentEntryOffset + 24,
                             Target.Sym);
    }
  }
}

template<typename ELFT>
static uint64_t getTargetSize(const CheriCapRelocLocation &Location,
                              const CheriCapReloc &Reloc, bool Strict) {
  uint64_t TargetSize = Reloc.Target.Sym->getSize();
  if (TargetSize > INT_MAX) {
    error("Insanely large symbol size for " + Reloc.Target.verboseToString() +
          "for cap_reloc at" + Location.toString<ELFT>());
    return 0;
  }
  auto TargetSym = Reloc.Target.Sym;
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
                        "\n>>> referenced by " + Location.toString<ELFT>();
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

template <class ELFT> void CheriCapRelocsSection<ELFT>::writeTo(uint8_t *Buf) {
  constexpr endianness E = ELFT::TargetEndianness;
  static_assert(RelocSize == sizeof(InMemoryCapRelocEntry<E>),
                "cap relocs size mismatch");
  uint64_t Offset = 0;
  for (const auto &I : RelocsMap) {
    const CheriCapRelocLocation &Location = I.first;
    const CheriCapReloc &Reloc = I.second;
    assert(Location.Offset <= Location.Section->getSize());
    // We write the virtual address of the location in in both static and the
    // shared library case:
    // In the static case we can compute the final virtual address and write it
    // In the dynamic case we write the virtual address relative to the load address
    // and the runtime linker will add the load address to that

    bool targetsTLS = Reloc.Target.Sym->isTls();
    bool targetNoOutput = Reloc.Target.Sym->getOutputSection() == nullptr;

    OutputSection *LocOutputSec = Location.Section->getOutputSection(); // FIXME this assert gets hit if you delete the captable in your linker script but then define symbols that go there.

    assert(LocOutputSec != nullptr);

    PhdrEntry *LocationPHDR = targetsTLS ? LocOutputSec->PtTLS :
                              LocOutputSec->PtLoad;
    PhdrEntry *TargetPHDR = targetNoOutput ? nullptr :
                            (targetsTLS ?  Reloc.Target.Sym->getOutputSection()->PtTLS :
                            Reloc.Target.Sym->getOutputSection()->PtLoad);

    assert(LocationPHDR != nullptr && "Could not find a program header for relocation location");

    uint64_t OutSecOffset = Location.Section->getOffset(Location.Offset);
    uint64_t OutSegOffset = LocationPHDR->p_vaddr;
    uint64_t LocationVA = LocOutputSec->Addr + (OutSecOffset - OutSegOffset);

    uint8_t LocationSegmentID = GetMagicIndexForSegment(LocationPHDR);
    uint8_t RelocSegmentID = GetMagicIndexForSegment(TargetPHDR);

    // For the target the virtual address the addend is always zero so
    // if we need a dynamic reloc we write zero
    // TODO: would it be more efficient for local symbols to write the DSO VA
    // and add a relocation against the load address?
    // Also this would make llvm-objdump -C more useful because it would
    // actually display the symbol that the relocation is against
    uint64_t TargetVA = Reloc.Target.Sym->getVA(Reloc.Target.Offset);
    bool PreemptibleDynReloc =
        Reloc.NeedsDynReloc && Reloc.Target.Sym->IsPreemptible;
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

    if(!targetsTLS && !targetNoOutput) {

      assert(TargetPHDR != nullptr && "COuld not find a program header for relocation target");

      // This is allowed to happen. Sometimes symbols VAs are used as a way to communicate between a
      // linker script and a program. We try to preserve behaviour here.

      TargetVA -= TargetPHDR->p_vaddr;
    }

    uint64_t TargetOffset = Reloc.CapabilityOffset;

    uint16_t flags = 0;
    if(targetsTLS) flags |=1;
    if(targetNoOutput) flags |= 2;

    // TODO: should we warn about symbols that are out-of-bounds?
    // mandoc seems to do it so I guess we need it
    // if (TargetOffset <= TargetSize || TargetOffset > TargetSize) warn(...);
    // TODO more accurate permission masks
    uint32_t Permissions = Reloc.Target.Sym->isFunc() ? 1U << 31 : 0;
    InMemoryCapRelocEntry<E> Entry{LocationVA, TargetVA, TargetOffset,
                                   TargetSize , Permissions, flags, LocationSegmentID, RelocSegmentID};
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
    std::stable_sort(reinterpret_cast<InMemoryCapRelocEntry<E> *>(Buf),
                     reinterpret_cast<InMemoryCapRelocEntry<E> *>(Buf + Offset),
                     [](const InMemoryCapRelocEntry<E> &a,
                        const InMemoryCapRelocEntry<E> &b) {
                       return a.capability_location < b.capability_location;
                     });
  assert(Offset == getSize() && "Not all data written?");
}


CheriCapTableSection::CheriCapTableSection(bool local)
  : SyntheticSection(SHF_ALLOC | SHF_WRITE, /* XXX: actually RELRO */
                     SHT_PROGBITS, Config->CapabilitySize, local ? ".cap_table_local" : ".cap_table") {
  assert(Config->CapabilitySize > 0);
  this->Entsize = Config->CapabilitySize;

  IsLocal = local;

  if(local) {
    Flags |= SHF_TLS;
  }

  if(local) {
    for(uint64_t i = 1; i <= 9; i++) {
      addFixedEntry(*(Symbol*)i, i-1);
    }
  }
}

void CheriCapTableSection::writeTo(uint8_t* Buf) {
  // Should be filled with all zeros and crt_init_globals fills it in
  // TODO: fill in the raw bits and use csettag
  (void)Buf;
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

void CheriCapTableSection::addEntry(Symbol &Sym, bool SmallImm, RelType Type,
                                    InputSectionBase *IS, uint64_t Offset) {
  // FIXME: can this be called from multiple threads?
  CapTableIndex Idx;
  Idx.NeedsSmallImm = SmallImm;
  Idx.IsFixed = false;
  Idx.priority = -1;
  Idx.UsedAsFunctionPointer = true;
  // If the symbol is only ever referenced by the CAP*CALL* relocations we can
  // emit a R_MIPS_CHERI_CAPABILITY_CALL instead of a R_MIPS_CHERI_CAPABILITY
  // relocation. This indicates to the runtime linker that the capability is not
  // used as a function pointer and therefore does not need a unique address
  // (plt stub) across all DSOs.
  switch (Type) {
  case R_MIPS_CHERI_CAPCALL20:
  case R_MIPS_CHERI_CAPCALL_CLC11:
  case R_MIPS_CHERI_CAPCALL_HI16:
  case R_MIPS_CHERI_CAPCALL_LO16:
    Idx.UsedAsFunctionPointer = false;
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
    if (SmallImm)
      it.first->second.NeedsSmallImm = true;
    if (Idx.UsedAsFunctionPointer)
      it.first->second.UsedAsFunctionPointer = true;
  } else {
    Entries.unfixed_entries++;
      if (SmallImm) {
        Entries.small_entries++;
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

void CheriCapTableSection::addFixedEntry(Symbol &Sym, uint64_t index) {
  assert((Config->CapTableScope == CapTableScopePolicy::All));
  CaptableMap &Entries = GlobalEntries;
  CapTableIndex Idx;
  Idx.IsFixed = true;
  Idx.Index = index;
  Idx.priority = -1;
  bool added = Entries.Map.insert(std::make_pair(&Sym, Idx)).second;
  assert(added);
  Entries.fixed_entries++;
  Entries.small_entries++;
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
struct sort_pair {
  uint32_t vector_index;
  int32_t priority; // priority of being placed in the cap table (larger more priority)
};

template <class ELFT>
uint64_t CheriCapTableSection::assignIndices(uint64_t StartIndex,
                                             CaptableMap &Entries,
                                             const Twine &SymContext) {
  // FIXME: we should not be hardcoding architecture specific relocation numbers
  // here

  assert(Entries.FirstIndex == std::numeric_limits<uint64_t>::max() &&
         "Should not be initialized yet!");
  Entries.FirstIndex = StartIndex;

  uint64_t fixed_entries = Entries.fixed_entries;
  uint64_t unfixed_entries = Entries.unfixed_entries;
  uint64_t small_entries = Entries.small_entries;

  // TODO allow per function cap tables as well
  assert(StartIndex == 0);

  Defined *CheriCapabilityTable = this->CheriCapabilityTable;

  /*
  if (CheriCapabilityTable && !Config->Relocatable) {
    size_t sz = getSize();
    CheriCapabilityTable->Size = getSize(); // FIXME: This assignment segfaults. Does it do anything?
  }
   */

  uint64_t total_entries = unfixed_entries + fixed_entries;

  assert(total_entries == Entries.size());

  // A map if which slots are taken.
  bool* taken_map = (bool*)alloca((sizeof(bool)) * total_entries);
  bzero(taken_map, (sizeof(bool)) * total_entries);

  // Everything taken until
  size_t taken_till = 0;

  unsigned MaxSmallEntries = (1 << 19) / Config->CapabilitySize;
  if (small_entries > MaxSmallEntries) {
    // Use warn here since the calculation may be wrong if the 11 bit clc is
    // used. We will error when writing the relocation values later anyway
    // so this will help find the error
    warn("added " + Twine(small_entries) + " entries to .cap_table but "
        "current maximum is " + Twine(MaxSmallEntries) + "; try recompiling "
        "non-performance critical source files with -mxcaptable");
  }
  if (errorHandler().Verbose) {
    message("Total " + Twine(Entries.size()) + " .captable entries: " +
        Twine(small_entries) + " use a small immediate and " +
        Twine(Entries.size() - small_entries) + " use -mxcaptable. ");
  }

  // Only add the @CAPTABLE symbols when running the LLD unit tests
  // errorHandler().ExitEarly is set to false if LLD_IN_TEST=1 so just reuse
  // that instead of calling getenv on every iteration
  const bool ShouldAddAtCaptableSymbols = !errorHandler().ExitEarly;

  // We assigned an index to entries where larger = referenced later. Put them in the cap captable in reverse order.

  // First create a map that we can sort efficiently
  sort_pair* SortMap = (sort_pair*)alloca(sizeof(sort_pair) * unfixed_entries);

  uint32_t assigned = 0;
  uint32_t vector_index = 0;
  uint32_t map_index = 0;

  // Hopefully we have done all lazy resolution now. Added the remaining symbol to an aux table to not cause any output
  if(!CheriCapTableSection::auxSymTab) {
    SymbolTable* old = Symtab;
    CheriCapTableSection::auxSymTab = make<SymbolTable>();
    Symtab = CheriCapTableSection::auxSymTab; // And THIS is why we don't use globals kids
    // Process all of the archives
    for(ArchiveFile* Arc : old->ArcVector) {
      Arc->fetchRemaining<ELFT>(CheriCapTableSection::auxSymTab);
    }
    Symtab = old;
  }

  for(ArchiveFile* Arc: Symtab->ArcVector) {
    // process the archive and assign indexs
    uint32_t pref_glob = 0;
    int32_t prio = (int32_t)Arc->GroupId;

    for(auto child : Arc->Children) {
      InputFile* file = child.second;
      for(Symbol* S: file->getSymbols()) {

        Symbol* AuxS = auxSymTab->find(S->getName());
        Symbol* MainS = Symtab->find(S->getName());

        if(AuxS != nullptr && MainS != nullptr) {
          // Due to the way we add symbols to the aux table, they may be duplicated
          // If we find two we need to choose a canonical one
          // We prefer to take mains, unless it is lazy, then we take the aux
          S = MainS->isLazy() ? AuxS : MainS;
        }

        if(S->ArchiveColoured == Arc || (S->isTls() != IsLocal)) continue; // Don't double count usage of the same symbol in the same archive
        S->ArchiveColoured = Arc;
        bool likely = (S->isUndefWeak() ||
                S->isTls() ||
                S->isObject() ||
                (S->isFunc() && !S->getName().startswith("__cross_domain")) ||
                (S->isDefined() && S->isLocal() && S->getName().startswith(".LJTI"))) &&
                      ~(S->isFile() || S->isSection());
        uint32_t pref = likely ? (pref_glob++) : (uint32_t)(~0);

        auto SEnt = Entries.Map.find(S);
        if(SEnt != Entries.Map.end()) {
          if(MainS) {
              assert(S == MainS); // We should only see symbols from the main symtab need entries.
          }
          if(prio > (*SEnt).second.priority) {
            (*SEnt).second.Index = pref;
            (*SEnt).second.priority = prio;
          }
        }
      }
    }
  }

  for (auto &it : Entries.Map) {
    if(it.second.IsFixed) {
      assert(it.second.Index.hasValue());
      assert(it.second.Index.getValue() < total_entries);
      assert(!taken_map[it.second.Index.getValue()]);
      taken_map[it.second.Index.getValue()] = true;
      assigned++;
    } else {
      Symbol* Sym = it.first;

      // Has a preferred index
      if(it.second.Index.hasValue()) {
        assert(map_index < unfixed_entries);
        SortMap[map_index].vector_index = vector_index;
        SortMap[map_index].priority = it.second.priority;
        map_index++;
      }
    }
    vector_index++;
  }

  // TODO: Currently sort of ignoring the small entry thing. In the case that we have small entries < small max < total
  // TODO: we need to assign small entries first and THEN other entries. Otherwise what we have now works if total <

  // Then sort the map. Give priority for placement first
  llvm::sort(SortMap, SortMap+map_index,
       [](sort_pair a, sort_pair b) -> bool
       {
         return a.priority > b.priority;
       }); // Items that are seen last come first

  // Now assign the indexs.
  // First give prefered positions.
  for(uint32_t i = 0; i != map_index; i++) {
    auto& entry = (Entries.Map.begin()+SortMap[i].vector_index)->second;
    Symbol* Sym = (Entries.Map.begin()+SortMap[i].vector_index)->first;
    assert(entry.Index.hasValue());
    uint32_t Position = entry.Index.getValue();
    if(Position == (uint32_t)-1) continue;
      Position += fixed_entries; // Fixed entries tend to go first and be of constant size. Bump everything up.

      if(Position < total_entries && !taken_map[Position]) {
        assigned++;
        taken_map[Position] = true;
        entry.Index = Position;
      } else {
        entry.Index.reset();
      }
  }

  uint32_t counted = 0;
  for(auto &it : Entries.Map) {
    counted++;
      auto& entry = it.second;
      if(!entry.Index.hasValue() || (entry.Index.getValue() == (uint32_t)(~0))) {
        while(taken_map[taken_till]) {
          taken_till++;
          assert(taken_till < total_entries);
        }
        assigned++;
        taken_map[taken_till] = true;
        entry.Index = taken_till;
        taken_till++;
      }
    assert(entry.Index.hasValue() && entry.Index.getValue() < total_entries);
    assert(taken_map[entry.Index.getValue()]);
  }


  assert(assigned == total_entries);

  /*
  for(uint32_t i = 0; i != total_entries; i++) {
      assert(taken_map[i]);
  }
  */
  for (auto &it : Entries.Map) {
    CapTableIndex& CTI = it.second;

    assert(CTI.Index.hasValue());
    if (CTI.IsFixed) continue;

    uint32_t Index = *CTI.Index;
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
    // If it is not used as a function pointer we can use
    // R_MIPS_CHERI_CAPABILITY_CALL instead which allows the runtime linker to
    // create non-unique plt stubs.
    RelType ElfCapabilityReloc = it.second.UsedAsFunctionPointer
                                     ? R_MIPS_CHERI_CAPABILITY
                                     : R_MIPS_CHERI_CAPABILITY_CALL;
    // All R_MIPS_CHERI_CAPABILITY_CALL relocations should end up in
    // the pltrel section rather than the normal relocation section to make
    // processing of PLT relocations in RTLD more efficient.
    RelocationBaseSection *DynRelSec =
        it.second.UsedAsFunctionPointer ? In.RelaDyn : In.RelaPlt;
    addCapabilityRelocation<ELFT>(
        *TargetSym, ElfCapabilityReloc, this, Off,
        R_CHERI_CAPABILITY, 0,
        [&]() { return "\n>>> referenced by " + RefName; }, DynRelSec);
  }

  return assigned;
}

template <class ELFT>
void CheriCapTableSection::assignValuesAndAddCapTableSymbols() {
  // FIXME: we should not be hardcoding architecture specific relocation numbers
  // here
  assert(Config->EMachine == EM_MIPS);

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
      In.RelaDyn->addReloc(Target->TlsModuleIndexRel, this, Offset, S);
    } else {
      // When building a shared library we still need a dynamic relocation
      // for the module index. Therefore only checking for
      // S->IsPreemptible is not sufficient (this happens e.g. for
      // thread-locals that have been marked as local through a linker script)
      if (!S->IsPreemptible && !Config->Pic)
        continue;
      In.RelaDyn->addReloc(Target->TlsModuleIndexRel, this, Offset, S);
      // However, we can skip writing the TLS offset reloc for non-preemptible
      // symbols since it is known even in shared libraries
      if (!S->IsPreemptible)
        continue;
      Offset += Config->Wordsize;
      In.RelaDyn->addReloc(Target->TlsOffsetRel, this, Offset, S);
    }
  }

  for (auto &it : TlsEntries.Map) {
    CapTableIndex &CTI = it.second;
    assert(!CTI.NeedsSmallImm);
    CTI.Index = TlsBaseIndex + AssignedTlsIndexes++;
    Symbol *S = it.first;
    uint64_t Offset = CTI.Index.getValue() * Config->Wordsize;
    if (S->IsPreemptible)
      In.RelaDyn->addReloc(Target->TlsGotRel, this, Offset, S);
  }

  ValuesAssigned = true;
}

SymbolTable* CheriCapTableSection::auxSymTab = nullptr;

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
  if (empty())
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
  // to use In.Symtab instead since we also want to add all local functions!
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

} // namespace elf
} // namespace lld
