#define DEBUG_TYPE "capability-register-invalidate"

#include "Mips.h"
#include "MipsTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace {
  class CheriInvalidatePass : public MachineFunctionPass {
    const MipsInstrInfo *InstrInfo;
    SmallVector<MachineInstr*, 16> StackStores;
    SmallVector<MachineInstr*, 4> Returns;
    public:
    static char ID;
    CheriInvalidatePass(MipsTargetMachine &TM) : MachineFunctionPass(ID) {
      InstrInfo = TM.getSubtargetImpl()->getInstrInfo();
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
      const Function *IRFunction = F.getFunction();
      const Module *Mod = IRFunction->getParent();
      NamedMDNode *SensitiveFunctions =
        Mod->getNamedMetadata("cheri.sensitive.functions");
      if (!SensitiveFunctions) return false;
      bool foundFunction = false;
      for (unsigned i=0 ; i<SensitiveFunctions->getNumOperands() ; i++) {
        Value *SensitiveFunction =
          cast<MDNode>(SensitiveFunctions->getOperand(i))->getOperand(0);
        if (SensitiveFunction == IRFunction) {
          foundFunction = true;
          break;
        }
      }
      if (!foundFunction) return false;


      DEBUG(dbgs() << "Zeroing stack spills\n");
      StackStores.clear();
      Returns.clear();
      SmallSet<int, 16> ZeroedLocations;
      for (MachineFunction::iterator FI = F.begin(), FE = F.end();
           FI != FE; ++FI)
        runOnMachineBasicBlock(*FI);

      if (StackStores.size() == 0) return false;

      for (SmallVector<MachineInstr*, 4>::iterator i=Returns.begin(),
           e=Returns.end() ; i!=e ; ++i) {
        MachineInstr *Ret = *i;
        ZeroedLocations.clear();
        for (SmallVector<MachineInstr*, 16>::iterator si=StackStores.begin(),
             se=StackStores.end() ; si!=se ; ++si) {
          MachineInstr *Store = *si;
          unsigned Opc = Store->getOpcode();
          MachineBasicBlock &MBB = *Ret->getParent();
          int FI = Store->getOperand(1).getIndex();
          // If we've already zeroed this location, skip it.
          if (ZeroedLocations.count(FI)) continue;
          ZeroedLocations.insert(FI);
          // If this is a capability store, then we just do a 64-bit integer
          // write.  This leaks information, but invalidates the capability.  
          if (Opc == Mips::STORECAP) {
            MachineInstrBuilder MIB(F, Ret);
            BuildMI(MBB, Ret, Ret->getDebugLoc(), InstrInfo->get(Mips::SD))
              .addReg(Mips::ZERO)
              .addFrameIndex(FI).addImm(Store->getOperand(2).getImm())
              .addMemOperand(InstrInfo->GetMemOperand(MBB, FI, MachineMemOperand::MOStore));
            DEBUG(dbgs() << "Zeroing capability spill\n");
          } else {
            // For other stores, we do the same type of store as was used for the spill, now with zeros.
            BuildMI(MBB, Ret, Ret->getDebugLoc(), InstrInfo->get(Opc))
              .addReg(Mips::ZERO)
              .addFrameIndex(FI).addImm(Store->getOperand(2).getImm())
              .addMemOperand(InstrInfo->GetMemOperand(MBB, FI, MachineMemOperand::MOStore));
            DEBUG(dbgs() << "Zeroing non-capability spill\n");
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
