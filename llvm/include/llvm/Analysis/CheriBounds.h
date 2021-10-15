#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

class CheriNeedBoundsChecker {
public:
  CheriNeedBoundsChecker(AllocaInst *AI, const DataLayout &DL)
      : RootInst(AI), DL(DL) {
    auto AllocaSize = AI->getAllocationSizeInBits(DL);
    if (AllocaSize)
      MinSizeInBytes = *AllocaSize / 8;
    PointerAS = AI->getType()->getAddressSpace();
  }
  CheriNeedBoundsChecker(Instruction *I, Optional<uint64_t> MinSize,
                         const DataLayout &DL)
      : RootInst(I), DL(DL), MinSizeInBytes(MinSize) {
    assert(I->getType()->isPointerTy());
    PointerAS = I->getType()->getPointerAddressSpace();
  }
  bool check(const Use &U) const;
  void findUsesThatNeedBounds(SmallVectorImpl<const Use *> *UsesThatNeedBounds,
                              bool BoundAllUses,
                              bool *MustUseSingleIntrinsic) const;
  bool anyUseNeedsBounds() const;

private:
  bool useNeedsBounds(const Use &U, const APInt &CurrentGEPOffset,
                      unsigned Depth) const;
  bool anyUserNeedsBounds(const Instruction *I, const APInt &CurrentGEPOffset,
                          unsigned Depth) const;
  bool canLoadStoreBeOutOfBounds(const Instruction *I, const Use &U,
                                 const APInt &CurrentGEPOffset,
                                 unsigned Depth) const;

  const Instruction *RootInst;
  const DataLayout &DL;
  Optional<uint64_t> MinSizeInBytes;
  unsigned PointerAS = 0;
};

} // namespace llvm
