#define DEBUG_TYPE "capability-register-invalidate"

#include "Mips.h"
#include "MipsTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"

using namespace llvm;

namespace {
  class CheriInvalidatePass : public MachineFunctionPass {
    const MipsInstrInfo *InstrInfo;
    SmallVector<MachineInstr*, 16> StackStores;
    SmallVector<MachineInstr*, 4> Returns;
    public:
    static char ID;
    CheriInvalidatePass(MipsTargetMachine &TM) : MachineFunctionPass(ID) {
      InstrInfo = TM.getInstrInfo();
    }

    void runOnMachineBasicBlock(MachineBasicBlock &MBB) {
      for (MachineBasicBlock::iterator I = MBB.instr_begin(); I != MBB.instr_end(); ++I) {
        int FI;
        MachineInstr *Inst = I;
        if (InstrInfo->isStoreToStackSlot(I, FI)) {
          StackStores.push_back(Inst);
        } else if (I->isReturn()) {
          Returns.push_back(I);
        }
      }
    }

    virtual bool runOnMachineFunction(MachineFunction &F) {
      fprintf(stderr, "Trying to eliminate stuff\n");
      StackStores.clear();
      Returns.clear();
      for (MachineFunction::iterator FI = F.begin(), FE = F.end();
           FI != FE; ++FI)
        runOnMachineBasicBlock(*FI);

      if (StackStores.size() == 0) return false;

      for (SmallVector<MachineInstr*, 4>::iterator i=Returns.begin(),
           e=Returns.end() ; i!=e ; ++i) {
        MachineInstr *Ret = *i;
        for (SmallVector<MachineInstr*, 16>::iterator si=StackStores.begin(),
             se=StackStores.end() ; si!=se ; ++si) {
          MachineInstr *Store = *si;
          if (Store->getOpcode() == Mips::STORECAP) {
            MachineInstrBuilder MIB(*i);
            MachineBasicBlock &MBB = *Ret->getParent();
            int FI = Store->getOperand(1).getIndex();
            BuildMI(MBB, Ret, Ret->getDebugLoc(), InstrInfo->get(Mips::SW))
              .addReg(Mips::ZERO)
              .addFrameIndex(FI).addImm(Store->getOperand(2).getImm())
              .addMemOperand(InstrInfo->GetMemOperand(MBB, FI, MachineMemOperand::MOStore));
          }
        }
      }

      return true;
    }
  };
}

char CheriInvalidatePass::ID;

FunctionPass *llvm::createCheriInvalidatePass(MipsTargetMachine &TM) {
  return new CheriInvalidatePass(TM);
}
