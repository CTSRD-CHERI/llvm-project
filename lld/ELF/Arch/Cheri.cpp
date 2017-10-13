#include "Cheri.h"
#include "../OutputSections.h"
#include "../SyntheticSections.h"
#include "../Target.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::ELF;

namespace lld {

template <typename ELFT>
static inline std::string verboseToString(elf::SymbolAndOffset Sym) {
  return verboseToString<ELFT>(Sym.Symbol, Sym.Offset);
}

namespace elf {
template <class ELFT>
CheriCapRelocsSection<ELFT>::CheriCapRelocsSection()
    : SyntheticSection(SHF_ALLOC, SHT_PROGBITS, 8, "__cap_relocs") {
  this->Entsize = RelocSize;
}

// TODO: copy MipsABIFlagsSection::create() instead of current impl?
template <class ELFT>
void CheriCapRelocsSection<ELFT>::addSection(InputSectionBase *S) {
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

  processSection(S);
}

template <class ELFT> void CheriCapRelocsSection<ELFT>::finalizeContents() {
  // TODO: sort by address for improved cache behaviour?
  // TODO: add the dynamic relocations here:
  //   for (const auto &I : RelocsMap) {
  //     // TODO: unresolved symbols -> add dynamic reloc
  //     const CheriCapReloc& Reloc = I.second;
  //     SymbolBody *LocationSym = I.first.first;
  //     uint64_t LocationOffset = I.first.second;
  //
  //   }
}

template <typename ELFT>
static SymbolAndOffset sectionWithOffsetToSymbol(InputSectionBase *IS,
                                                 uint64_t Offset,
                                                 SymbolBody *Src) {
  SymbolBody *FallbackResult = nullptr;
  uint64_t FallbackOffset = Offset;
  for (SymbolBody *B : IS->getFile<ELFT>()->getSymbols()) {
    if (auto *D = dyn_cast<DefinedRegular>(B)) {
      if (D->Section != IS)
        continue;
      if (D->Value <= Offset && Offset < D->Value + D->Size) {
        // XXXAR: should we accept any symbol that encloses or only exact
        // matches?
        if (D->Value == Offset && (D->isFunc() || D->isObject()))
          return {D, D->Value - Offset}; // perfect match
        FallbackResult = D;
        FallbackOffset = Offset - D->Value;
      }
    }
  }
  if (!FallbackResult) {
    assert(IS->Name.startswith(".rodata.str"));
    FallbackResult = Src;
  }
  // we should have found at least a section symbol
  assert(FallbackResult && "SHOULD HAVE FOUND A SYMBOL!");
  return {FallbackResult, FallbackOffset};
}

template <typename ELFT>
SymbolAndOffset SymbolAndOffset::findRealSymbol() const {
  if (!Symbol->isSection())
    return *this;

  if (DefinedRegular *DefinedSym = dyn_cast<DefinedRegular>(Symbol)) {
    if (InputSectionBase *IS =
            dyn_cast<InputSectionBase>(DefinedSym->Section)) {
      return sectionWithOffsetToSymbol<ELFT>(IS, Offset, Symbol);
    }
  }
  return *this;
}

template <class ELFT>
void elf::CheriCapRelocsSection<ELFT>::processSection(InputSectionBase *S) {
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
    SymbolBody *LocationSym =
        &S->getFile<ELFT>()->getRelocTargetSym(LocationRel);
    SymbolBody &TargetSym = S->getFile<ELFT>()->getRelocTargetSym(TargetRel);

    if (LocationSym->getFile() != S->File) {
      error("Expected capability relocation to point to " + toString(S->File) +
            " but got " + toString(LocationSym->getFile()));
      continue;
    }
    //    errs() << "Adding cap reloc at " << toString(LocationSym) << " type "
    //           << Twine((int)LocationSym.Type) << " against "
    //           << toString(TargetSym) << "\n";
    const uint64_t LocationOffset = LocationRel.r_addend;
    const uint64_t TargetOffset = TargetRel.r_addend;
    auto *RawInput = reinterpret_cast<const InMemoryCapRelocEntry<E> *>(
        S->Data.begin() + CapRelocsOffset);
    bool LocNeedsDynReloc = false;
    if (!isa<DefinedRegular>(LocationSym)) {
      error("Unhandled symbol kind for cap_reloc: " +
            Twine(LocationSym->kind()));
      continue;
    }

    const SymbolAndOffset RelocLocation{LocationSym, LocationOffset};
    const SymbolAndOffset RelocTarget{&TargetSym, TargetOffset};
    SymbolAndOffset RealLocation = RelocLocation.findRealSymbol<ELFT>();
    SymbolAndOffset RealTarget = RelocTarget.findRealSymbol<ELFT>();
    if (Config->VerboseCapRelocs) {
      message("Adding capability relocation at " +
              verboseToString<ELFT>(RealLocation) + "\nagainst " +
              verboseToString<ELFT>(RealTarget));
    }

    if (TargetSym.isUndefined()) {
      std::string Msg = "cap_reloc against undefined symbol: " +
                        toString(*RealTarget.Symbol) + "\n>>> referenced by " +
                        verboseToString<ELFT>(RealLocation);
      if (Config->AllowUndefinedCapRelocs)
        warn(Msg);
      else
        error(Msg);
      continue;
    }
    bool TargetNeedsDynReloc = false;
    if (TargetSym.IsPreemptible) {
      // Do we need this?
      // TargetNeedsDynReloc = true;
    }
    switch (TargetSym.kind()) {
    case SymbolBody::DefinedRegularKind:
      break;
    case SymbolBody::DefinedCommonKind:
      // TODO: do I need to do anything special here?
      // message("Common symbol: " + toString(TargetSym));
      break;
    case SymbolBody::SharedKind:
      if (Config->Static) {
        error("cannot create a capability relocation against a shared symbol"
              " when linking statically");
        continue;
      }
      // TODO: shouldn't undefined be an error?
      TargetNeedsDynReloc = true;
      break;
    default:
      error("Unhandled symbol kind for cap_reloc target: " +
            Twine(TargetSym.kind()));
      continue;
    }

    LocNeedsDynReloc = LocNeedsDynReloc || Config->Pic || Config->Pie;
    TargetNeedsDynReloc = TargetNeedsDynReloc || Config->Pic || Config->Pie;
    uint64_t CurrentEntryOffset = RelocsMap.size() * RelocSize;
    auto It = RelocsMap.insert(std::pair<CheriCapRelocLocation, CheriCapReloc>(
        {LocationSym, LocationOffset, LocNeedsDynReloc},
        {RealTarget, RawInput->offset, RawInput->size, TargetNeedsDynReloc}));
    if (!It.second) {
      // Maybe happens with vtables?
      error("Symbol already added to cap relocs");
      continue;
    }
    if (LocNeedsDynReloc) {
      assert(LocationSym->isSection()); // Needed because local symbols cannot
                                        // be used in dynamic relocations
      // TODO: do this better
      // message("Adding dyn reloc at " + toString(this) + "+0x" +
      // utohexstr(CurrentEntryOffset));
      assert(CurrentEntryOffset < getSize());
      // Add a dynamic relocation so that RTLD fills in the right base address
      // We only have the offset relative to the load address...
      // Ideally RTLD/crt_init_globals would just add the load address to all
      // cap_relocs entries that have a RELATIVE flag set instead of requiring a
      // full Elf_Rel/Elf_Rela Can't use RealLocation here because that will
      // usually refer to a local symbol
      In<ELFT>::RelaDyn->addReloc({Target->RelativeRel, this,
                                   CurrentEntryOffset, true, LocationSym,
                                   static_cast<int64_t>(LocationOffset)});
    }
    if (TargetNeedsDynReloc) {
      // Capability target is the second field -> offset + 8
      uint64_t OffsetInOutSec = CurrentEntryOffset + 8;
      assert(OffsetInOutSec < getSize());
      // message("Adding dyn reloc at " + toString(this) + "+0x" +
      // utohexstr(OffsetInOutSec) + " against " + toString(TargetSym)); In the
      // RELA case (not yet) addend is already written by writeTo() below
      int64_t Addend = Config->IsRela ? TargetOffset : 0;
      In<ELFT>::RelaDyn->addReloc({Target->RelativeRel, this, OffsetInOutSec,
                                   false, &TargetSym, Addend});
    }
  }
}

template <class ELFT> void CheriCapRelocsSection<ELFT>::writeTo(uint8_t *Buf) {
  constexpr endianness E = ELFT::TargetEndianness;
  static_assert(RelocSize == sizeof(InMemoryCapRelocEntry<E>),
                "cap relocs size mismatch");
  uint64_t Offset = 0;
  for (const auto &I : RelocsMap) {
    const CheriCapRelocLocation &Location = I.first;
    const CheriCapReloc &Reloc = I.second;
    SymbolBody *LocationSym = Location.BaseSym;
    int64_t LocationOffset = Location.Offset;
    // If we don't need a dynamic relocation just write the VA
    // We always write the virtual address here:
    // In the shared library case this will be an address relative to the load
    // address and will be handled by crt_init_globals. In the static case we
    // can compute the final virtual address
    uint64_t LocationVA = LocationSym->getVA(LocationOffset);
    // For the target the virtual address the addend is always zero so
    // if we need a dynamic reloc we write zero
    // TODO: would it be more efficient for local symbols to write the DSO VA
    // and add a relocation against the load address?
    // Also this would make llvm-objdump -C more useful because it would
    // actually display the symbol that the relocation is against
    uint64_t TargetVA = Reloc.Target.Symbol->getVA(Reloc.Target.Offset);
    if (Reloc.NeedsDynReloc && Reloc.Target.Symbol->IsPreemptible) {
      // If we have a relocation against a preemptible symbol (even in the
      // current DSO) we can't compute the virtual address here so we only write
      // the addend
      TargetVA = Reloc.Target.Offset;
    }
    uint64_t TargetOffset = Reloc.Offset;
    uint64_t TargetSize = Reloc.Target.Symbol->template getSize<ELFT>();
    if (TargetSize == 0) {
      bool WarnAboutUnknownSize = true;
      // currently clang doesn't emit the necessary symbol information for local
      // string constants such as: struct config_opt opts[] = { { ..., "foo" },
      // { ..., "bar" } }; As this pattern is quite common don't warn if the
      // target section is .rodata.str
      if (DefinedRegular *DefinedSym =
              dyn_cast<DefinedRegular>(Reloc.Target.Symbol)) {
        if (DefinedSym->isSection() &&
            DefinedSym->Section->Name.startswith(".rodata.str")) {
          WarnAboutUnknownSize = false;
        }
      }
      // TODO: are there any other cases that can be ignored?

      if (WarnAboutUnknownSize || Config->Verbose) {
        auto RealLocation =
            SymbolAndOffset(LocationSym, LocationOffset).findRealSymbol<ELFT>();
        warn("could not determine size of cap reloc against " +
             verboseToString<ELFT>(Reloc.Target) + "\n>>> referenced by " +
             verboseToString<ELFT>(RealLocation));
      }
      if (OutputSection *OS = Reloc.Target.Symbol->getOutputSection()) {
        assert(TargetVA >= OS->Addr);
        uint64_t OffsetInOS = TargetVA - OS->Addr;
        assert(OffsetInOS < OS->Size);
        TargetSize = OS->Size - OffsetInOS;
#if 0
        if (Config->VerboseCapRelocs)
          errs() << " OS OFFSET 0x" << utohexstr(OS->Addr) << "SYM OFFSET 0x"
                 << utohexstr(OffsetInOS) << " SECLEN 0x" << utohexstr(OS->Size)
                 << " -> target size 0x" << utohexstr(TargetSize) << "\n";
#endif
      } else {
        warn("Could not find size for symbol '" +
             verboseToString<ELFT>(Reloc.Target) +
             "' and could not determine section size. Using UINT64_MAX.");
        TargetSize = std::numeric_limits<uint64_t>::max();
      }
    }
    assert(TargetOffset <= TargetSize);
    uint64_t Permissions = Reloc.Target.Symbol->isFunc() ? 1ULL << 63 : 0;
    InMemoryCapRelocEntry<E> Entry{LocationVA, TargetVA, TargetOffset,
                                   TargetSize, Permissions};
    memcpy(Buf + Offset, &Entry, sizeof(Entry));
    //     if (Config->Verbose) {
    //       errs() << "Added capability reloc: loc=" << utohexstr(LocationVA)
    //              << ", object=" << utohexstr(TargetVA)
    //              << ", offset=" << utohexstr(TargetOffset)
    //              << ", size=" << utohexstr(TargetSize)
    //              << ", permissions=" << utohexstr(Permissions) << "\n";
    //     }
    Offset += RelocSize;
  }
  assert(Offset == getSize() && "Not all data written?");
}

template class elf::CheriCapRelocsSection<ELF32LE>;
template class elf::CheriCapRelocsSection<ELF32BE>;
template class elf::CheriCapRelocsSection<ELF64LE>;
template class elf::CheriCapRelocsSection<ELF64BE>;

} // namespace elf
} // namespace lld
