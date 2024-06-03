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

#ifndef LLVM_SUPPORT_CHERISETBOUNDS_H
#define LLVM_SUPPORT_CHERISETBOUNDS_H

#include "llvm/ADT/FunctionExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Alignment.h"

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
  using ErrorCallback = llvm::unique_function<void(StringRef, const std::error_code &)>;
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
    std::optional<uint64_t> RequestedSize;
    std::optional<uint64_t> RequestedSizeMultipleOf;
    Align KnownAlignment;
    SetBoundsPointerSource PointerKind;
    std::string SourceLocation;
    std::string Pass;
    std::string Details;
  };
  CSetBoundsStatistics();
  ~CSetBoundsStatistics();

  void add(Align KnownAlignment, std::optional<uint64_t> Length, StringRef Pass,
           SetBoundsPointerSource Kind, const Twine &Details,
           std::string SourceLoc, std::optional<uint64_t> SizeMultipleOf = None);
  void print(llvm::raw_ostream &OS, StringRef MainFile, bool PrintHeader);
  void print(StatsOutputFile &S, StringRef MainFile);
  static StringRef outputFile();

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

#endif // LLVM_SUPPORT_CHERISETBOUNDS_H
