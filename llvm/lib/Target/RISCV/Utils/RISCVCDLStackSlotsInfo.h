#ifndef LLVM_RISCVCAPDERIVEDLIFETIMESSTACKSLOTSINFO_H
#define LLVM_RISCVCAPDERIVEDLIFETIMESSTACKSLOTSINFO_H

#include "llvm/IR/RevocationStrategy.h"

#include <cassert>
#include <optional>

namespace llvm {
namespace cheri {

enum class CDLStackSlot { ViolationHappened, SavedSP, SavedFP };

/**
 * Manages the sizes and layout of the stack slots used by the
 * capability-derived lifetimes scheme to save information.
 */
class RISCVCDLStackSlotsInfo {
private:
  RevocationStrategy RevStrategy;
  bool HasFP;

public:
  RISCVCDLStackSlotsInfo(RevocationStrategy revStrategy, bool hasFP)
      : RevStrategy(revStrategy), HasFP(hasFP){};

  /// Determine the size of a particular stack slot.
  int getSize(CDLStackSlot slot) const;

  /// Determine the offset ABOVE the stack pointer of a particular stack slot.
  int getOffset(CDLStackSlot slot) const;

  /// Determine the total size of CDL-reserved stack space for this
  /// configuration.
  int totalSize() const;

  /// Returns true if the function uses a fixed frame pointer, and false
  /// otherwise.
  bool hasFP() const { return HasFP; }
};

} // namespace cheri
} // namespace llvm

#endif // LLVM_RISCVCAPDERIVEDLIFETIMESSTACKSLOTSINFO_H
