#pragma once

#include "../SymbolTable.h"
#include "../Symbols.h"
#include "../Target.h"
#include "../SyntheticSections.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/Support/Endian.h"

namespace lld {
namespace elf {

// See CheriBSD crt_init_globals()
template <llvm::support::endianness E> struct InMemoryCapRelocEntry {
  using CapRelocUint64 = llvm::support::detail::packed_endian_specific_integral<
      uint64_t, E, llvm::support::aligned>;
  InMemoryCapRelocEntry(uint64_t Loc, uint64_t Obj, uint64_t Off, uint64_t S,
                        uint64_t Perms)
      : capability_location(Loc), object(Obj), offset(Off), size(S),
        permissions(Perms) {}
  CapRelocUint64 capability_location;
  CapRelocUint64 object;
  CapRelocUint64 offset;
  CapRelocUint64 size;
  CapRelocUint64 permissions;
};

struct SymbolAndOffset {
  SymbolAndOffset(Symbol *S, int64_t O) : Sym(S), Offset(O) {}
  SymbolAndOffset(const SymbolAndOffset &) = default;
  SymbolAndOffset &operator=(const SymbolAndOffset &) = default;
  Symbol *Sym = nullptr;
  int64_t Offset = 0;

  // for __cap_relocs against local symbols clang emits section+offset instead
  // of the local symbol so that it still works even if the local symbol table
  // is stripped. This function tries to find the local symbol to a better match
  SymbolAndOffset findRealSymbol() const;

  static SymbolAndOffset fromSectionWithOffset(InputSectionBase *IS,
                                               uint64_t Offset,
                                               Symbol *Default = nullptr);

  template <typename ELFT> inline std::string verboseToString() const {
    return lld::verboseToString<ELFT>(Sym, Offset);
  }
};

struct CheriCapRelocLocation {
  InputSectionBase *Section;
  uint64_t Offset;
  bool NeedsDynReloc;
  bool operator==(const CheriCapRelocLocation &Other) const {
    return Section == Other.Section && Offset == Other.Offset;
  }
  template<typename ELFT>
  std::string toString() const;
};

struct CheriCapReloc {
  // We can't use a plain Symbol* here as capabilities to string constants
  // will be e.g. `.rodata.str + 0x90` -> need to store offset as well
  SymbolAndOffset Target;
  int64_t CapabilityOffset;
  bool NeedsDynReloc;
  bool operator==(const CheriCapReloc &Other) const {
    return Target.Sym == Other.Target.Sym &&
           Target.Offset == Other.Target.Offset &&
           CapabilityOffset == Other.CapabilityOffset &&
           NeedsDynReloc == Other.NeedsDynReloc;
  }
};

template <class ELFT> class CheriCapRelocsSection : public SyntheticSection {
public:
  CheriCapRelocsSection();
  static constexpr size_t RelocSize = 40;
  // Add a __cap_relocs section from in input object file
  void addSection(InputSectionBase *S);
  bool empty() const override { return RelocsMap.empty() && LegacyInputs.empty(); }
  size_t getSize() const override { return RelocsMap.size() * Entsize; }
  void finalizeContents() override;
  void writeTo(uint8_t *Buf) override;
  void addCapReloc(CheriCapRelocLocation Loc, const SymbolAndOffset &Target,
                   bool TargetNeedsDynReloc, int64_t CapabilityOffset,
                   Symbol *SourceSymbol = nullptr);

private:
  void processSection(InputSectionBase *S);
  bool addEntry(CheriCapRelocLocation Loc, CheriCapReloc Relocation) {
    auto it = RelocsMap.insert(std::make_pair(Loc, Relocation));
    // assert(it.first->second == Relocation);
    if (!(it.first->second == Relocation)) {
      error("Newly inserted relocation at " + Loc.toString<ELFT>() +
            " does not match existing one:\n>   Existing: " +
            it.first->second.Target.template verboseToString<ELFT>() +
            ", cap offset=" + Twine(it.first->second.CapabilityOffset) +
            ", dyn=" + Twine(it.first->second.NeedsDynReloc) +
            "\n>   New:     " + Relocation.Target.verboseToString<ELFT>() +
            ", cap offset=" + Twine(Relocation.CapabilityOffset) +
            ", dyn=" + Twine(Relocation.NeedsDynReloc));
    }
    return it.second;
  }
  // TODO: list of added dynamic relocations?

  llvm::MapVector<CheriCapRelocLocation, CheriCapReloc> RelocsMap;
  std::vector<InputSectionBase *> LegacyInputs;
  // If we have dynamic relocations we can't sort the __cap_relocs section
  // before writing it. TODO: actually we can but it will require refactoring
  bool ContainsDynamicRelocations = false;
  // If this is true reduce number of warnings for compat
  bool containsLegacyCapRelocs() const { return !LegacyInputs.empty(); }
};

class CheriCapTableSection : public SyntheticSection {
public:
  CheriCapTableSection();
  void addEntry(Symbol &Sym, bool NeedsSmallImm);
  uint32_t getIndex(const Symbol &Sym) const;
  bool empty() const override { return Entries.empty(); }
  void writeTo(uint8_t *Buf) override;
  template <class ELFT> void assignValuesAndAddCapTableSymbols();
  size_t getSize() const override {
    if (!Entries.empty() > 0)
      assert(Config->CapabilitySize > 0 &&
             "Cap table entries present but cap size unknown???");
    return Entries.size() * Config->CapabilitySize;
  }
private:
  struct CapTableIndex {
    // The index will be assigned once all symbols have been added
    // We do this so that we can order all symbols that need a small
    // immediate can be ordered before ones that are accessed using the
    // longer sequence of instructions
    // int64_t Index = -1;
    llvm::Optional<uint32_t> Index;
    bool NeedsSmallImm = false;
  };
  llvm::MapVector<Symbol *, CapTableIndex> Entries;
  bool ValuesAssigned = false;
};

template <typename ELFT, typename CallBack>
static void foreachGlobalSizesSymbol(InputSection *IS, CallBack &&CB) {
  assert(IS->Name == ".global_sizes");
  for (Symbol *B : IS->File->getSymbols()) {
    if (auto *D = dyn_cast<Defined>(B)) {
      if (D->Section != IS)
        continue;
      // skip the initial .global_sizes symbol (exists e.g. in
      // openpam_static_modules.o)
      if (D->isSection() && D->isLocal() && D->getName().empty())
        continue;
      StringRef Name = D->getName();
      if (!Name.startswith(".size.")) {
        error(".global_sizes symbol name is invalid: " +
              verboseToString<ELFT>(D));
        continue;
      }
      StringRef RealSymName = Name.drop_front(strlen(".size."));
      Symbol *Target = Symtab->find(RealSymName);
      CB(RealSymName, Target, D->Value);
    }
  }
}

inline bool isSectionEndSymbol(StringRef Name) {
  // Section end symbols like __preinit_array_end, etc. should actually be
  // zero size symbol since they are just markers for the end of a section
  // and not usable as a valid pointer
  return Name.startswith("__stop_") ||
         (Name.startswith("__") && Name.endswith("_end")) || Name == "end" ||
         Name == "_end" || Name == "etext" || Name == "_etext" ||
         Name == "edata" || Name == "_edata";
}

inline bool isSectionStartSymbol(StringRef Name) {
  // Section end symbols like __preinit_array_start, might end up pointing to
  // .text (see commments in Writer.cpp) if they are emtpy to avoid relocation
  // overflow. In that case returning a size of 0 is fine too.
  return Name.startswith("__start_") ||
         (Name.startswith("__") && Name.endswith("_start")) ||
         Name == "_DYNAMIC";
}

inline void readOnlyCapRelocsError(Symbol &Sym, const Twine &SourceMsg) {
  error("attempting to add a capability relocation against " +
        (Sym.getName().empty() ? "local symbol" : "symbol " + toString(Sym)) +
        " in a read-only section; pass -Wl,-z,notext if you really want to do "
        "this" +
        SourceMsg);
}

template <typename ELFT, typename ReferencedByFunc>
inline void addCapabilityRelocation(Symbol &Sym, RelType Type,
                                    InputSectionBase *Sec, uint64_t Offset,
                                    RelExpr Expr, int64_t Addend,
                                    ReferencedByFunc &&ReferencedBy) {
  // Emit either the legacy __cap_relocs section or a R_CHERI_CAPABILITY reloc
  // For local symbols we can also emit the untagged capability bits and
  // instruct csu/rtld to run CBuildCap
  assert(Config->ProcessCapRelocs);
  CapRelocsMode CapRelocMode = Sym.IsPreemptible
                                   ? Config->PreemptibleCapRelocsMode
                                   : Config->LocalCapRelocsMode;
  // local cap relocs don't need a Elf relocation with a full symbol lookup:
  if (CapRelocMode == CapRelocsMode::ElfReloc) {
    if (!Config->Pic && SharedFiles.empty()) {
      error(
          "attempting to emit a R_CAPABILITY relocation against " +
          (Sym.getName().empty() ? "local symbol" : "symbol " + toString(Sym)) +
          " in binary without a dynamic linker; try removing -Wl,-" +
          (Sym.IsPreemptible ? "preemptible" : "local") + "-caprelocs=elf" +
          ReferencedBy());
      return;
    }
    assert(Config->HasDynSymTab && "Should have been checked in Driver.cpp");
    // We don't use a R_MIPS_CHERI_CAPABILITY relocation for the input but
    // instead need to use an absolute pointer size relocation to write
    // the offset addend
    InX::RelaDyn->addReloc(Type, Sec, Offset, &Sym, Addend, Expr,
                           *Target->AbsPointerRel);
    // in the case that -local-caprelocs=elf is passed we need to ensure that
    // the target symbol is included in the dynamic symbol table
    if (!InX::DynSymTab) {
      error("R_CHERI_CAPABILITY relocations need a dynamic symbol table");
      return;
    }
    // The following is a hack for allowing R_MIPS_CHERI_CAPABILITY relocation
    // for local symbols (this should probably be removed in the future)
    if (!Sym.includeInDynsym()) {
      static std::vector<Symbol *> AddedToDynSymTab;
      // Ensure that it is included in the dynamic symbol table
      Sym.ExportDynamic = true;
      Sym.ForceExportDynamic = true;
      // Local symbols will not be part of Symtab so the code in Writer.cpp
      // will not add them to DynSymTab. Do it manually here if it hasn't
      // been added yet.
      if (Sym.isLocal() && !llvm::is_contained(AddedToDynSymTab, &Sym)) {
        Sym.Binding = llvm::ELF::STB_GLOBAL;
        Sym.Visibility = llvm::ELF::STV_INTERNAL;
        InX::DynSymTab->addSymbol(&Sym);
      }
    }
    if (!Sym.includeInDynsym()) {
      error("added a R_CHERI_CAPABILITY relocation but symbol not included "
            "in dynamic symbol: " +
            verboseToString<ELFT>(&Sym));
      return;
    }
  } else if (CapRelocMode == CapRelocsMode::Legacy) {
    In<ELFT>::CapRelocs->addCapReloc({Sec, Offset, Config->Pic}, {&Sym, 0u},
                                     Sym.IsPreemptible, Addend);
  } else {
    assert(Config->LocalCapRelocsMode == CapRelocsMode::CBuildCap);
    error("CBuildCap method not implemented yet!");
  }
}

} // namespace elf
} // namespace lld

namespace llvm {
template <> struct DenseMapInfo<lld::elf::CheriCapRelocLocation> {
  static inline lld::elf::CheriCapRelocLocation getEmptyKey() {
    return {nullptr, 0, false};
  }
  static inline lld::elf::CheriCapRelocLocation getTombstoneKey() {
    return {nullptr, std::numeric_limits<uint64_t>::max(), false};
  }
  static unsigned getHashValue(const lld::elf::CheriCapRelocLocation &Val) {
    auto Pair = std::make_pair(Val.Section, Val.Offset);
    return DenseMapInfo<decltype(Pair)>::getHashValue(Pair);
  }
  static bool isEqual(const lld::elf::CheriCapRelocLocation &LHS,
                      const lld::elf::CheriCapRelocLocation &RHS) {
    return LHS == RHS;
  }
};
} // namespace llvm
