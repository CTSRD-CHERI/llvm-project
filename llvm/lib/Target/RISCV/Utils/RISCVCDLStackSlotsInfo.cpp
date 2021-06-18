#include "RISCVCDLStackSlotsInfo.h"

#include "llvm/ADT/SmallVector.h"

namespace llvm {
namespace cheri {

namespace {

constexpr unsigned CapSize = 16;

// This is for alignment, not because we actually need that much data
constexpr unsigned ViolationHappenedMarkerSize = CapSize;

/// Defines the ordering of the CDL scheme's stack slots.
const SmallVector<CDLStackSlot, 3> &getOrder() {
  static SmallVector<CDLStackSlot, 3> Order{CDLStackSlot::ViolationHappened,
                                            CDLStackSlot::SavedSP,
                                            CDLStackSlot::SavedFP};
  return Order;
}

} // anonymous namespace

int RISCVCDLStackSlotsInfo::getSize(CDLStackSlot slot) const {
  switch (slot) {
  case CDLStackSlot::ViolationHappened:
    return RevStrategy == RevocationStrategy::ConditionalRevocation
               ? ViolationHappenedMarkerSize
               : 0;
  case CDLStackSlot::SavedSP:
    return CapSize;
  case CDLStackSlot::SavedFP:
    return HasFP ? CapSize : 0;
  }
}

int RISCVCDLStackSlotsInfo::getOffset(CDLStackSlot slot) const {
  int Offset = 0;
  for (CDLStackSlot Slot : getOrder()) {
    if (Slot == slot)
      break;
    Offset += getSize(Slot);
  }
  return Offset;
}

int RISCVCDLStackSlotsInfo::totalSize() const {
  return getSize(CDLStackSlot::ViolationHappened) +
         getSize(CDLStackSlot::SavedSP) + getSize(CDLStackSlot::SavedFP);
}

} // namespace cheri
} // namespace llvm
