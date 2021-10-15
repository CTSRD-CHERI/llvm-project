#include "llvm/Analysis/CheriBounds.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

// TODO: replace this with cheri-bounds
#define DEBUG_TYPE "cheri-bound-allocas"

using namespace llvm;
#define DBG_MESSAGE(...) LLVM_DEBUG(dbgs() << DEBUG_TYPE ": " << __VA_ARGS__)
#define DBG_INDENTED(...)                                                      \
  DBG_MESSAGE(llvm::right_justify("-", Depth + 1) << __VA_ARGS__)

// This is similar to CaptureTracking but we treat way more cases as "captured"
bool CheriNeedBoundsChecker::check(const Use &U) const {
  if (!MinSizeInBytes) {
    DBG_MESSAGE("dyanmic size alloca always needs bounds: ";
                U.getUser()->dump());
    return true;
  }
  APInt CurrentGEPOffset = APInt(DL.getIndexSizeInBits(PointerAS), 0);
  bool Result = useNeedsBounds(U, CurrentGEPOffset, 1);
  if (Result) {
    DBG_MESSAGE("Found alloca use that needs bounds: "; U.getUser()->dump());
  }
  return Result;
}

void CheriNeedBoundsChecker::findUsesThatNeedBounds(
    SmallVectorImpl<Use *> *UsesThatNeedBounds, bool BoundAllUses,
    bool *MustUseSingleIntrinsic) const {
  // TODO: don't replace load/store instructions with the bounded value
  for (Use &U : RootInst->uses()) {
    if (BoundAllUses || check(U)) {
      UsesThatNeedBounds->push_back(&U);
    }
  }
}

bool CheriNeedBoundsChecker::anyUseNeedsBounds() const {
  // TODO: don't replace load/store instructions with the bounded value
  for (const Use &U : RootInst->uses()) {
    if (check(U)) {
      return true;
    }
  }
  return false;
}

bool CheriNeedBoundsChecker::useNeedsBounds(const Use &U,
                                            const APInt &CurrentGEPOffset,
                                            unsigned Depth) const {
  const Instruction *I = cast<Instruction>(U.getUser());
  // Value *V = U->get();
  if (Depth > 10) {
    DBG_INDENTED("reached max depth, assuming bounds needed.\n");
    return true;
  }

  switch (I->getOpcode()) {
  case Instruction::Call:
  case Instruction::Invoke: {
    auto CI = cast<CallBase>(I);
    switch (CI->getIntrinsicID()) {
    case Intrinsic::not_intrinsic:
      // not an intrinsic call -> always need bounds:
      DBG_INDENTED("Adding stack bounds since it is passed to call: ";
                   I->dump());
      return true;
    case Intrinsic::lifetime_start:
    case Intrinsic::lifetime_end:
      DBG_INDENTED("No need for stack bounds for lifetime_{start,end}: ";
                   I->dump());
      return false;
    case Intrinsic::dbg_declare:
    case Intrinsic::dbg_value:
    case Intrinsic::dbg_label:
      DBG_INDENTED("No need for stack bounds for dbg_{declare,value,label}: ";
                   I->dump());
      return false;
    case Intrinsic::assume:
      DBG_INDENTED("No need for stack bounds for assume: ";
                   I->dump());
      return false;
    case Intrinsic::memset:
    case Intrinsic::memset_element_unordered_atomic:
    case Intrinsic::memcpy:
    case Intrinsic::memcpy_element_unordered_atomic:
    case Intrinsic::memmove:
    case Intrinsic::memmove_element_unordered_atomic:
      // TODO: an inline-expanded memset/memcpy/memmove doesn't need bounds!
      DBG_INDENTED("Adding stack bounds for memset/memcpy/memmove: ";
                   I->dump());
      return true;

    case Intrinsic::vastart:
    case Intrinsic::vacopy:
    case Intrinsic::vaend:
      // TODO: are bounds always needed?
      DBG_INDENTED("Adding stack bounds for va_start/va_copy/va_end: ";
                   I->dump());
      return true;

    case Intrinsic::cheri_bounded_stack_cap:
    case Intrinsic::cheri_cap_bounds_set:
    case Intrinsic::cheri_cap_bounds_set_exact: {
      auto SizeArg = I->getOperand(1);
      if (auto CI = dyn_cast<ConstantInt>(SizeArg)) {
        if (MinSizeInBytes) {
          APInt Zero(CurrentGEPOffset.getBitWidth(), 0);
          APInt Max(CurrentGEPOffset.getBitWidth(), *MinSizeInBytes);
          APInt LastAddr = CurrentGEPOffset + CI->getValue();
          if (CurrentGEPOffset.slt(Zero) || LastAddr.sgt(Max)) {
            DBG_INDENTED(I->getFunction()->getName()
                             << ": setbounds use offset OUT OF BOUNDS and will "
                                "trap -> adding csetbounds: ";
                         I->dump());
            return true;
          } else {
            // TODO: elide bounds if size <= less and all uses are in bounds
            DBG_INDENTED("No need for stack bounds for use in setbounds with "
                         "smaller or equal size: original size="
                             << MinSizeInBytes
                             << ", setbounds size=" << CI->getValue()
                             << " current offset=" << CurrentGEPOffset << ":";
                         I->dump());
            return false;
          }
        }
      }
      DBG_INDENTED("Adding stack bounds for unknown csetbounds: "; I->dump());
      return true;
    }
    case Intrinsic::cheri_cap_address_get:
    case Intrinsic::cheri_cap_address_set:
    case Intrinsic::cheri_cap_base_get:
    case Intrinsic::cheri_cap_build:
    case Intrinsic::cheri_cap_conditional_seal:
    case Intrinsic::cheri_cap_diff:
    case Intrinsic::cheri_cap_from_pointer:
    case Intrinsic::cheri_cap_length_get:
    case Intrinsic::cheri_cap_load_tags:
    case Intrinsic::cheri_cap_offset_get:
    case Intrinsic::cheri_cap_offset_set:
    case Intrinsic::cheri_cap_perms_and:
    case Intrinsic::cheri_cap_perms_check:
    case Intrinsic::cheri_cap_perms_get:
    case Intrinsic::cheri_cap_flags_set:
    case Intrinsic::cheri_cap_flags_get:
    case Intrinsic::cheri_cap_seal:
    case Intrinsic::cheri_cap_seal_entry:
    case Intrinsic::cheri_cap_sealed_get:
    case Intrinsic::cheri_cap_subset_test:
    case Intrinsic::cheri_cap_tag_clear:
    case Intrinsic::cheri_cap_tag_get:
    case Intrinsic::cheri_cap_to_pointer:
    case Intrinsic::cheri_cap_type_check:
    case Intrinsic::cheri_cap_type_copy:
    case Intrinsic::cheri_cap_type_get:
    case Intrinsic::cheri_cap_unseal:
      // CHERI intrinsics might depend on the bounds
      DBG_INDENTED("Adding stack bounds for alloca used in CHERI intrinsic:";
                   I->dump());
      return true;

    default:
      errs() << DEBUG_TYPE
             << ": Don't know how to handle intrinsic. Assuming bounds needed";
      I->print(errs(), true);
      DBG_INDENTED("Adding stack bounds for unknown intrinsic call: ";
                   I->dump());
      return true;
    }
  }
  case Instruction::Load:
    // Loading from a stack slot does not need bounds if the offset is known to
    // be within the alloca. We can just use cl* offset($csp) istead of adding
    // a cincoffset+csetbouds.
    return canLoadStoreBeOutOfBounds(I, U, CurrentGEPOffset, Depth);
  case Instruction::Store:
    // Storing to a stack slot does not need bounds if the offset is known to
    // be within the alloca. We can just use cs* offset($csp) istead of adding
    // a cincoffset+csetbouds.
    return canLoadStoreBeOutOfBounds(I, U, CurrentGEPOffset, Depth);

  case Instruction::AtomicRMW:
  case Instruction::AtomicCmpXchg:
    // cmpxchg and rmw are the same as load/store: if it is a fixed stack
    // slot we can omit the bounds
    return canLoadStoreBeOutOfBounds(I, U, CurrentGEPOffset, Depth);

  case Instruction::GetElementPtr: {
    auto GEPI = cast<GetElementPtrInst>(I);
    if (!GEPI->isInBounds() && !GEPI->hasAllConstantIndices()) {
      DBG_INDENTED("Adding stack bounds since GEP is not all constants: ";
                   I->dump());
      return true;
    }
    APInt NewGepOffset(CurrentGEPOffset);
    if (!GEPI->accumulateConstantOffset(DL, NewGepOffset)) {
      DBG_INDENTED("Could not accumulate constant GEP Offset -> need bounds: ";
                   I->dump());
      return true;
    }
    return anyUserNeedsBounds(GEPI, NewGepOffset, Depth);
  }
  case Instruction::PHI:
  case Instruction::Select:
  case Instruction::BitCast:
  case Instruction::AddrSpaceCast:
    // If any use of the cast/phi/selects needs bounds then we must add bounds
    // for the initial value.
    return anyUserNeedsBounds(I, CurrentGEPOffset, Depth);
  case Instruction::ICmp:
    // comparisons need bounds since cexeq takes bounds into account.
    // TODO: when only comparing addresses we might not need them
    DBG_INDENTED("Adding stack bounds since comparisons need bounds: ";
                 I->dump());
    return true;
  case Instruction::Ret:
    DBG_INDENTED("Adding stack bounds for alloca that is returned: ";
                 I->dump());
    return true;
  case Instruction::PtrToInt:
    DBG_INDENTED("No need for stack bounds for ptrtoint: "; I->dump());
    return false;
  default:
    // Something else - be conservative and say it needs bounds.
    errs() << "DON'T know how to handle ";
    I->print(errs(), true);
    DBG_INDENTED("Adding stack bounds since don't know how to handle: ";
                 I->dump());
    return true;
  }
}

bool CheriNeedBoundsChecker::anyUserNeedsBounds(const Instruction *I,
                                                const APInt &CurrentGEPOffset,
                                                unsigned Depth) const {
  DBG_INDENTED("Checking if " << I->getOpcodeName() << " needs stack bounds: ";
               I->dump());
  for (const Use &U : I->uses()) {
    if (useNeedsBounds(U, CurrentGEPOffset, Depth + 1)) {
      DBG_INDENTED("Adding stack bounds since " << I->getOpcodeName()
                                                << " user needs bounds: ";
                   U.getUser()->dump());
      return true;
    }
  }
  DBG_INDENTED("no " << I->getOpcodeName() << " users need bounds: ";
               I->dump());
  return false;
}

bool CheriNeedBoundsChecker::canLoadStoreBeOutOfBounds(
    const Instruction *I, const Use &U, const APInt &CurrentGEPOffset,
    unsigned Depth) const {
  DBG_INDENTED("Checking if load/store needs bounds (GEP offset is "
                   << CurrentGEPOffset << "): ";
               I->dump());
  Depth++; // indent the next debug message more
  assert(MinSizeInBytes && "dynamic size alloca should have been checked earlier");
  Type *LoadStoreType = nullptr;
  if (auto CmpXchg = dyn_cast<AtomicCmpXchgInst>(I)) {
    if (U.get() != CmpXchg->getPointerOperand()) {
      DBG_INDENTED(
          "Stack slot used as value and not pointer -> must set bounds\n";);
      return true;
    }
    LoadStoreType = CmpXchg->getNewValOperand()->getType();
  } else if (auto RMW = dyn_cast<AtomicRMWInst>(I)) {
    LoadStoreType = RMW->getValOperand()->getType();
    if (U.get() != RMW->getPointerOperand()) {
      DBG_INDENTED(
          "Stack slot used as value and not pointer -> must set bounds\n";);
      return true;
    }
  } else if (const auto *Store = dyn_cast<StoreInst>(I)) {
    if (U.get() != Store->getPointerOperand()) {
      DBG_INDENTED(
          "Stack slot used as value and not pointer -> must set bounds\n";);
      return true;
    }
    LoadStoreType = Store->getValueOperand()->getType();
  } else if (const auto *Load = dyn_cast<LoadInst>(I)) {
    assert(U.get() == Load->getPointerOperand() && "Invalid load?");
    LoadStoreType = Load->getType();
  } else {
    llvm_unreachable("Invalid load/store type");
  }
  auto Size = DL.getTypeStoreSize(LoadStoreType);
  DBG_INDENTED("Load/store size="
                   << Size << ", alloca size=" << MinSizeInBytes
                   << ", current GEP offset=" << CurrentGEPOffset << " for ";
               LoadStoreType->dump(););
  APInt Zero(CurrentGEPOffset.getBitWidth(), 0);
  APInt Max(CurrentGEPOffset.getBitWidth(), *MinSizeInBytes);
  APInt LastAddr = CurrentGEPOffset + Size;
  if (CurrentGEPOffset.slt(Zero) || LastAddr.sgt(Max)) {
    DBG_INDENTED(I->getFunction()->getName()
                     << ": stack load/store offset OUT OF BOUNDS -> adding "
                        "csetbounds: ";
                 I->dump());
    return true;
  }
  DBG_INDENTED("Load/store is in bounds -> can reuse $csp for "; I->dump(););
  return false;
}
