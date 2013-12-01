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
  bool tryToFoldAdd(unsigned vreg, MachineRegisterInfo &RI, MachineInstr *&AddInst, int64_t &offset) {
    AddInst = RI.getUniqueVRegDef(vreg);
    unsigned reg;
    int64_t off;
    // If this is an add-immediate then we can possibly fold it
    switch (AddInst->getOpcode()) {
      case Mips::DADDi:
      case Mips::DADDiu:
        // Don't try to fold in things that have relocations yet
        if (!AddInst->getOperand(2).isImm())
          return false;
        reg = AddInst->getOperand(1).getReg();
        off = AddInst->getOperand(2).getImm();
        break;
      default:
        return false;
    }
    // We potentially could fold non-zero adds, but we'll leave them for now.
    if (reg != Mips::ZERO_64)
      return false;
    off += offset;
    // If the result is too big to fit in the offset field, give up
    if (off < -127 || off > 127)
    {
      return false;
    }
    offset = off;
    return true;
  }
  bool foldMachineFunction(MachineFunction &MF) {
    if (DisableAddressingModeFolder) return false;

    MachineRegisterInfo &RI = MF.getRegInfo();
    std::set<MachineInstr*> IncBases;
    std::set<MachineInstr*> Adds;
    bool modified = false;
    for (MachineFunction::iterator BI=MF.begin(), BE=MF.end() ; BI!=BE ; ++BI)
      for (MachineBasicBlock::iterator I=BI->begin(), E=BI->end() ; I!=E ;
          ++I) {
        int Op = I->getOpcode();
        // Only look at cap-relative loads and stores
        if (!(Op == Mips::CAPLOAD8 || Op == Mips::CAPLOADU8 ||
              Op == Mips::CAPLOAD16 || Op == Mips::CAPLOADU16 ||
              Op == Mips::CAPLOAD32 || Op == Mips::CAPLOAD64 ||
              Op == Mips::CAPSTORE8 || Op == Mips::CAPSTORE16 ||
              Op == Mips::LOADCAP || Op == Mips::STORECAP ||
              Op == Mips::CAPSTORE32 || Op == Mips::CAPSTORE64)) {
          continue;
        }
        int64_t offset = I->getOperand(2).getImm();
        // If the load is not currently at register-zero offset, we can't fix
        // it up to use relative addressing, but we may be able to modify it so
        // that it is...
        if (I->getOperand(1).getReg() != Mips::ZERO_64) {
          MachineInstr *AddInst;
          if (tryToFoldAdd(I->getOperand(1).getReg(), RI, AddInst, offset)) {
            Adds.insert(AddInst);
            I->getOperand(2).setImm(offset);
            I->getOperand(1).setReg(Mips::ZERO_64);
          } else 
            continue;
        }
        // Ignore ones that are not based on a CIncBase op
        MachineInstr *IncBase = RI.getUniqueVRegDef(I->getOperand(3).getReg());
        if (!IncBase || IncBase->getOpcode() != Mips::CIncBase) continue;
        assert(IncBase);

        MachineOperand Cap = IncBase->getOperand(1);
        MachineOperand Offset = IncBase->getOperand(2);
        assert(Cap.isReg());
        MachineInstr *AddInst;
        // If the CIncBase is of a daddi[u] then we can potentially replace
        // both by just folding the 
        if (tryToFoldAdd(Offset.getReg(), RI, AddInst, offset)) {
          // the IncBase needs to be inserted first so that it will
          Adds.insert(AddInst);
          I->getOperand(1).setReg(Mips::ZERO_64);
          I->getOperand(2).setImm(offset);
        } else
          I->getOperand(1).setReg(Offset.getReg());
        I->getOperand(3).setReg(Cap.getReg());
        IncBases.insert(IncBase);
        modified = true;
      }
    for (std::set<MachineInstr*>::iterator I=IncBases.begin(),E=IncBases.end();
         I!=E ; ++I) {
      if (!RI.use_empty((*I)->getOperand(0).getReg())) {
        continue;
      }
      (*I)->eraseFromBundle();
    }
    for (std::set<MachineInstr*>::iterator I=Adds.begin(),E=Adds.end();
         I!=E ; ++I) {
      if (!RI.use_empty((*I)->getOperand(0).getReg())) {
        continue;
      }
      (*I)->eraseFromBundle();
    }
    return modified;
  }
  virtual bool runOnMachineFunction(MachineFunction &MF) {
    bool modified = false;
    while (foldMachineFunction(MF))
      modified = true;
    return modified;
  }
};
}
char CheriAddressingModeFolder::ID;

MachineFunctionPass *llvm::createCheriAddressingModeFolder(void) {
  return new CheriAddressingModeFolder();
}
