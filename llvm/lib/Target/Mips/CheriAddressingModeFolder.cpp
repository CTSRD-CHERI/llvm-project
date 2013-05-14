#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "Mips.h"
#include "MipsInstrInfo.h"

using namespace llvm;

static cl::opt<bool> DisableAddressingModeFolder(
  "disable-cheri-addressing-mode-folder",
  cl::init(false),
  cl::desc("Allow redundant capability manipulations"),
  cl::Hidden);

namespace {
struct CheriAddressingModeFolder : public MachineFunctionPass {
  static char ID;
  CheriAddressingModeFolder() : MachineFunctionPass(ID) {}
  virtual bool runOnMachineFunction(MachineFunction &MF) {
    if (DisableAddressingModeFolder) return false;

    MachineRegisterInfo &RI = MF.getRegInfo();
    std::set<MachineInstr*> IncBases;
    bool modified = false;
    for (MachineFunction::iterator BI=MF.begin(), BE=MF.end() ; BI!=BE ; ++BI)
      for (MachineBasicBlock::iterator I=BI->begin(), E=BI->end() ; I!=E ;
          ++I) {
        int Op = I->getOpcode();
        // Only look at cap-relative loads
        if (!(Op == Mips::CAPLOAD8 || Op == Mips::CAPLOADU8 ||
              Op == Mips::CAPLOAD16 || Op == Mips::CAPLOADU16 ||
              Op == Mips::CAPLOAD32 || Op == Mips::CAPLOAD64 ||
              Op == Mips::CAPSTORE8 || Op == Mips::CAPSTORE16 ||
              Op == Mips::CAPSTORE32 || Op == Mips::CAPSTORE64)) {
          continue;
        }
        // If the load is not currently at register-zero offset, we can't fix
        // it up to use relative addressing, so skip it.
        if (I->getOperand(1).getReg() != Mips::ZERO_64) continue;
        // Ignore ones that are not based on a CIncBase op
        MachineInstr *IncBase = RI.getUniqueVRegDef(I->getOperand(3).getReg());
        if (!IncBase || IncBase->getOpcode() != Mips::CIncBase) continue;
        assert(IncBase);

        MachineOperand Cap = IncBase->getOperand(1);
        MachineOperand Offset = IncBase->getOperand(2);
        assert(Cap.isReg());
        I->getOperand(1).setReg(Offset.getReg());
        I->getOperand(3).setReg(Cap.getReg());
        IncBases.insert(IncBase);
        modified = true;
      }
    for (std::set<MachineInstr*>::iterator I=IncBases.begin(),E=IncBases.end();
         I!=E ; ++I) {
      if (!RI.use_empty((*I)->getOperand(0).getReg())) continue;
      (*I)->eraseFromBundle();
    }
    return modified;
  }
};
}
char CheriAddressingModeFolder::ID;

MachineFunctionPass *llvm::createCheriAddressingModeFolder(void) {
  return new CheriAddressingModeFolder();
}
