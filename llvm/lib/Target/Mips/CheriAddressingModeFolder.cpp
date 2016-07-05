#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "Mips.h"
#include "MipsInstrInfo.h"
#include "MipsTargetMachine.h"

using namespace llvm;

static cl::opt<bool> DisableAddressingModeFolder(
    "disable-cheri-addressing-mode-folder", cl::init(false),
    cl::desc("Allow redundant capability manipulations"), cl::Hidden);

namespace {
struct CheriAddressingModeFolder : public MachineFunctionPass {
  static char ID;
  const MipsInstrInfo *InstrInfo;

  CheriAddressingModeFolder(MipsTargetMachine &TM) : MachineFunctionPass(ID) {
    InstrInfo = TM.getSubtargetImpl()->getInstrInfo();
  }
  bool tryToFoldAdd(unsigned vreg, MachineRegisterInfo &RI,
                    MachineInstr *&AddInst, int64_t &offset) {
    AddInst = RI.getUniqueVRegDef(vreg);
    // If we can't uniquely identify the definition, give up.
    if (AddInst == 0)
      return false;
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
    // FIXME: Offset for clc / csc is bigger...
    if (off < -127 || off > 127) {
      return false;
    }
    offset = off;
    return true;
  }
  unsigned MipsOpForCHERIOp(unsigned Op) {
    switch (Op) {
      default:
        llvm_unreachable("No MIPS equivalent");
      case Mips::CAPLOAD8: return Mips::LB64;
      case Mips::CAPLOADU8: return Mips::LBu64;
      case Mips::CAPLOAD16: return Mips::LH64;
      case Mips::CAPLOADU16: return Mips::LHu64;
      case Mips::CAPLOAD32: return Mips::LW;
      case Mips::CAPLOADU32: return Mips::LWu;
      case Mips::CAPLOAD832: return Mips::LB;
      case Mips::CAPLOADU832: return Mips::LBu;
      case Mips::CAPLOAD1632: return Mips::LH;
      case Mips::CAPLOADU1632: return Mips::LHu;
      case Mips::CAPLOAD3264: return Mips::LW64;
      case Mips::CAPLOAD64: return Mips::LD;
      case Mips::CAPSTORE8: return Mips::SB64;
      case Mips::CAPSTORE16: return Mips::SH64;
      case Mips::CAPSTORE32: return Mips::SW64;
      case Mips::CAPSTORE64: return Mips::SD;
    }
  }
  template <typename T>
  void Remove(T &Instrs, MachineRegisterInfo &RI) {
    for (auto *I : Instrs)
      if (RI.use_empty(I->getOperand(0).getReg()))
        I->eraseFromBundle();
  }

  bool foldMachineFunction(MachineFunction &MF) {
    if (DisableAddressingModeFolder)
      return false;

    MachineRegisterInfo &RI = MF.getRegInfo();
    std::set<MachineInstr *> IncOffsets;
    std::set<MachineInstr *> Adds;
    llvm::SmallVector<std::pair<MachineInstr *, MachineInstr *>, 8> C0Ops;
    std::set<MachineInstr *> GetPCCs;
    bool modified = false;
    for (auto &MBB : MF)
      for (MachineInstr &I : MBB) {
        int Op = I.getOpcode();
        if (Op == Mips::CGetPCC) {
          GetPCCs.insert(&I);
          continue;
        }
        // Only look at cap-relative loads and stores
        if (!(Op == Mips::CAPLOAD8 || Op == Mips::CAPLOADU8 ||
              Op == Mips::CAPLOAD16 || Op == Mips::CAPLOADU16 ||
              Op == Mips::CAPLOAD32 || Op == Mips::CAPLOADU32 ||
              Op == Mips::CAPLOAD832 || Op == Mips::CAPLOADU832 ||
              Op == Mips::CAPLOAD1632 || Op == Mips::CAPLOADU1632 ||
              Op == Mips::CAPLOAD3264 || Op == Mips::CAPLOAD64 ||
              Op == Mips::CAPSTORE8 || Op == Mips::CAPSTORE16 ||
              Op == Mips::LOADCAP || Op == Mips::STORECAP ||
              Op == Mips::CAPSTORE32 || Op == Mips::CAPSTORE64))
          continue;
        int64_t offset = I.getOperand(2).getImm();
        // If the load is not currently at register-zero offset, we can't fix
        // it up to use relative addressing, but we may be able to modify it so
        // that it is...
        if (I.getOperand(1).getReg() != Mips::ZERO_64) {
          MachineInstr *AddInst;
          // If the register offset is a simple constant, then try to move it
          // into the memory operation
          if (tryToFoldAdd(I.getOperand(1).getReg(), RI, AddInst, offset)) {
            Adds.insert(AddInst);
            I.getOperand(2).setImm(offset);
            I.getOperand(1).setReg(Mips::ZERO_64);
          } else
            continue;
        }
        // If we get to here, then the memory operation has a capability and
        // (possibly) an immediate offset, but the register offset is $zero.
        // If the capability is formed by incrementing an offset, then try to
        // pull that calculation into the memory operation.

        MachineInstr *IncOffset = RI.getUniqueVRegDef(I.getOperand(3).getReg());
        if (!IncOffset)
          continue;
        // If this is CFromPtr-relative load or store, then we may be able to
        // fold it into a MIPS load.
        if (IncOffset->getOpcode() == Mips::CFromPtr) {
          if ((Op == Mips::LOADCAP) || (Op == Mips::STORECAP))
            continue;
          if (IncOffset->getOperand(1).getReg() == Mips::C0)
            C0Ops.emplace_back(&I, IncOffset);
          continue;
        }
        // Ignore ones that are not based on a CIncOffset op
        if (IncOffset->getOpcode() != Mips::CIncOffset)
          continue;
        assert(IncOffset);

        MachineOperand Cap = IncOffset->getOperand(1);
        MachineOperand Offset = IncOffset->getOperand(2);
        assert(Cap.isReg());
        MachineInstr *AddInst;
        // If the CIncOffset is of a daddi[u] then we can potentially replace
        // both by just folding the register and immediate offsets into the
        // load / store.
        if (tryToFoldAdd(Offset.getReg(), RI, AddInst, offset)) {
          // If we managed to pull the offset calculation entirely away, then
          // just use the computed immediate
          Adds.insert(AddInst);
          I.getOperand(1).setReg(Mips::ZERO_64);
          I.getOperand(2).setImm(offset);
        } else
          // If we didn't, then use the CIncOffset's register value as our
          // offset
          I.getOperand(1).setReg(Offset.getReg());
        I.getOperand(3).setReg(Cap.getReg());
        IncOffsets.insert(IncOffset);
        modified = true;
      }
    for (auto &I : C0Ops) {
      IncOffsets.insert(I.second);
      unsigned BaseReg = I.second->getOperand(2).getReg();
      // This can't be a symbolic address (yet) because we don't have
      // relocations that fit.
      MachineOperand &Offset = I.first->getOperand(2);
      // If this was the result of a daddiu, fold the immediate into the result
      // as well.
      MachineInstr *AddInst = RI.getUniqueVRegDef(BaseReg);
      if (AddInst && (AddInst->getOpcode() == Mips::DADDiu)) {
        MachineOperand &MO = AddInst->getOperand(2);
        // FIXME: We could probably fold the add into the load in some cases
        // even when it's not initially zero, as our immediate field has just
        // grown, but that's probably not the common case.
        if (Offset.isImm() && (Offset.getImm() == 0)) {
          Offset = MO;
          BaseReg = AddInst->getOperand(1).getReg();
        }
      } else
        AddInst = nullptr;
      BuildMI(*I.first->getParent(), I.first, I.first->getDebugLoc(),
          InstrInfo->get(MipsOpForCHERIOp(I.first->getOpcode())),
          I.first->getOperand(0).getReg())
        .addReg(BaseReg).addOperand(Offset);
      I.first->eraseFromBundle();
      if (AddInst)
        Adds.insert(AddInst);
      modified = true;
    }
    Remove(GetPCCs, RI);
    Remove(IncOffsets, RI);
    Remove(Adds, RI);
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

MachineFunctionPass *
llvm::createCheriAddressingModeFolder(MipsTargetMachine &TM) {
  return new CheriAddressingModeFolder(TM);
}
