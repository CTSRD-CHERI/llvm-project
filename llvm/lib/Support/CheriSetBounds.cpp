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

#include "llvm/Support/CheriSetBounds.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/YAMLParser.h"

#include <sys/file.h>
#include <unistd.h>

#include "DebugOptions.h"

namespace {
struct CreateCollectSetBoundsStats {
  static void *call() {
    return new llvm::cl::opt<llvm::cheri::StatsFormat, true>(
        "collect-csetbounds-stats", llvm::cl::ZeroOrMore, llvm::cl::Hidden,
        llvm::cl::desc("Collect statistics on CSetBounds uses"),
        llvm::cl::location(llvm::cheri::ShouldCollectCSetBoundsStats),
        llvm::cl::values(clEnumValN(llvm::cheri::StatsOff, "off",
                                    "Do not collect statistics"),
                         clEnumValN(llvm::cheri::StatsCSV, "csv",
                                    "Print stats in CSV format"),
                         clEnumValN(llvm::cheri::StatsJSON, "json",
                                    "Print stats in JSON format")));
  }
};

struct CreateSetBoundsOutput {
  static void *call() {
    return new llvm::cl::opt<std::string>(
        "collect-csetbounds-output", llvm::cl::ZeroOrMore, llvm::cl::Hidden,
        llvm::cl::desc("output file for CSetBounds statistics"));
  }
};
static llvm::ManagedStatic<llvm::cl::opt<llvm::cheri::StatsFormat, true>,
                           CreateCollectSetBoundsStats>
    CollectSetBoundsStats;
static llvm::ManagedStatic<llvm::cl::opt<std::string>, CreateSetBoundsOutput>
    SetBoundsOutput;
} // namespace

void llvm::initSetBoundsOptions() {
  *CollectSetBoundsStats;
  *SetBoundsOutput;
}

namespace llvm {
namespace cheri {

StatsFormat ShouldCollectCSetBoundsStats = StatsOff;
ManagedStatic<CSetBoundsStatistics> CSetBoundsStats;

void CSetBoundsStatistics::add(Align KnownAlignment,
                               std::optional<uint64_t> KnownSize, StringRef Pass,
                               SetBoundsPointerSource Kind,
                               const Twine &Details, std::string SourceLoc,
                               std::optional<uint64_t> SizeMultipleOf) {
  Entries.push_back({KnownSize, SizeMultipleOf, KnownAlignment, Kind,
                     std::move(SourceLoc), Pass.str(), Details.str()});
}

StringRef CSetBoundsStatistics::outputFile() { return *SetBoundsOutput; }

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
      OS << "\n\t\t\"alignment\": " << E.KnownAlignment.value();
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
      unsigned AlignmentBits = Log2(E.KnownAlignment);
      OS << AlignmentBits << ',';
      if (E.RequestedSize) {
        OS << *E.RequestedSize;
      } else {
        // IF the size is unknown
        if (E.RequestedSizeMultipleOf)
          OS << "<unknown multiple of " << *E.RequestedSizeMultipleOf << ">";
        else
          OS << "<unknown>";
      }

      if (E.PointerKind == SetBoundsPointerSource::Stack) {
        OS << ",s";
      } else if (E.PointerKind == SetBoundsPointerSource::Heap) {
        OS << ",h";
      } else if (E.PointerKind == SetBoundsPointerSource::SubObject) {
        OS << ",o";
      } else if (E.PointerKind == SetBoundsPointerSource::GlobalVar) {
        OS << ",g";
      } else if (E.PointerKind == SetBoundsPointerSource::CodePointer) {
        OS << ",c";
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
  if (flock(FD, LOCK_UN) != 0 && errno != ENOTSUP) {
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
  if (flock(StatsFD, LOCK_EX) != 0 && errno != ENOTSUP) {
    // Lock error is not fatal we just get mixed output in the stats file
    OnLockError(File, std::error_code(errno, std::generic_category()));
  }
  // can't use make_unique here since the ctor is private
  return std::unique_ptr<StatsOutputFile>(new StatsOutputFile(
      StatsFD,
      std::make_unique<llvm::raw_fd_ostream>(StatsFD, CloseOnStreamDelete)));
}

} // namespace cheri
} // namespace llvm
