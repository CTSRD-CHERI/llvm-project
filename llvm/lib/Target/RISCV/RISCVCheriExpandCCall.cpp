#include "RISCV.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "llvm/IR/Verifier.h"

#define DEBUG_TYPE "cheri-expand-ccall"
using namespace llvm;

namespace {

enum Interrupts { Disabled, Enabled, Inherit };

/**
 * Returns the interrupt status associated with the specified function.
 */
Interrupts getInterruptStatus(const Function &fn) {
  // If the interrupt posture attribute is not present then the function
  // inherits interrupt posture.
  if (fn.hasFnAttribute("interrupt-state")) {
    return StringSwitch<Interrupts>(
               fn.getFnAttribute("interrupt-state").getValueAsString())
        .Case("disabled", Disabled)
        .Case("enabled", Enabled)
        .Case("inherit", Inherit)
        .Default(Inherit);
  }
  return Inherit;
}

bool isSafeToDirectCall(const Function &from, const Function &to) {
  auto toStatus = getInterruptStatus(to);
  if (toStatus == Inherit)
    return true;
  auto fromStatus = getInterruptStatus(from);
  return fromStatus == toStatus;
}

/**
 * Perform pre-codegen expansion of CHERI cross-domain calls.  This inserts
 * the compartment import symbols and performs any lowering that's easier in
 * the IR.
 */
class RISCVCheriExpandCCall : public ModulePass,
                              public InstVisitor<RISCVCheriExpandCCall> {
  /// The module that's currently being inspected.
  Module *M;
  /// Have we made any changes to the module yet?
  bool Modified;
  /// Export table entries are opaque from the perspective of this pass, cache
  /// a type to use as a placeholder.
  IntegerType *I8Ty;
  /// Type to use for addresses.  We must be able to add 1 to this to set the
  /// low bit.
  IntegerType *AddrTy;
  /// The name used for the library pseudo-compartment
  static constexpr char const *LibraryCompartmentName = "libcalls";
  /// Set of calls that must be converted to indirect calls to correctly
  /// preserve interrupt status.
  SmallVector<CallBase *, 8> IndirectCalls;

public:
  /// Unique-address identifier for the pass.
  static char ID;

  /// Constructor.
  RISCVCheriExpandCCall() : ModulePass(ID) {
    initializeRISCVCheriExpandCCallPass(*PassRegistry::getPassRegistry());
  }

  /// Pass name, for debugging
  StringRef getPassName() const override {
    return "CHERI pre-codegen ccall expansion";
  }

  /// Insert an import table entry for calling the specified function.
  GlobalVariable *getOrInsertImportTableEntry(Function &F) {
    return getOrInsertImportTableEntry(getCalleeCompartmentName(F), F.getName(),
                                       F.getCallingConv(),
                                       F.hasExternalLinkage());
  }

  /// Insert
  GlobalVariable *getOrInsertInternalImportTableEntry(Function &F) {
    return getOrInsertImportTableEntry(
        getCalleeCompartmentName(F), F.getName(), CallingConv::CHERI_LibCall,
        false, (F.getCallingConv() == CallingConv::CHERI_CCallee));
  }

  /**
   * Insert an import table entry for calling the function called `FnName` in
   * compartment `Compartment`.  The calling convention is specified by `CC`.
   * The `IsExternal` parameter defines whether this is an externally visible
   * symbol.
   */
  GlobalVariable *getOrInsertImportTableEntry(StringRef Compartment,
                                              StringRef FnName, int CC,
                                              bool IsExternal,
                                              bool IsAlsoExported = false) {
    std::string ImportName =
        getImportExportTableName(Compartment, FnName, CC, /*isImport*/ true);
    // If we already have the import table entry, stop now
    if (auto *Import = M->getGlobalVariable(ImportName, !IsExternal))
      return Import;
    std::string ExportName = getImportExportTableName(
        Compartment, FnName, IsAlsoExported ? CallingConv::CHERI_CCall : CC,
        /*isImport*/ false);
    Constant *Export = M->getGlobalVariable(ExportName);
    if (Export == nullptr)
      Export = new GlobalVariable(*M, I8Ty, true, GlobalValue::ExternalLinkage,
                                  nullptr, ExportName, nullptr,
                                  GlobalValue::NotThreadLocal, 0);
    auto *ImportType =
        StructType::create({Export->getType(), Export->getType()});
    auto AS0PtrTy = I8Ty->getPointerTo(0);
    Export = ConstantExpr::getBitCast(Export, AS0PtrTy);
    // LibCalls set the low bit in the target address to indicate to the
    // loader that it should provide a capability that bypasses the
    // compartment switcher.  Calls that exist to toggle the interrupt status
    // are processed as library calls, irrespective of their internal calling
    // convention.
    if ((CC == CallingConv::CHERI_LibCall) || (CC < CallingConv::FirstTargetCC))
      Export = ConstantExpr::getGetElementPtr(
          I8Ty, Export, ConstantInt::get(AddrTy, 1), true);
    auto *ImportValue = ConstantStruct::get(
        ImportType, Export,
        ConstantPointerNull::get(cast<PointerType>(Export->getType())));
    auto *Import = new GlobalVariable(
        *M, ImportType, true,
        IsExternal ? GlobalValue::LinkOnceODRLinkage
                   : GlobalValue::InternalLinkage,
        ImportValue, ImportName, nullptr, GlobalValue::NotThreadLocal, 200);
    if (IsExternal)
      Import->setComdat(M->getOrInsertComdat(ImportName));
    Import->setSection(".compartment_imports");
    return Import;
  }

  /**
   * Returns the name of the compartment in which a function is defined.  For
   * normal compartments, this is stored in a function attribute.  Functions
   * that are not defined in a compartment are assumed to be in the library
   * pseudo-compartment.
   */
  StringRef getCallerCompartmentName(const Function &Fn) {
    if (Fn.hasFnAttribute("cheri-compartment"))
      return Fn.getFnAttribute("cheri-compartment").getValueAsString();
    return LibraryCompartmentName;
  }

  /**
   * Returns the name of the compartment for a called function.  Functions
   * using the library calling convention are assumed to all be in the same
   * pseudo-compartment.
   */
  StringRef getCalleeCompartmentName(const Function &Fn) {
    return (Fn.getCallingConv() == CallingConv::CHERI_LibCall)
               ? LibraryCompartmentName
               : Fn.getFnAttribute("cheri-compartment").getValueAsString();
  }

  /**
   * Visit a call or invoke instruction and, if it defines a cross-compartment
   * call, ensure that there's a corresponding import table.
   */
  void visitCallBase(CallBase &C) {
    // Make sure that we'll emit the compartment switcher symbol if there are
    // any indirect cross-compartment calls, even though we're not
    // transforming this call.
    if (C.isIndirectCall() &&
        (C.getCallingConv() == CallingConv::CHERI_CCall)) {
      Modified = true;
      return;
    }
    auto CC = C.getCallingConv();
    auto *F = C.getCalledFunction();
    if (!F)
      return;
    // Some intrinsics are expanded in the back end
    if (F->isIntrinsic()) {
      const char *FnName = nullptr;
      switch (F->getIntrinsicID()) {
      default:
        return;
      case Intrinsic::memmove:
        FnName = "_Z7memmovePvPKvj";
        break;
      case Intrinsic::memcpy:
        FnName = "_Z6memcpyPvPKvj";
        break;
      case Intrinsic::memset:
        FnName = "_Z6memsetPvij";
        break;
      }
      if (FnName) {
        getOrInsertImportTableEntry(LibraryCompartmentName, FnName,
                                    CallingConv::CHERI_LibCall, true);
        Modified = true;
      }
      return;
    }
    Function &Caller = *C.getParent()->getParent();
    if ((CC != CallingConv::CHERI_CCall) &&
        (CC != CallingConv::CHERI_LibCall)) {
      if (isSafeToDirectCall(Caller, *F))
        return;
      // If it is not safe to direct call, then we need to use an indirect
      // call.  Set the calling convention to libcall and insert the export
      // table entry.
      C.setCallingConv(CallingConv::CHERI_LibCall);
      getOrInsertInternalImportTableEntry(*F);
      IndirectCalls.push_back(&C);
      Modified = true;
      return;
    }
    // If we are calling a function from the same compartment as its
    // definition, we will do a direct call, so don't insert an import table
    // entry.  LibCalls may be across libraries so, conservatively, assume that
    // they might be in different libraries.
    if ((CC != CallingConv::CHERI_LibCall) &&
        (getCalleeCompartmentName(*F) == getCallerCompartmentName(Caller)))
      return;
    // If the function is defined in the same compilation unit as the caller, do
    // a direct call if safe to do so (interrupt status changes require an
    // indirect call).
    if (!F->isDeclaration() && isSafeToDirectCall(*F, Caller))
      return;
    getOrInsertImportTableEntry(*F);
    Modified = true;
  }

  /**
   * Process all cross-compartment calls, inserting any necessary import table
   * entry and changing the calling convention for exported functions to give
   * the correct lowering.  If there are any cross-compartment calls, insert
   * the compartment-switcher symbol for later use.
   */
  bool runOnModule(Module &Mod) override {
    M = &Mod;
    auto &Ctx = Mod.getContext();
    Modified = false;
    I8Ty = IntegerType::get(M->getContext(), 8);
    AddrTy = IntegerType::get(M->getContext(), 32);
    for (Function &F : Mod) {
      visit(F);
      auto ConvertToImportTableReference = [&](SmallVector<Use *> &Uses) {
        if (!Uses.empty()) {
          Constant *Import = getOrInsertImportTableEntry(F);
          for (auto *U : Uses) {
            if (auto *I = dyn_cast<Instruction>(U->getUser())) {
              Type *PtrTy = PointerType::get(Ctx, 200);
              if (Ctx.supportsTypedPointers())
                Import =
                    ConstantExpr::getBitCast(Import, PtrTy->getPointerTo(200));
              Value *FPtr = new LoadInst(PtrTy, Import, "import_table_load", I);
              if (Ctx.supportsTypedPointers())
                FPtr = new BitCastInst(FPtr, F.getType(), "fake_fptr", I);
              U->set(FPtr);
            }
          }
        }
      };
      // Functions that are exposed as callbacks have the `CHERI_CCall`
      // calling convention in the IR)
      if (F.getCallingConv() == CallingConv::CHERI_CCall) {
        // Reset the calling convention to ccallee so that it's lowered
        // correctly.
        F.setCallingConv(CallingConv::CHERI_CCallee);
        SmallVector<Use *> Uses;
        for (auto &U : F.uses()) {
          // If this is a function call and we're just calling the
          // function, we want to be using the function directly, so do
          // nothing.
          if (auto *CI = dyn_cast<CallBase>(U.getUser())) {
            if (CI->getCalledOperandUse() == U)
              continue;
          }
          Uses.push_back(&U);
        }
        ConvertToImportTableReference(Uses);
      } else if (F.getCallingConv() == CallingConv::CHERI_LibCall) {
        SmallVector<Use *> Uses;
        for (auto &U : F.uses()) {
          // If this is a function call and we're just calling the
          // function, we want to be using the function directly, so do
          // nothing.
          if (auto *CI = dyn_cast<CallBase>(U.getUser())) {
            if (CI->getCalledOperand() == &F)
              continue;
          }
          Uses.push_back(&U);
        }
        ConvertToImportTableReference(Uses);
      }
    }

    for (auto *Call : IndirectCalls) {
      auto &F = *Call->getCalledFunction();
      Constant *Import = getOrInsertInternalImportTableEntry(F);
      Type *PtrTy = PointerType::get(Ctx, 200);
      if (Ctx.supportsTypedPointers())
        Import = ConstantExpr::getBitCast(Import, PtrTy->getPointerTo(200));
      Value *FPtr = new LoadInst(PtrTy, Import, "import_table_load", Call);
      if (Ctx.supportsTypedPointers())
        FPtr = new BitCastInst(
            FPtr, PointerType::getWithSamePointeeType(F.getType(), 200),
            "fake_fptr", Call);
      Call->setCalledOperand(FPtr);
    }

    // If we've found a cross-domain call, make sure that there's a symbol for
    // the compartment switcher.
    if (Modified)
      new GlobalVariable(Mod, I8Ty->getPointerTo(200), true,
                         GlobalValue::ExternalLinkage, nullptr,
                         ".compartment_switcher", nullptr,
                         GlobalValue::NotThreadLocal, 0);

    return Modified;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }
};

} // anonymous namespace

char RISCVCheriExpandCCall::ID;
INITIALIZE_PASS(RISCVCheriExpandCCall, DEBUG_TYPE,
                "CHERI pre-codegen call expansion", false, false)

ModulePass *llvm::createRISCVCheriExpandCCallPass(void) {
  return new RISCVCheriExpandCCall();
}
