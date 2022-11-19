#include "Mips.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Cheri.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/Transforms/Utils/Local.h"

#include <string>
#include <tuple>
#include <utility>

#define DEBUG_TYPE "cheri-range-checker"

using namespace llvm;
using std::pair;

namespace {
// Operands for an allocation.  Either one or two integers (constant or
// variable).  If there are two, then they must be multiplied together.
struct ValueSource {
  ValueSource() = default;
  Value *Base = nullptr;
  int64_t Offset = 0;
};
struct AllocOperands {
  AllocOperands() = default;
  Value *Size = nullptr;
  Value *SizeMultiplier = nullptr;
  ValueSource ValueSrc;
  cheri::SetBoundsPointerSource Src = cheri::SetBoundsPointerSource::Unknown;
  bool operator!=(const AllocOperands &Other) {
    return Size != Other.Size || SizeMultiplier != Other.SizeMultiplier ||
        ValueSrc.Base != Other.ValueSrc.Base ||
        ValueSrc.Offset != Other.ValueSrc.Offset || Src != Other.Src;
  }
};
class CheriRangeChecker : public FunctionPass,
                          public InstVisitor<CheriRangeChecker> {
  struct ConstantCast {
    Instruction *Instr;
    unsigned OpNo;
    User *Origin;
  };
  std::unique_ptr<DataLayout> TD;
  Module *M;
  IntegerType *SizeTy;
  PointerType *CapPtrTy;
  SmallVector<pair<AllocOperands, Instruction *>, 32> Casts;
  SmallVector<pair<AllocOperands, ConstantCast>, 32> ConstantCasts;
  Function *SetLengthFn;

  ValueSource getValueSource(Value *Src) {
    int64_t Offset = 0;
    Src = Src->stripPointerCasts();
    auto Base = GetPointerBaseWithConstantOffset(Src, Offset, *TD);
    if (Base && Base != Src) {
      LLVM_DEBUG(dbgs() << "Found base: "; Base->dump());
      Src = Base;
    }
    return ValueSource{Src, Offset};
  }

  AllocOperands getRangeForAllocation(ValueSource Src) {
    // FIXME: This should not hardcode function names but instead use the
    //  alloc_size attribute!
    if (auto Malloc = dyn_cast<CallBase>(Src.Base)) {
      Function *Fn = Malloc->getCalledFunction();
      if (!Fn)
        return AllocOperands();
      switch (StringSwitch<int>(Fn->getName())
                  .Case("malloc", 1)
                  .Case("valloc", 1)
                  .Case("realloc", 2)
                  .Case("aligned_alloc", 2)
                  .Case("reallocf", 2)
                  .Case("calloc", 3)
                  .Default(-1)) {
      default:
        return AllocOperands();
      case 1:
        return AllocOperands{Malloc->getArgOperand(0), nullptr, Src,
                             cheri::SetBoundsPointerSource::Heap};
      case 2:
        return AllocOperands{Malloc->getArgOperand(1), nullptr, Src,
                             cheri::SetBoundsPointerSource::Heap};
      case 3:
        return AllocOperands{Malloc->getArgOperand(0), Malloc->getArgOperand(1),
                             Src, cheri::SetBoundsPointerSource::Heap};
      }
    } else if (AllocaInst *AI = dyn_cast<AllocaInst>(Src.Base)) {
      PointerType *AllocaTy = AI->getType();
      Value *ArraySize = AI->getArraySize();
      Type *AllocationTy = AllocaTy->getElementType();
      unsigned ElementSize = TD->getTypeAllocSize(AllocationTy);
      if (ElementSize == 1)
        return AllocOperands{ArraySize, nullptr, Src,
                             cheri::SetBoundsPointerSource::Stack};
      Value *Size = ConstantInt::get(ArraySize->getType(), ElementSize);
      return AllocOperands{Size, ArraySize, Src,
                           cheri::SetBoundsPointerSource::Stack};
    }
    return AllocOperands();
  }
  Value *RangeCheckedValue(Instruction *InsertPt, AllocOperands AO, Value *I2P,
                           Value *&BitCast) {
    LLVM_DEBUG(dbgs() << "Adding RangeChecker bounds\n";
               dbgs() << "\tCast = "; I2P->dump();
               dbgs() << "\tBase = "; AO.ValueSrc.Base->dump();
               dbgs() << "\tOffset = " << AO.ValueSrc.Offset << "\n";);
    IRBuilder<> B(InsertPt);
    Value *Size =
        AO.SizeMultiplier ? B.CreateMul(AO.Size, AO.SizeMultiplier) : AO.Size;
    BitCast = B.CreatePointerBitCastOrAddrSpaceCast(AO.ValueSrc.Base, CapPtrTy);
    if (Size->getType() != SizeTy)
      Size = B.CreateZExt(Size, SizeTy);
    CallInst *SetLength = B.CreateCall(SetLengthFn, {BitCast, Size});
    if (cheri::ShouldCollectCSetBoundsStats) {
      Value *AlignmentSource = BitCast;
      Instruction *DebugInst = dyn_cast<Instruction>(AlignmentSource);
      if (!DebugInst)
        DebugInst = InsertPt;
      cheri::addSetBoundsStats(Align(getKnownAlignment(AlignmentSource, *TD)),
                               Size, getPassName(), AO.Src, "",
                               cheri::inferSourceLocation(DebugInst));
    }
    if (BitCast == AO.ValueSrc.Base)
      BitCast = SetLength;
    Value* Result = SetLength;
    if (AO.ValueSrc.Offset != 0) {
      LLVM_DEBUG(dbgs() << "Inserting GEP for non-zero Offset "
                        << AO.ValueSrc.Offset << "\n";
                     BitCast->dump(););
      Result = B.CreateConstGEP1_64(B.getInt8Ty(), Result, AO.ValueSrc.Offset,
                                    "offs");
    }
    return B.CreateBitCast(Result, I2P->getType());
  }

public:
  static char ID;
  CheriRangeChecker() : FunctionPass(ID) {}
  StringRef getPassName() const override { return "CHERI range checker"; }
  bool doInitialization(Module &Mod) override {
    M = &Mod;
    TD = std::make_unique<DataLayout>(M);
    SizeTy = IntegerType::get(M->getContext(), TD->getIndexSizeInBits(200));
    CapPtrTy = PointerType::get(IntegerType::get(M->getContext(), 8), 200);
    return true;
  }
  virtual ~CheriRangeChecker() {}
  bool checkOpcode(Value *V, unsigned Opcode) {
    if (Instruction *I = dyn_cast<Instruction>(V))
      return I->getOpcode() == Opcode;
    if (ConstantExpr *E = dyn_cast<ConstantExpr>(V))
      return E->getOpcode() == Opcode;
    return false;
  }

  User *testI2P(User &I2P) {
    PointerType *DestTy = dyn_cast<PointerType>(I2P.getType());
    if (DestTy && isCheriPointer(DestTy, TD.get())) {
      if (checkOpcode(I2P.getOperand(0), Instruction::PtrToInt)) {
        User *P2I = cast<User>(I2P.getOperand(0));
        PointerType *SrcTy =
            dyn_cast<PointerType>(P2I->getOperand(0)->getType());
        if (SrcTy && SrcTy->getAddressSpace() == 0) {
          Value *Src = P2I->getOperand(0)->stripPointerCasts();
          if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Src))
            return GV->hasExternalLinkage() ? 0 : P2I;
          if (isa<AllocaInst>(Src) || isa<CallBase>(Src)) {
            return P2I;
          }
        }
      }
    }
    return 0;
  }
  void visitAddrSpaceCast(AddrSpaceCastInst &ASC) {
    PointerType *DestTy = dyn_cast<PointerType>(ASC.getType());
    PointerType *SrcTy = dyn_cast<PointerType>(ASC.getOperand(0)->getType());
    LLVM_DEBUG(dbgs() << "Visiting address space cast: "; ASC.dump());

    if ((DestTy && isCheriPointer(DestTy, TD.get())) &&
        (SrcTy && SrcTy->getAddressSpace() == 0)) {
      auto Src = getValueSource(ASC.getOperand(0));
      if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Src.Base)) {
        if (GV->hasExternalLinkage())
          return;
      } else if (!(isa<AllocaInst>(Src.Base) || isa<CallBase>(Src.Base)))
        return;
      AllocOperands AO = getRangeForAllocation(Src);
      if (AO != AllocOperands())
        Casts.push_back(pair<AllocOperands, Instruction *>(AO, &ASC));
    }
  }
  void visitIntToPtrInst(IntToPtrInst &I2P) {
    if (User *P2I = testI2P(I2P)) {
      auto Src = getValueSource(P2I->getOperand(0));
      AllocOperands AO = getRangeForAllocation(Src);
      if (AO != AllocOperands())
        Casts.push_back(pair<AllocOperands, Instruction *>(AO, &I2P));
    }
  }
  void visitRet(ReturnInst &RI) {
    Value *RV = RI.getReturnValue();
    if (RV && isa<ConstantExpr>(RV)) {
      ConstantCast C = {&RI, 0, testI2P(*cast<User>(RV))};
      if (C.Origin) {
        auto Src = getValueSource(C.Origin->getOperand(0));
        AllocOperands AO = getRangeForAllocation(Src);
        if (AO != AllocOperands())
          ConstantCasts.push_back(pair<AllocOperands, ConstantCast>(AO, C));
      }
    }
  }
  void visitCall(CallInst &CI) {
    for (unsigned i = 0; i < CI.getNumOperands(); i++) {
      Value *AV = CI.getOperand(i);
      if (AV && isa<ConstantExpr>(AV)) {
        ConstantCast C = {&CI, i, testI2P(*cast<User>(AV))};
        if (C.Origin) {
          auto Src = getValueSource(C.Origin->getOperand(0));
          AllocOperands AO = getRangeForAllocation(Src);
          if (AO != AllocOperands())
            ConstantCasts.push_back(pair<AllocOperands, ConstantCast>(AO, C));
        }
      }
    }
  }
  bool runOnFunction(Function &F) override{
    Casts.clear();
    ConstantCasts.clear();

    visit(F);

    if (!(Casts.empty() && ConstantCasts.empty())) {
      Intrinsic::ID SetLength = Intrinsic::cheri_cap_bounds_set;
      SetLengthFn = Intrinsic::getDeclaration(M, SetLength, SizeTy);
      Value *BitCast = 0;

      for (auto *i = Casts.begin(), *e = Casts.end(); i != e; ++i) {
        Instruction *I2P = i->second;
        auto InsertPt = I2P->getParent()->begin();
        while (&(*InsertPt) != I2P) {
          ++InsertPt;
        }
        ++InsertPt;
        Value *New = RangeCheckedValue(&*InsertPt, i->first,
                                       I2P, BitCast);
        LLVM_DEBUG(dbgs() << "Replacing "; I2P->dump(); dbgs() << "  with "; New->dump());
        I2P->replaceAllUsesWith(New);
        // XXX: why was this needed?
        // cast<Instruction>(BitCast)->setOperand(0, I2P);
        RecursivelyDeleteTriviallyDeadInstructions(I2P);
      }
      for (pair<AllocOperands, ConstantCast> *i = ConstantCasts.begin(),
                                             *e = ConstantCasts.end();
           i != e; ++i) {
        Value *I2P = i->second.Instr->getOperand(i->second.OpNo);
        Value *New = RangeCheckedValue(i->second.Instr, i->first, I2P, BitCast);
        i->second.Instr->setOperand(i->second.OpNo, New);
      }
      return true;
    }
    return false;
  }
};
}

char CheriRangeChecker::ID;
INITIALIZE_PASS(CheriRangeChecker, DEBUG_TYPE, "CHERI rage checker", false,
                false)

namespace llvm {
FunctionPass *createCheriRangeChecker(void) { return new CheriRangeChecker(); }
} // namespace llvm
