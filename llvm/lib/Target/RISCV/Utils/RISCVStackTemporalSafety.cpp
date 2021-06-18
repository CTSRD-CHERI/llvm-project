#include "RISCVStackTemporalSafety.h"

#include "llvm/Support/CommandLine.h"

namespace llvm {
namespace RISCVStackTemporalSafety {

namespace {

cl::opt<bool> EnableStackTemporalSafetyMitigations(
    "enable-stack-temporal-safety-mitigations",
    cl::desc("Enable mitigations to provide stack temporal memory safety"),
    cl::init(false));

//cl::opt<bool> DisableCapDerivedLifetimes(
//    "disable-cap-derived-lifetimes",
//    cl::desc("Disable the capability-derived lifetimes scheme"),
//    cl::init(false));

} // anonymous namespace

bool stackTemporalSafetyMitigationsEnabled() {
  return EnableStackTemporalSafetyMitigations;
}

} // namespace RISCVStackTemporalSafety
} // namespace llvm
