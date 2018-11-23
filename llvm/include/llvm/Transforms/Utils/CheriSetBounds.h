//===- CheriSetBounds.h - Functions to log information on CSetBounds ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This family of functions perform various local transformations to the
// program.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H
#define LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils.h"
#include <functional>

namespace llvm {

class Value;
class Instruction;

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

// This probably exists somewhere else
inline Optional<uint64_t> inferConstantValue(Value *V) {
  Optional<uint64_t> Result;
  if (auto CI = dyn_cast_or_null<ConstantInt>(V)) {
    Result = CI->getSExtValue();
  } else {
    // TODO: use KnownBits to infer something?
  }
  return Result;
}

class CSetBoundsStatistics {
public:
  struct Entry {
    uint64_t KnownAlignment;
    Optional<uint64_t> RequestedSize;
    Optional<uint64_t> RequestedSizeMultipleOf;
    SetBoundsPointerSource PointerKind;
    std::string SourceLocation;
    std::string Pass;
    std::string Details;
  };
  CSetBoundsStatistics();
  ~CSetBoundsStatistics();

  void add(unsigned KnownAlignment, Value *Length, StringRef Pass,
           SetBoundsPointerSource Kind, const Twine &Details,
           std::string SourceLoc, Optional<uint64_t> SizeMultipleOf = None) {
    add(KnownAlignment, inferConstantValue(Length), Pass, Kind, Details,
        std::move(SourceLoc), SizeMultipleOf);
  }
  void add(unsigned KnownAlignment, Optional<uint64_t> Length, StringRef Pass,
           SetBoundsPointerSource Kind, const Twine &Details,
           std::string SourceLoc, Optional<uint64_t> SizeMultipleOf = None);
  void print(llvm::raw_ostream &OS, StringRef MainFile, bool PrintHeader);
  void print(StatsOutputFile &S, StringRef MainFile);
  static StringRef outputFile();

private:
  SmallVector<Entry, 32> Entries;
};

inline SetBoundsPointerSource inferPointerSource(const Value *V) {
  // look through casts and find out if they are global/alloca/unknown
  return SetBoundsPointerSource::Unknown;
}

// These 3 helpers should probably be somewhere else

// Look at the attached debug info to get the name of the local variable or if
// not known return the name of the allocainst
std::string inferLocalVariableName(AllocaInst *AI);

// returns a source file + line if debug info is present valid, otherwise falls
// back to "<somewhere in $FUNCTION_NAME>"
std::string inferSourceLocation(Instruction *Inst);
std::string inferSourceLocation(const DebugLoc &DL, StringRef Function);

enum StatsFormat {
  StatsOff = 0,
  StatsCSV,
  StatsJSON,
};

extern StatsFormat ShouldCollectCSetBoundsStats;
extern ManagedStatic<CSetBoundsStatistics> CSetBoundsStats;

} // end namespace cheri

} // end namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H
