//===-- Cheri.cpp - Implement CHERI related utility methods   -------------===//
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

#include "llvm/IR/Cheri.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ScopedPrinter.h"
#include "llvm/Support/YAMLParser.h"

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
                               Twine Details, Instruction *DebugInst,
                               std::string KnownSourceLoc) {
  std::string SourceLoc = std::move(KnownSourceLoc);
  if (SourceLoc.empty()) {
    assert(DebugInst && "Missing instruction?");
    if (DILocation *Loc =
            DebugInst->getDebugLoc()) { // Here I is an LLVM instruction
      SourceLoc = (Loc->getFilename() + ":" + Twine(Loc->getLine()) + ":" +
                   Twine(Loc->getColumn()))
                      .str();
    }
    if (SourceLoc.empty()) {
      SourceLoc = ("<somewhere in " +
                   DebugInst->getParent()->getParent()->getName() + ">")
                      .str();
    }
  }
  Optional<uint64_t> KnownSize = None;
  if (auto CI = dyn_cast<ConstantInt>(Length)) {
    KnownSize = CI->getSExtValue();
  } else {
    // TODO: use KnownBits to infer something about size?
  }
  Entries.push_back(
      {KnownAlignment, KnownSize, Kind, SourceLoc, Pass, Details.str()});
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
      unsigned AlignmentBits = Log2_64(E.KnownAlignment);
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
