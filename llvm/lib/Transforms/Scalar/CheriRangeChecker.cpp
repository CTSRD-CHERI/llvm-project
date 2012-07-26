#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/CallSite.h"
#include "llvm/InstVisitor.h"

#include <string>
#include <utility>

#include "llvm/IR/Verifier.h"

using namespace llvm;
using std::pair;

namespace 
{
  class CheriRangeChecker : public FunctionPass,
                            public InstVisitor<CheriRangeChecker> {
    DataLayout *TD;
    Module *M;
    IntegerType *Int64Ty;
    PointerType *CapPtrTy;
    SmallVector<pair<PtrToIntInst*, IntToPtrInst*>, 32> Casts;
    struct ConstantCast {
      Instruction *Instr;
      unsigned OpNo;
      User *Origin;
    };
    SmallVector<ConstantCast, 32> ConstantCasts;
    public:
      static char ID;
      CheriRangeChecker() : FunctionPass(ID) {}
      virtual bool doInitialization(Module &Mod) {
        M = &Mod;
        TD = new DataLayout(M);
        Int64Ty  = IntegerType::get(M->getContext(), 64);
        CapPtrTy = PointerType::get(IntegerType::get(M->getContext(), 8), 200);
        return true;
      }
      virtual ~CheriRangeChecker() {
        delete TD;
      }
      bool checkOpcode(Value *V, unsigned Opcode) {
        if (Instruction *I = dyn_cast<Instruction>(V))
          return I->getOpcode() == Opcode;
        if (ConstantExpr *E = dyn_cast<ConstantExpr>(V))
          return E->getOpcode() == Opcode;
        return false;
      }
      User *testI2P(User &I2P) {
        PointerType *DestTy = dyn_cast<PointerType>(I2P.getType());
        if (DestTy && DestTy->getAddressSpace() == 200) {
          if (checkOpcode(I2P.getOperand(0), Instruction::PtrToInt)) {
            User *P2I = cast<User>(I2P.getOperand(0));
            PointerType *SrcTy = dyn_cast<PointerType>(P2I->getOperand(0)->getType());
            if (SrcTy && SrcTy->getAddressSpace() == 0) {
              Value *Src = P2I->getOperand(0)->stripPointerCasts();
              if (isa<AllocaInst>(Src) || isa<GlobalVariable>(Src)) {
                return P2I;
              }
            }
          }
        }
        return 0;
      }
      void visitIntToPtrInst(IntToPtrInst &I2P) {
        if (User *P2I = testI2P(I2P))
          Casts.push_back(pair<PtrToIntInst*,
              IntToPtrInst*>(cast<PtrToIntInst>(P2I),
                cast<IntToPtrInst>(&I2P)));
      }
      void visitRet(ReturnInst &RI) {
        Value *RV = RI.getReturnValue();
        if (RV && isa<ConstantExpr>(RV)) {
          ConstantCast C = { &RI, 0, testI2P(*cast<User>(RV)) };
          if (C.Origin)
            ConstantCasts.push_back(C);
        }
      }
      void visitCall(CallInst &CI) {
        for (unsigned i=0 ; i<CI.getNumOperands() ; i++) {
          Value *AV = CI.getOperand(i);
          if (AV && isa<ConstantExpr>(AV)) {
            ConstantCast C = { &CI, i, testI2P(*cast<User>(AV)) };
            if (C.Origin)
              ConstantCasts.push_back(C);
          }
        }
      }
      virtual bool runOnFunction(Function &F) {
        Casts.clear();
        ConstantCasts.clear();

        visit(F);

        if (!(Casts.empty() && ConstantCasts.empty())) {
          Function *SetLengthFn = Intrinsic::getDeclaration(M,
              Intrinsic::cheri_set_cap_length);

          for (pair<PtrToIntInst*, IntToPtrInst*> *i=Casts.begin(), *e=Casts.end() ; i!=e ; ++i) {
            PtrToIntInst *P2I = i->first;
            IntToPtrInst *I2P = i->second;
            Value *Src = P2I->getOperand(0);
            PointerType *AllocType = cast<PointerType>(Src->getType());
            uint64_t size = TD->getTypeAllocSize(AllocType->getElementType());
            ilist_iterator<llvm::Instruction> InsertPt = I2P->getParent()->begin();
            while (InsertPt.getNodePtrUnchecked() != (Instruction*)I2P) {
              ++InsertPt;
            }
            ++InsertPt;
            IRBuilder<> B(InsertPt.getNodePtrUnchecked());
            Instruction *BitCast = cast<Instruction>(B.CreateBitCast(I2P, CapPtrTy));
            CallInst *SetLength = B.CreateCall2(SetLengthFn, BitCast,
                ConstantInt::get(Int64Ty, size));
            Value *New = B.CreateBitCast(SetLength, I2P->getType());
            I2P->replaceAllUsesWith(New);
            BitCast->setOperand(0, I2P);
          }
          for (ConstantCast *i=ConstantCasts.begin(), *e=ConstantCasts.end() ;
              i!=e ; ++i) {
            Value *I2P = i->Instr->getOperand(i->OpNo);
            Value *Src = i->Origin->getOperand(0);
            PointerType *AllocType = cast<PointerType>(Src->getType());
            uint64_t size = TD->getTypeAllocSize(AllocType->getElementType());
            IRBuilder<> B(i->Instr);
            Value *BitCast = B.CreateBitCast(I2P, CapPtrTy);
            CallInst *SetLength = B.CreateCall2(SetLengthFn, BitCast,
                ConstantInt::get(Int64Ty, size));
            Value *New = B.CreateBitCast(SetLength, I2P->getType());
            i->Instr->setOperand(i->OpNo, New);
          }
          return true;
        }
        return false;
      }
  };
}

namespace llvm {
FunctionPass *createCheriRangeChecker(void) {
  return new CheriRangeChecker();
}
}
