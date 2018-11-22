//===- CheriSetBounds.cpp - Functions to log information on CSetBounds ----===//
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

#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Transforms/Utils/Local.h"

#include <sys/file.h>
#include <unistd.h>

static llvm::cl::opt<llvm::cheri::StatsFormat, true> CollectPointerCastStats(
    "collect-csetbounds-stats", llvm::cl::ZeroOrMore, llvm::cl::Hidden,
    llvm::cl::desc("Collect statistics on numbers of int <-> pointer casts"),
    llvm::cl::location(llvm::cheri::ShouldCollectCSetBoundsStats),
    llvm::cl::values(
        clEnumValN(llvm::cheri::StatsOff, "off", "Do not collect statistics"),
        clEnumValN(llvm::cheri::StatsCSV, "csv", "Print stats in CSV format"),
        clEnumValN(llvm::cheri::StatsJSON, "json",
                   "Print stats in JSON format")));

namespace llvm {
namespace cheri {

StatsFormat ShouldCollectCSetBoundsStats = StatsOff;
ManagedStatic<CSetBoundsStatistics> CSetBoundsStats;

void CSetBoundsStatistics::add(unsigned KnownAlignment, Value *Length,
                               StringRef Pass, SetBoundsPointerSource Kind,
                               Twine Details, std::string SourceLoc) {
  Optional<uint64_t> KnownSize = None;
  if (auto CI = dyn_cast_or_null<ConstantInt>(Length)) {
    KnownSize = CI->getSExtValue();
  } else {
    // TODO: use KnownBits to infer something about size?
  }
  Entries.push_back({KnownAlignment, KnownSize, Kind, std::move(SourceLoc),
                     Pass, Details.str()});
}

void CSetBoundsStatistics::print(StatsOutputFile &S, StringRef MainFile) {
  print(S.stream(), MainFile, S.size() == 0);
}

void CSetBoundsStatistics::print(llvm::raw_ostream &OS, StringRef MainFile,
                                 bool PrintHeader) {
  if (ShouldCollectCSetBoundsStats == StatsJSON) {
    OS << "{ \"csetbounds_stats\": {";
    OS << "\n\t\"count\": " << Entries.size() << ',';
    OS << "\n\t\"details\": [";
    bool Comma = false;
    for (const Entry &E : Entries) {
      if (!Comma) {
        Comma = true;
        OS << "\n\t{";
      } else {
        OS << ",\n\t{";
      }
      OS << "\n\t\t\"alignment\": " << E.KnownAlignment;
      if (E.RequestedSize)
        OS << ",\n\t\t\"size\": " << *E.RequestedSize;
      else
        OS << ",\n\t\t\"size\": null,";
      OS << ",\n\t\t\"location\": \"" << llvm::yaml::escape(E.SourceLocation)
         << '"';
      if (!E.Pass.empty())
        OS << ",\n\t\t\"pass\": \"" << llvm::yaml::escape(E.Pass) << '"';
      if (!E.Details.empty())
        OS << ",\n\t\t\"details\": \"" << llvm::yaml::escape(E.Details) << '"';
      OS << "\n\t}";
    }
    OS << "\n\t]\n} }\n";
  } else if (ShouldCollectCSetBoundsStats == StatsCSV) {
    if (PrintHeader) {
      OS << "alignment_bits,size,kind,source_loc,compiler_pass,details\n";
    }
    for (const Entry &E : Entries) {
      // The analysis scripts expect alignment as a power of two instead of the
      // value
      unsigned AlignmentBits = 0;
      // Log2_64 will return -1 for 0 but we want 0 in that case
      if (E.KnownAlignment != 0)
        AlignmentBits = Log2_64(E.KnownAlignment);
      OS << AlignmentBits << ',';
      if (E.RequestedSize)
        OS << *E.RequestedSize;
      else
        OS << "<unknown>";

      if (E.PointerKind == SetBoundsPointerSource::Stack) {
        OS << ",s";
      } else if (E.PointerKind == SetBoundsPointerSource::Heap) {
        OS << ",h";
      } else if (E.PointerKind == SetBoundsPointerSource::SubObject) {
        OS << ",o";
      } else if (E.PointerKind == SetBoundsPointerSource::GlobalVar ||
                 E.PointerKind == SetBoundsPointerSource::CodePointer) {
        OS << ",g";
      } else {
        OS << ",?";
      }
      OS << ",\"" << llvm::yaml::escape(E.SourceLocation) << '"';
      OS << ",\"" << llvm::yaml::escape(E.Pass) << '"';
      OS << ",\"" << llvm::yaml::escape(E.Details) << '"';
      OS << "\n";
    }
  }
}

CSetBoundsStatistics::CSetBoundsStatistics() {
  assert(ShouldCollectCSetBoundsStats && "Created in invalid state");
}

CSetBoundsStatistics::~CSetBoundsStatistics() {
  // TODO: dump to stderr if not dumped already?
}

uint64_t StatsOutputFile::size() {
  sys::fs::file_status s;
  if (sys::fs::status(FD, s)) {
    return 0;
  }
  return s.getSize();
}

StatsOutputFile::~StatsOutputFile() {
  if (flock(FD, LOCK_UN) != 0) {
    errs() << "WARNING: unlocking statistics FD failed. Should be "
              "released automatically on exit anyway.\n";
  }
}

std::unique_ptr<StatsOutputFile>
StatsOutputFile::open(StringRef File, ErrorCallback OnOpenError,
                      ErrorCallback OnLockError) {
  // raw_fd_ostream has no way of getting the FD and I don't want to change
  // the interface just for this function to enable file locking since
  // that would require a recompile of all of LLVM....
  int StatsFD = STDERR_FILENO;
  bool CloseOnStreamDelete = true;
  if (!File.empty()) {
    std::error_code EC = sys::fs::openFileForWrite(
        File, StatsFD, sys::fs::CD_CreateAlways, sys::fs::OF_Append);
    if (EC) {
      OnOpenError(File, EC);
      return nullptr;
    }
  } else {
    File = "/dev/stderr";
    CloseOnStreamDelete = false;
  }
  assert(StatsFD != -1);
  if (flock(StatsFD, LOCK_EX) != 0) {
    // Lock error is not fatal we just get mixed output in the stats file
    OnLockError(File, std::error_code(errno, std::generic_category()));
  }
  // can't use make_unique here since the ctor is private
  return std::unique_ptr<StatsOutputFile>(new StatsOutputFile(
      StatsFD,
      llvm::make_unique<llvm::raw_fd_ostream>(StatsFD, CloseOnStreamDelete)));
}

} // namespace cheri
} // namespace llvm

std::string llvm::cheri::inferSourceLocation(const DebugLoc &DL,
                                             StringRef FunctionName) {
  std::string Result;
  raw_string_ostream OS(Result);
  if (DL) {
    DL.print(OS);
    OS.flush();
  }
  if (Result.empty() && !FunctionName.empty()) {
    OS << "<somewhere in " << FunctionName << ">";
    OS.flush();
  }
  return Result;
}

std::string llvm::cheri::inferSourceLocation(Instruction *AI) {
  assert(AI);
  std::string Result = inferSourceLocation(AI->getDebugLoc(), StringRef());
  if (Result.empty()) {
    // some instructions such as alloca instruction don't have a debug loc
    // attached directly so we need to look for calls to llvm.dbg.declare()
    SmallVector<DbgVariableIntrinsic *, 2> DbgVars;
    findDbgUsers(DbgVars, AI);
    for (auto &DbgV : DbgVars) {
      Result = inferSourceLocation(DbgV->getDebugLoc(), StringRef());
      if (!Result.empty())
        break;
    }
  }
  // No debug instructions found -> just fall back to function name
  if (Result.empty()) {
    Result = ("<somewhere in " + AI->getFunction()->getName() + ">").str();
  }
  return Result;
}

std::string llvm::cheri::inferLocalVariableName(AllocaInst *AI) {
  assert(AI);
  // try to find a
  // some instructions such as alloca instruction don't have a debug loc
  // attached directly so we need to look for calls to llvm.dbg.declare()
  SmallVector<DbgVariableIntrinsic *, 2> DbgVars;
  findDbgUsers(DbgVars, AI);
  for (auto &DbgV : DbgVars) {
    if (DbgV->isAddressOfVariable()) {
      return ("local variable " + DbgV->getVariable()->getName()).str();
    }
  }
  // No debug instructions found -> just fall back to alloca name
  if (AI->hasName())
    return ("AllocaInst " + AI->getName()).str();
  std::string Result;
  llvm::raw_string_ostream OS(Result);
  OS << "anonymous AllocaInst of type ";
  AI->getType()->print(OS);
  OS.flush();
  return Result;
}
