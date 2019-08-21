#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

class StackAllocNeedBoundsChecker {
public:
  StackAllocNeedBoundsChecker(AllocaInst *AI, const DataLayout &DL)
      : AI(AI), DL(DL) {
    AllocaSize = AI->getAllocationSizeInBits(DL);
  }
  bool check(const Use &U);
  void findUsesThatNeedBounds(SmallVectorImpl<const Use *> *UsesThatNeedBounds,
                              bool BoundAllUses, bool *MustUseSingleIntrinsic);

private:
  bool useNeedsBounds(const Use &U, const APInt &CurrentGEPOffset, unsigned Depth);
  bool anyUserNeedsBounds(const Instruction *I, const APInt &CurrentGEPOffset, unsigned Depth);
  bool canLoadStoreBeOutOfBounds(const Instruction *I, const Use& U, const APInt &CurrentGEPOffset, unsigned Depth);

  const AllocaInst *AI;
  const DataLayout &DL;
  Optional<uint64_t> AllocaSize;
};

// Returns true if a CSetBounds of size @p Size is safe to omit for user @p User
// and otherwise false.
// Examples include csetbounds instruction on allocas that are only accessed
// at statically known in-bounds offsets.
bool isSafeToOmitCheriSetBounds(const Instruction *User, const Value *Src, const Value* Size);

} // namespace llvm
