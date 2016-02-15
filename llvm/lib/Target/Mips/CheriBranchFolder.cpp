#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "Mips.h"
#include "MipsInstrInfo.h"

using namespace llvm;

static cl::opt<bool>
    DisableBranchFolder("disable-cheri-branch-folder", cl::init(false),
                        cl::desc("Allow inefficient capability tag checks"),
                        cl::Hidden);

namespace llvm {
extern const MCInstrDesc MipsInsts[];
}

namespace {
/// CheriBranchFolder - Turn sequences of CGetTag followed by a branch on the
/// result into CBTS / CBTU instructions.
struct CheriBranchFolder : public MachineFunctionPass {
  static char ID;
  typedef std::pair<MachineBasicBlock::iterator, unsigned> Replacement;
  CheriBranchFolder() : MachineFunctionPass(ID) {}
  virtual bool runOnMachineFunction(MachineFunction &MF) {
    if (DisableBranchFolder)
      return false;

    MachineRegisterInfo &RI = MF.getRegInfo();
    std::set<MachineInstr *> GetTags;
    SmallVector<Replacement, 16> Branches;
    bool modified = false;
    for (MachineFunction::iterator BI = MF.begin(), BE = MF.end(); BI != BE;
         ++BI)
      for (MachineBasicBlock::iterator I = BI->getFirstTerminator(),
                                       E = BI->end();
           I != E; ++I) {
        int Op = I->getOpcode();
        // Only look at basic blocks that end with a branch.
        if (!(Op == Mips::BEQ64 || Op == Mips::BNE64))
          continue;
        // And only if the branch if comparing with 0
        if (I->getOperand(1).getReg() != Mips::ZERO_64)
          continue;
        // Now see what the condition is.
        unsigned Cond = I->getOperand(0).getReg();
        // If the condition register has multiple uses, then don't generate the
        // CBT[SU] even if we can, because we'll still need to get the tag
        // value and we can slightly reduce the need for pipeline interlocks by
        // making sure that the value is in the main pipeline.
        if (!RI.hasOneUse(Cond))
          continue;
        MachineInstr *GetTag = RI.getUniqueVRegDef(Cond);
        // If there's no unique definition, or there is a unique definition but
        // it isn't a CGetTag instruction, then we can't fold the tag access
        // into the branch.
        if (!(GetTag && GetTag->getOpcode() == Mips::CGetTag))
          continue;
        Branches.push_back(std::make_pair(I, GetTag->getOperand(1).getReg()));
        GetTags.insert(GetTag);
        modified = true;
        // Once we've found one terminator that is a branch, skip any others.
        break;
      }
    for (auto &I : Branches) {
      MachineBasicBlock::iterator Branch = I.first;
      MachineBasicBlock *MBB = Branch->getParent();
      unsigned Reg = I.second;
      int Op = Branch->getOpcode();
      if (Op == Mips::BEQ64) {
        Op = Mips::CBTU;
      } else {
        Op = Mips::CBTS;
      }
      BuildMI(*MBB, Branch, Branch->getDebugLoc(), MipsInsts[Op])
          .addReg(Reg)
          .addOperand(Branch->getOperand(2))
          .addOperand(Branch->getOperand(3));
      Branch->eraseFromBundle();
    }
    for (auto I : GetTags) {
      if (!RI.use_empty(I->getOperand(0).getReg())) {
        continue;
      }
      I->eraseFromBundle();
    }
    return modified;
  }
};
}
char CheriBranchFolder::ID;

MachineFunctionPass *llvm::createCheriBranchFolder(void) {
  return new CheriBranchFolder();
}
