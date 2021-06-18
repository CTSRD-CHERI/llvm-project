#ifndef LLVM_REVOCATIONSTRATEGY_H
#define LLVM_REVOCATIONSTRATEGY_H

#include "llvm/IR/Function.h"

namespace llvm {
namespace cheri {

/// Denotes the conditions under which a function will need a revocation sweep
/// before it returns.
enum class RevocationStrategy {

  /// No revocation required, this function is definitely safe.
  NoRevocation,

  /// This function includes lifetime checks, so we should test before any
  /// terminator whether any a StackLifetimeViolation exception happened and
  /// revoke if any did.
  ConditionalRevocation,

  /// This function should unconditionally have a revocation sweep before it
  /// terminates.
  UnconditionalRevocation
};

void setFunctionRevocationStrategy(Function &F, RevocationStrategy Strategy);

RevocationStrategy getFunctionRevocationStrategy(const Function &F);

} // namespace cheri
} // namespace llvm

#endif // LLVM_REVOCATIONSTRATEGY_H
