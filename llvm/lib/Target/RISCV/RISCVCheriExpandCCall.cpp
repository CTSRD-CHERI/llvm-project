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

class RISCVCheriExpandCCall : public ModulePass,
                              public InstVisitor<RISCVCheriExpandCCall> {
  Module *M;
  bool Modified;
  IntegerType *I8Ty;

public:
  static char ID;
  RISCVCheriExpandCCall() : ModulePass(ID) {
    initializeRISCVCheriExpandCCallPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "CHERI pre-codegen ccall expansion";
  }

  void visitCallBase(CallBase &C) {
    if (C.isIndirectCall())
      return;
    if (C.getCallingConv() != CallingConv::CHERI_CCall)
      return;
    auto *F = C.getCalledFunction();
    StringRef Compartment =
        F->getFnAttribute("cheri-compartment").getValueAsString();
    std::string ImportName =
        (Twine("__import_") + Compartment + "_" + F->getName()).str();
    // If we already have the import table entry, stop now
    if (M->getGlobalVariable(ImportName))
      return;
    std::string ExportName =
        (Twine("__export_") + Compartment + "_" + F->getName()).str();
    auto *Export = new GlobalVariable(
        *M, I8Ty, true, GlobalValue::ExternalLinkage, nullptr, ExportName,
        nullptr, GlobalValue::NotThreadLocal, 0);
    auto *ImportType =
        StructType::create({Export->getType(), Export->getType()});
    auto *ImportValue = ConstantStruct::get(
        ImportType, Export, ConstantPointerNull::get(Export->getType()));
    auto *Import = new GlobalVariable(
        *M, ImportType, true, GlobalValue::LinkOnceODRLinkage, ImportValue,
        ImportName, nullptr, GlobalValue::NotThreadLocal, 200);
    Import->setComdat(M->getOrInsertComdat(ImportName));
    Import->setSection(".compartment_imports");
    Modified = true;
  }

  bool runOnModule(Module &Mod) override {
    M = &Mod;
    Modified = false;
    I8Ty = IntegerType::get(M->getContext(), 8);
    for (Function &F : Mod)
      visit(F);

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
