#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "Mips.h"
#include "MipsInstrInfo.h"
#include "MipsTargetMachine.h"

using namespace llvm;

static cl::opt<bool> EnableCheri128FailHard(
    "enable-cheri128-fail-hard", cl::init(false),
    cl::desc("Trap if CHERI128 operations lose their tags"), cl::Hidden);

namespace {
struct Cher128FailHardPass : public MachineFunctionPass {
  static char ID;
  const MipsInstrInfo *InstrInfo = nullptr;

  Cher128FailHardPass() : MachineFunctionPass(ID) {
  }
  Cher128FailHardPass(MipsTargetMachine &TM) : MachineFunctionPass(ID) {
    InstrInfo = TM.getSubtargetImpl()->getInstrInfo();
  }

  StringRef getPassName() const override { return "Cheri128 fail hard"; }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (!EnableCheri128FailHard)
      return false;
    InstrInfo = MF.getSubtarget<MipsSubtarget>().getInstrInfo();
    SmallVector<MachineInstr*, 16> instrs;
    std::set<MachineBasicBlock*> bbs;
    for (auto &MBB : MF)
      for (auto &MI : MBB)
        if ((MI.getOpcode() == Mips::CIncOffset) ||
            (MI.getOpcode() == Mips::CSetOffset)) {
          instrs.push_back(&MI);
          bbs.insert(&MBB);
        }
    if (instrs.empty())
      return false;
    MachineRegisterInfo &RI = MF.getRegInfo();
    for (auto *MI : instrs) {
      unsigned tag_before = RI.createVirtualRegister(&Mips::GPR64RegClass);
      unsigned tag_after = RI.createVirtualRegister(&Mips::GPR64RegClass);
      auto &MBB = *MI->getParent();
      BuildMI(MBB, MI,
        MI->getDebugLoc(), InstrInfo->get(Mips::CGetTag))
          .addDef(tag_before).addUse(MI->getOperand(1).getReg());
      MachineBasicBlock::iterator after = 
          BuildMI(*MI->getParent(), ++MachineBasicBlock::iterator(MI),
          MI->getDebugLoc(), InstrInfo->get(Mips::CGetTag))
        .addDef(tag_after).addUse(MI->getOperand(0).getReg()).getInstr();
      auto Sink = MF.CreateMachineBasicBlock(MBB.getBasicBlock());
      MF.insert(++MachineFunction::iterator(MBB), Sink);
      // Transfer the remainder of BB and its successor edges to Sink.
      Sink->splice(Sink->begin(), &MBB, std::next(after),
                   MBB.end());
      Sink->transferSuccessorsAndUpdatePHIs(&MBB);
      MBB.addSuccessor(Sink);
      assert(Sink->getParent() == &MF);
      after = BuildMI(MBB, ++after,
          MI->getDebugLoc(), InstrInfo->get(Mips::BEQ64))
        .addUse(tag_after, RegState::Kill)
        .addUse(tag_before, RegState::Kill)
        .addMBB(Sink);
      BuildMI(MBB, ++after, MI->getDebugLoc(), InstrInfo->get(Mips::TRAP));
    }
    return true;
  }
};
}
namespace llvm {
  void initializeCher128FailHardPassPass(PassRegistry&);
}


char Cher128FailHardPass::ID;
INITIALIZE_PASS_BEGIN(Cher128FailHardPass, "cheri128failhard",
                    "CHERI128 abort on tag loss ", false, false)
INITIALIZE_PASS_END(Cher128FailHardPass, "cheri128failhard",
                    "CHERI abort on tag loss", false, false)



MachineFunctionPass *
llvm::createCheri128FailHardPass() {
  return new Cher128FailHardPass();
}
