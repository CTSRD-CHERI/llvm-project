//===- llvm/IR/Cheri.h - various CHERI related utility functions     ----*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains various CHERI utilities. This is a separate file to avoid
// changes to headers that will require a full LLVM recompile.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_CHERI_H
#define LLVM_IR_CHERI_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

namespace llvm {

/// Try to get a DataLayout from an Instr/BB/Func/Module:
inline const DataLayout *getDataLayoutOrNull(const Module *M) {
  return M ? &M->getDataLayout() : nullptr;
}
inline const DataLayout *getDataLayoutOrNull(const Function *FuncForDL) {
  return FuncForDL ? getDataLayoutOrNull(FuncForDL->getParent()) : nullptr;
}
inline const DataLayout *getDataLayoutOrNull(const BasicBlock *BBForDL) {
  return BBForDL ? getDataLayoutOrNull(BBForDL->getParent()) : nullptr;
}
inline const DataLayout *getDataLayoutOrNull(const Instruction *InstForDL) {
  return InstForDL ? getDataLayoutOrNull(InstForDL->getParent()) : nullptr;
}

/// This function can be used if there is no DataLayout available and will
/// in that case assume that AS200 is only used for CHERI
inline bool isCheriPointer(unsigned AddrSpace, const DataLayout *DL) {
  // FIXME: this assumes no other backend uses AS200/201
  if (DL)
    return DL->isFatPointer(AddrSpace);
  return AddrSpace == 200 /* TODO: || AddrSpace == 201 */;
}

inline bool isCheriPointer(Type *Ty, const DataLayout *DL) {
  return Ty->isPointerTy() && isCheriPointer(Ty->getPointerAddressSpace(), DL);
}

#if 0
/// Same again but try to derive the DataLayout from an llvm::Module
inline bool isCheriPointer(Type* Ty, const Module* ModForDL) {
  return isCheriPointer(Ty, getDataLayoutOrNull(ModForDL));
}

/// Same again but try to derive the DataLayout from an llvm::Function
inline bool isCheriPointer(Type* Ty, const Function* FuncForDL) {
  return isCheriPointer(Ty, getDataLayoutOrNull(FuncForDL));
}

/// Same again but try to derive the DataLayout from an llvm::BasicBlock
inline bool isCheriPointer(Type* Ty, const BasicBlock* BBForDL) {
  return isCheriPointer(Ty, getDataLayoutOrNull(BBForDL));
}

/// Same again but try to derive the DataLayout from an llvm::Instruction
inline bool isCheriPointer(Type* Ty, const Instruction* InstForDL) {
  return isCheriPointer(Ty, getDataLayoutOrNull(InstForDL));
}
#endif

namespace cheri {

class StatsOutputFile {
  std::unique_ptr<llvm::raw_fd_ostream> Stream;
  int FD;
  explicit StatsOutputFile(int FD, std::unique_ptr<llvm::raw_fd_ostream> S)
      : Stream(std::move(S)), FD(FD) {}

public:
  using ErrorCallback = std::function<void(StringRef, const std::error_code &)>;
  llvm::raw_fd_ostream &stream() { return *Stream; }
  uint64_t size();
  ~StatsOutputFile();
  static std::unique_ptr<StatsOutputFile>
  open(StringRef File, ErrorCallback OnOpenError, ErrorCallback OnLockError);
};

enum class SetBoundsPointerSource {
  Unknown,
  Heap,
  Stack,
  GlobalVar,
  CodePointer,
  SubObject,
};

class CSetBoundsStatistics {
public:
  struct Entry {
    uint64_t KnownAlignment;
    Optional<uint64_t> RequestedSize;
    SetBoundsPointerSource PointerKind;
    std::string SourceLocation;
    std::string Pass;
    std::string Details;
  };
  CSetBoundsStatistics();
  ~CSetBoundsStatistics();
  void add(unsigned KnownAlignment, Value *Length, StringRef Pass,
           SetBoundsPointerSource Kind, Twine Details, Instruction *DebugInst);
  void print(llvm::raw_ostream &OS, StringRef MainFile, bool PrintHeader);
  void print(StatsOutputFile &S, StringRef MainFile);

private:
  SmallVector<Entry, 32> Entries;
};

enum StatsFormat {
  StatsOff = 0,
  StatsCSV,
  StatsJSON,
};

extern StatsFormat ShouldCollectCSetBoundsStats;
extern ManagedStatic<CSetBoundsStatistics> CSetBoundsStats;

} // end namespace cheri

} // end namespace llvm

#endif // LLVM_IR_CHERI_H
