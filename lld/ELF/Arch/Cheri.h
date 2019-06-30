#pragma once

#include "../SymbolTable.h"
#include "../Symbols.h"
#include "../SyntheticSections.h"
#include "../Target.h"
#include "../Writer.h"
#include "lld/Common/ErrorHandler.h"
#include "lld/Common/Memory.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/Support/Endian.h"

namespace lld {
namespace elf {

struct SymbolAndOffset {
public:
  SymbolAndOffset(Symbol *S, int64_t O) : SymOrSec(S), Offset(O) {
    assert(S && "Should not be null");
  }
  SymbolAndOffset(const SymbolAndOffset &) = default;
  SymbolAndOffset &operator=(const SymbolAndOffset &) = default;

  // for __cap_relocs against local symbols clang emits section+offset instead
  // of the local symbol so that it still works even if the local symbol table
  // is stripped. This function tries to find the local symbol to a better match
  SymbolAndOffset findRealSymbol() const;

  static SymbolAndOffset
  fromSectionWithOffset(InputSectionBase *IS, int64_t Offset,
                        const SymbolAndOffset *Default = nullptr);

  inline std::string verboseToString() const {
    assert(!SymOrSec.isNull());
    if (SymOrSec.is<Symbol *>())
      return lld::verboseToString(SymOrSec.get<Symbol *>(), Offset);
    assert(SymOrSec.is<InputSectionBase *>());
    InputSectionBase *IS = SymOrSec.get<InputSectionBase *>();
    return IS->getObjMsg(Offset);
  }
  Symbol *sym() const { return SymOrSec.get<Symbol *>(); }

  llvm::PointerUnion<Symbol *, InputSectionBase *> SymOrSec = nullptr;
  int64_t Offset = 0;
private:
  SymbolAndOffset(InputSectionBase *IS, int64_t O) : SymOrSec(IS), Offset(O) {
    assert(IS && "Should not be null");
  }
};

struct CheriCapRelocLocation {
  InputSectionBase *Section;
  uint64_t Offset;
  bool NeedsDynReloc;
  bool operator==(const CheriCapRelocLocation &Other) const {
    return Section == Other.Section && Offset == Other.Offset;
  }
  std::string toString() const;
};

struct CheriCapReloc {
  // We can't use a plain Symbol* here as capabilities to string constants
  // will be e.g. `.rodata.str + 0x90` -> need to store offset as well
  SymbolAndOffset Target;
  int64_t CapabilityOffset;
  bool NeedsDynReloc;
  bool operator==(const CheriCapReloc &Other) const {
    return Target.SymOrSec == Other.Target.SymOrSec &&
           Target.Offset == Other.Target.Offset &&
           CapabilityOffset == Other.CapabilityOffset &&
           NeedsDynReloc == Other.NeedsDynReloc;
  }
};

template <class ELFT> class CheriCapRelocsSection : public SyntheticSection {
public:
  CheriCapRelocsSection();
  static constexpr size_t FieldSize = ELFT::Is64Bits ? 8 : 4;
  static constexpr size_t RelocSize = FieldSize * 5;
  // Add a __cap_relocs section from in input object file
  void addSection(InputSectionBase *S);
  bool isNeeded() const override { return !RelocsMap.empty() || !LegacyInputs.empty(); }
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
      error("Newly inserted relocation at " + Loc.toString() +
            " does not match existing one:\n>   Existing: " +
            it.first->second.Target.verboseToString() +
            ", cap offset=" + Twine(it.first->second.CapabilityOffset) +
            ", dyn=" + Twine(it.first->second.NeedsDynReloc) +
            "\n>   New:     " + Relocation.Target.verboseToString() +
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

// General cap table structure (for CapSize = 2*WordSize):
//
// +-------------------------------+
// |       Small Index Cap 1       |
// +-------------------------------+
// |               :               |
// +-------------------------------+
// |       Small Index Cap m       |
// +-------------------------------+
// |       Large Index Cap 1       |
// +-------------------------------+
// |               :               |
// +-------------------------------+
// |       Large Index Cap n       |
// +---------------+---------------+
// | Dyn TLS Mod 1 | Dyn TLS Off 2 |
// +---------------+---------------+
// |               :               |
// +---------------+---------------+
// | Dyn TLS Mod p | Dyn TLS Off p |
// +---------------+---------------+
// |  IE TPREL 1   |  IE TPREL 2   |
// +---------------+---------------+
// |               :               |
// +---------------+---------------+
// | IE TPREL q-1  |  IE TPREL q   |
// +-------------------------------+
//
// If we are compiling with per function/per file tables it starts as follows:
// +---------------------------------------+
// | Small Index Cap 1 for File/Function 1 |
// +---------------------------------------+
// |                :                      |
// +---------------------------------------+
// | Small Index Cap m for File/Function 1 |
// +---------------------------------------+
// | Large Index Cap 1 for File/Function 1 |
// +---------------------------------------+
// |                :                      |
// +---------------------------------------+
// | Large Index Cap n for File/Function 1 |
// +---------------------------------------+
// | Small Index Cap 1 for File/Function 2 |
// +---------------------------------------+
// |                :                      |
// +---------------------------------------+
// | Small Index Cap m for File/Function 2 |
// +---------------------------------------+
// | Large Index Cap 1 for File/Function 2 |
// +---------------------------------------+
// |                :                      |
// +---------------------------------------+
// | Large Index Cap n for File/Function 2 |
// +---------------------------------------+
// |                :                      |
// |                :                      |
// +---------------------------------------+
// | Small Index Cap 1 for File/Function N |
// +---------------------------------------+
// |                :                      |
// +---------------------------------------+
// | Small Index Cap m for File/Function N |
// +---------------------------------------+
// | Large Index Cap 1 for File/Function N |
// +---------------------------------------+
// |                :                      |
// +---------------------------------------+
// | Large Index Cap n for File/Function N |
// +---------------------------------------+
// TODO: TLS caps also need to be per file/function

class CheriCapTableSection : public SyntheticSection {
public:
  CheriCapTableSection();
  // InputFile and Offset is needed in order to implement per-file/per-function
  // tables
  void addEntry(Symbol &Sym, RelExpr Expr, InputSectionBase *IS,
                uint64_t Offset);
  void addDynTlsEntry(Symbol &Sym);
  void addTlsIndex();
  void addTlsEntry(Symbol &Sym);
  uint32_t getIndex(const Symbol &Sym, InputSectionBase *IS,
                    uint64_t Offset) const;
  uint32_t getDynTlsOffset(const Symbol &Sym) const;
  uint32_t getTlsIndexOffset() const;
  uint32_t getTlsOffset(const Symbol &Sym) const;
  bool isNeeded() const override {
    return nonTlsEntryCount() != 0 || !DynTlsEntries.empty() ||
           !TlsEntries.empty();
  }
  void writeTo(uint8_t *Buf) override;
  template <class ELFT> void assignValuesAndAddCapTableSymbols();
  size_t getSize() const override {
    size_t NonTlsEntries = nonTlsEntryCount();
    if (NonTlsEntries > 0 || !TlsEntries.empty() || !DynTlsEntries.empty()) {
      assert(Config->CapabilitySize > 0 &&
             "Cap table entries present but cap size unknown???");
    }
    size_t Bytes =
        nonTlsEntryCount() * Config->CapabilitySize +
        (DynTlsEntries.size() * 2 + TlsEntries.size()) * Config->Wordsize;
    return llvm::alignTo(Bytes, Config->CapabilitySize);
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
    bool UsedInCallExpr = false;
    llvm::Optional<SymbolAndOffset> FirstUse;
  };
  struct CaptableMap {
    uint64_t FirstIndex = std::numeric_limits<uint64_t>::max();
    llvm::MapVector<Symbol *, CapTableIndex> Map;
    size_t size() const { return Map.size(); }
    bool empty() const { return Map.empty(); }
  };
  template <class ELFT>
  uint64_t assignIndices(uint64_t StartIndex, CaptableMap &Entries,
                         const Twine &SymContext);
  /// @return a refernces to the captable map where the given symbol should
  /// be inserted. Usually this will just be the GlobalEntries map, but when
  /// compiling with the experimental per-function/per-file captables it will
  /// return a reference to the file/function that matches InputFile+Offset
  const CaptableMap &getCaptableMapForFileAndOffset(InputSectionBase *IS,
                                                    uint64_t Offset) const {
    return const_cast<CheriCapTableSection *>(this)
        ->getCaptableMapForFileAndOffset(IS, Offset);
  }
  CaptableMap &getCaptableMapForFileAndOffset(InputSectionBase *IS,
                                              uint64_t Offset);
  size_t nonTlsEntryCount() const {
    size_t TotalCount = GlobalEntries.size();
    if (LLVM_LIKELY(Config->CapTableScope == CapTableScopePolicy::All)) {
      assert(PerFileEntries.empty() && PerFunctionEntries.empty());
    } else {
      // Add all the per-file and per-function entries
      for (const auto &it : PerFileEntries)
        TotalCount += it.second.size();
      for (const auto &it : PerFunctionEntries)
        TotalCount += it.second.size();
    }
    return TotalCount;
  }

  // The two maps are only used in the experimental
  llvm::MapVector<InputFile *, CaptableMap> PerFileEntries;
  llvm::MapVector<Symbol *, CaptableMap> PerFunctionEntries;
  CaptableMap GlobalEntries;
  CaptableMap DynTlsEntries;
  CaptableMap TlsEntries;
  bool ValuesAssigned = false;
  friend class CheriCapTableMappingSection;
};

// TODO: could shrink these to reduce size overhead but this is experimental
// code that will never be particularly efficient so it's fine
struct CaptableMappingEntry {
  uint64_t FuncStart;      // virtual address relative to base address
  uint64_t FuncEnd;        // virtual address relative to base address
  uint32_t CapTableOffset; // offset in bytes into captable
  uint32_t SubTableSize;   // Size in bytes of this sub-table
};

// Map from symbol vaddr -> captable subset so that RTLD can setup the correct
// trampolines to initialize $cgp to the correct subset
class CheriCapTableMappingSection : public SyntheticSection {
public:
  CheriCapTableMappingSection();
  bool isNeeded() const override {
    if (Config->CapTableScope == CapTableScopePolicy::All)
      return false;
    return In.CheriCapTable && In.CheriCapTable->isNeeded();
  }
  void writeTo(uint8_t *Buf) override;
  size_t getSize() const override;
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
        error(".global_sizes symbol name is invalid: " + verboseToString(D));
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

template <typename ELFT>
void addCapabilityRelocation(Symbol *Sym, RelType Type, InputSectionBase *Sec,
                             uint64_t Offset, RelExpr Expr, int64_t Addend,
                             bool IsCallExpr,
                             llvm::function_ref<std::string()> ReferencedBy,
                             RelocationBaseSection *DynRelSec = nullptr);
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
