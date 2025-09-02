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
  SymbolAndOffset(llvm::PointerUnion<Symbol *, InputSectionBase *> s, int64_t o)
      : symOrSec(s), offset(o) {
    assert(s && "Should not be null");
  }
  SymbolAndOffset(const SymbolAndOffset &) = default;
  SymbolAndOffset &operator=(const SymbolAndOffset &) = default;

  inline std::string verboseToString() const {
    assert(!symOrSec.isNull());
    SymbolAndOffset resolved = findRealSymbol();
    if (resolved.symOrSec.is<Symbol *>())
      return lld::verboseToString(resolved.symOrSec.get<Symbol *>(), offset);
    return resolved.symOrSec.get<InputSectionBase *>()->getObjMsg(offset);
  }
  // for __cap_relocs against local symbols clang emits section+offset instead
  // of the local symbol so that it still works even if the local symbol table
  // is stripped. This function tries to find the local symbol to a better match
  SymbolAndOffset findRealSymbol() const;
  SymbolAndOffset findSymbolForCapabilityRelocation() const;
  Symbol *sym() const {
    assert(symOrSec.is<Symbol *>());
    return symOrSec.get<Symbol *>();
  }
  bool operator==(const SymbolAndOffset &other) const {
    return symOrSec == other.symOrSec && offset == other.offset;
  }

  llvm::PointerUnion<Symbol *, InputSectionBase *> symOrSec = nullptr;
  int64_t offset = 0;
private:
  static SymbolAndOffset
  fromSectionWithOffset(InputSectionBase *isec, int64_t offset,
                        const SymbolAndOffset *Default = nullptr);
};

struct CheriCapRelocLocation {
  InputSectionBase *section;
  uint64_t offset;
  bool operator==(const CheriCapRelocLocation &other) const {
    return section == other.section && offset == other.offset;
  }
  std::string toString() const;
};

struct CheriCapReloc {
  // Indicates this is a relocation where the symbol is a function symbol but
  // we do not want any interposition, as this reference is for a specific
  // instruction within the function. Normal references to functions should not
  // set this.
  bool isCode;
  // We can't use a plain Symbol* here as capabilities to string constants
  // will be e.g. `.rodata.str + 0x90` -> need to store offset as well
  SymbolAndOffset target;
  int64_t capabilityOffset;
  bool operator==(const CheriCapReloc &other) const {
    return isCode == other.isCode && target == other.target &&
           capabilityOffset == other.capabilityOffset;
  }
};

class CheriCapRelocsSection : public SyntheticSection {
public:
  CheriCapRelocsSection(StringRef name);
  bool isNeeded() const override { return !relocsMap.empty(); }
  size_t getSize() const override { return relocsMap.size() * entsize; }
  void writeTo(uint8_t *buf) override;
  void addCapReloc(bool isCode, CheriCapRelocLocation loc,
                   const SymbolAndOffset &target, int64_t capabilityOffset,
                   Symbol *sourceSymbol = nullptr);

private:
  template <class ELFT> void writeToImpl(uint8_t *);
  bool addEntry(CheriCapRelocLocation loc, CheriCapReloc relocation) {
    auto it = relocsMap.insert(std::make_pair(loc, relocation));
    // assert(it.first->second == Relocation);
    if (!(it.first->second == relocation)) {
      error("Newly inserted relocation at " + loc.toString() +
            " does not match existing one:\n>   Existing: " +
            it.first->second.target.verboseToString() +
            ", cap offset=" + Twine(it.first->second.capabilityOffset) +
            ", is code=" + Twine(it.first->second.isCode) +
            "\n>   New:     " + relocation.target.verboseToString() +
            ", cap offset=" + Twine(relocation.capabilityOffset) +
            ", is code=" + Twine(relocation.isCode));
    }
    return it.second;
  }

  llvm::MapVector<CheriCapRelocLocation, CheriCapReloc> relocsMap;
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

class MipsCheriCapTableSection : public SyntheticSection {
public:
  MipsCheriCapTableSection();
  void addConstant(const Relocation &r) { addReloc(r); }
  // InputFile and Offset is needed in order to implement per-file/per-function
  // tables
  void addEntry(Symbol &sym, RelExpr expr, InputSectionBase *isec,
                uint64_t offset);
  void addDynTlsEntry(Symbol &sym);
  void addTlsIndex();
  void addTlsEntry(Symbol &sym);
  uint32_t getIndex(const Symbol &sym, const InputSectionBase *isec,
                    uint64_t offset) const;
  uint32_t getDynTlsOffset(const Symbol &sym) const;
  uint32_t getTlsIndexOffset() const;
  uint32_t getTlsOffset(const Symbol &sym) const;
  bool isNeeded() const override {
    return nonTlsEntryCount() != 0 || !dynTlsEntries.empty() ||
           !tlsEntries.empty();
  }
  void writeTo(uint8_t *buf) override;
  void assignValuesAndAddCapTableSymbols();
  size_t getSize() const override {
    size_t nonTlsEntries = nonTlsEntryCount();
    if (nonTlsEntries > 0 || !tlsEntries.empty() || !dynTlsEntries.empty()) {
      assert(config->capabilitySize > 0 &&
             "Cap table entries present but cap size unknown???");
    }
    size_t bytes =
        nonTlsEntryCount() * config->capabilitySize +
        (dynTlsEntries.size() * 2 + tlsEntries.size()) * config->wordsize;
    return llvm::alignTo(bytes, config->capabilitySize);
  }
private:
  struct CapTableIndex {
    // The index will be assigned once all symbols have been added
    // We do this so that we can order all symbols that need a small
    // immediate can be ordered before ones that are accessed using the
    // longer sequence of instructions
    // int64_t Index = -1;
    std::optional<uint32_t> index;
    bool needsSmallImm = false;
    bool usedInCallExpr = false;
    std::optional<SymbolAndOffset> firstUse;
  };
  struct CaptableMap {
    uint64_t firstIndex = std::numeric_limits<uint64_t>::max();
    llvm::MapVector<Symbol *, CapTableIndex> map;
    size_t size() const { return map.size(); }
    bool empty() const { return map.empty(); }
  };
  uint64_t assignIndices(uint64_t startIndex, CaptableMap &entries,
                         const Twine &symContext);
  /// @return a refernces to the captable map where the given symbol should
  /// be inserted. Usually this will just be the GlobalEntries map, but when
  /// compiling with the experimental per-function/per-file captables it will
  /// return a reference to the file/function that matches InputFile+Offset
  CaptableMap &getCaptableMapForFileAndOffset(const InputSectionBase *isec,
                                              uint64_t offset);
  size_t nonTlsEntryCount() const {
    size_t totalCount = globalEntries.size();
    if (LLVM_LIKELY(config->capTableScope == CapTableScopePolicy::All)) {
      assert(perFileEntries.empty() && perFunctionEntries.empty());
    } else {
      // Add all the per-file and per-function entries
      for (const auto &it : perFileEntries)
        totalCount += it.second.size();
      for (const auto &it : perFunctionEntries)
        totalCount += it.second.size();
    }
    return totalCount;
  }

  // The two maps are only used in the experimental
  llvm::MapVector<InputFile *, CaptableMap> perFileEntries;
  llvm::MapVector<Symbol *, CaptableMap> perFunctionEntries;
  CaptableMap globalEntries;
  CaptableMap dynTlsEntries;
  CaptableMap tlsEntries;
  bool valuesAssigned = false;
  friend class MipsCheriCapTableMappingSection;
};

// TODO: could shrink these to reduce size overhead but this is experimental
// code that will never be particularly efficient so it's fine
struct CaptableMappingEntry {
  uint64_t funcStart;      // virtual address relative to base address
  uint64_t funcEnd;        // virtual address relative to base address
  uint32_t capTableOffset; // offset in bytes into captable
  uint32_t subTableSize;   // Size in bytes of this sub-table
};

// Map from symbol vaddr -> captable subset so that RTLD can setup the correct
// trampolines to initialize $cgp to the correct subset
class MipsCheriCapTableMappingSection : public SyntheticSection {
public:
  MipsCheriCapTableMappingSection();
  bool isNeeded() const override {
    if (config->capTableScope == CapTableScopePolicy::All)
      return false;
    return in.mipsCheriCapTable && in.mipsCheriCapTable->isNeeded();
  }
  void writeTo(uint8_t *buf) override;
  size_t getSize() const override;
};

inline bool isSectionEndSymbol(StringRef name) {
  // Section end symbols like __preinit_array_end, etc. should actually be
  // zero size symbol since they are just markers for the end of a section
  // and not usable as a valid pointer
  return name.startswith("__stop_") ||
         (name.startswith("__") && name.endswith("_end")) || name == "end" ||
         name == "_end" || name == "etext" || name == "_etext" ||
         name == "edata" || name == "_edata";
}

inline bool isSectionStartSymbol(StringRef name) {
  // Section end symbols like __preinit_array_start, might end up pointing to
  // .text (see commments in Writer.cpp) if they are emtpy to avoid relocation
  // overflow. In that case returning a size of 0 is fine too.
  return name.startswith("__start_") ||
         (name.startswith("__") && name.endswith("_start")) ||
         name == "_DYNAMIC";
}

inline void readOnlyCapRelocsError(Symbol &sym, const Twine &sourceMsg) {
  error("attempting to add a capability relocation against " +
        (sym.getName().empty() ? "local symbol" : "symbol " + toString(sym)) +
        " in a read-only section; pass -Wl,-z,notext if you really want to do "
        "this" +
        sourceMsg);
}

void addRelativeCapabilityRelocation(
    InputSectionBase &isec, uint64_t offsetInSec,
    llvm::PointerUnion<Symbol *, InputSectionBase *> symOrSec, int64_t addend,
    RelExpr expr, RelType type);

// Align OutputSections as needed to ensure the bounds of capabilities
// such as PCC do not permit undesired access to portions of other
// OutputSections.  Return true if the alignment of any OutputSection
// was modified.
bool cheriCapabilityBoundsAlign();
} // namespace elf
} // namespace lld

namespace llvm {
template <> struct DenseMapInfo<lld::elf::CheriCapRelocLocation> {
  static inline lld::elf::CheriCapRelocLocation getEmptyKey() {
    return {nullptr, 0};
  }
  static inline lld::elf::CheriCapRelocLocation getTombstoneKey() {
    return {nullptr, std::numeric_limits<uint64_t>::max()};
  }
  static unsigned getHashValue(const lld::elf::CheriCapRelocLocation &val) {
    auto pair = std::make_pair(val.section, val.offset);
    return DenseMapInfo<decltype(pair)>::getHashValue(pair);
  }
  static bool isEqual(const lld::elf::CheriCapRelocLocation &lhs,
                      const lld::elf::CheriCapRelocLocation &rhs) {
    return lhs == rhs;
  }
};
} // namespace llvm
