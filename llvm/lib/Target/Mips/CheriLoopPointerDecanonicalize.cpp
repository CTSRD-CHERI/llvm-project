#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Verifier.h"
#include "Mips.h"


using namespace llvm;

static cl::opt<bool> DisableLoopDecanonicalize(
  "disable-cheri-loop-decanonicalization", cl::init(false),
  cl::desc("Don't attempt to optimize pointer operations in loops"), cl::Hidden);


namespace {
class CheriLoopPointerDecanonicalize : public FunctionPass {
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool runOnFunction(Function &F) override {
    if (DisableLoopDecanonicalize)
      return false;
    bool Modified = false;
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    DenseMap<const SCEV*,Value*> SCEVs;
    SmallVector<std::tuple<GetElementPtrInst&, BasicBlock*, BasicBlock*,
      BasicBlock*, Value*, Value*>, 8> Replacements;
    for (auto &BB : F)
      for (auto &I : BB) {
        if (SE.isSCEVable(I.getType()))
          SCEVs[SE.getSCEV(&I)] = &I;
      }
    for (auto &BB : F) {
      Loop *L = LI.getLoopFor(&BB);
      // Skip basic blocks that are not inside a loop
      if (!L)
        continue;
      // If the loop doesn't have a preheader, give up because we'll want to
      // insert there.
      auto *Preheader = L->getLoopPreheader();
      if (!Preheader)
        continue;
      // Skip loops that have more than a single back edge for now.
      if (L->getNumBackEdges() != 1)
        continue;
      auto *Header = L->getHeader();
      auto PI = pred_begin(Header), PE = pred_end(Header);
      while ((PI != PE) && (*PI == Preheader))
        ++PI;
      assert(PI != PE);
      auto BackBB = *PI;


      for (auto &I : BB) {
        // Skip instructions that are not GEPs
        if (!isa<GetElementPtrInst>(I))
          continue;
        auto &GEP = cast<GetElementPtrInst>(I);
        // Skip GEPs where we can't trivially replace the induction variable
        // with the original.
        if (GEP.getType() != GEP.getOperand(0)->getType())
          continue;
        // If the GEP base does not dominate the end of the preheader then we
        // can't hoist the start out.
        if (auto *I = dyn_cast<Instruction>(GEP.getOperand(0)))
          if (!DT.dominates(I, Preheader->getTerminator()))
            continue;
        // Skip more complex GEPs for now - we mostly care about simple array
        // iteration anyway.
        if (GEP.getNumOperands() > 2)
          continue;
        // Skip non-capability pointers
        if (GEP.getPointerAddressSpace() != 200)
          continue;
        if (!DT.dominates(&GEP, BackBB))
          continue;
        auto *Idx = GEP.getOperand(1);
        auto *S = SE.getSCEV(Idx);
        if (isa<SCEVCastExpr>(S))
          S = cast<SCEVCastExpr>(S)->getOperand();
        if (auto *SA = dyn_cast<SCEVAddRecExpr>(S)) {
          auto *Start = SA->getStart();
          if (!Start)
            continue;
          auto *StartValue = SCEVs[Start];
          if (!StartValue)
            continue;
          if (auto *I = dyn_cast<Instruction>(StartValue))
            if (!DT.dominates(I, Preheader->getTerminator()))
              continue;
          auto Rec = SA->getStepRecurrence(SE);
          if (auto *Step = dyn_cast<SCEVUnknown>(Rec))
            Replacements.emplace_back(GEP, Header, Preheader, BackBB, Step->getValue(), StartValue);
          else if (auto Step = dyn_cast<SCEVConstant>(Rec))
            Replacements.emplace_back(GEP, Header, Preheader, BackBB, Step->getValue(), StartValue);
        }
      }
    }
    for (auto R : Replacements) {
      auto &GEP = std::get<0>(R); 
      auto *Header = std::get<1>(R);
      auto *Preheader = std::get<2>(R);
      auto *BackBB = std::get<3>(R);
      auto *StepValue = std::get<4>(R);
      auto *StartValue = std::get<5>(R);
      auto *PHI = PHINode::Create(GEP.getType(), 2,
        "decanonicalized_ptr", &*Header->begin());
      auto *NewGEP =
        GetElementPtrInst::Create(GEP.getResultElementType(), PHI, {
            StepValue }, "decanonicalized", &GEP);
      PHI->addIncoming(NewGEP, BackBB);
      auto *StartGEP =
        GetElementPtrInst::Create(GEP.getResultElementType(), GEP.getOperand(0), {
            StartValue }, "decanonicalized_start", &*(--(Preheader->end())));
      PHI->addIncoming(StartGEP, Preheader);
      GEP.replaceAllUsesWith(PHI);
      Modified = true;
    }
    return Modified;
  }
  public:
  static char ID;
  CheriLoopPointerDecanonicalize() : FunctionPass(ID) {}
  StringRef getPassName() const override {
    return "CheriLoopPointerDecanonicalize";
  }
};
}

namespace llvm {
    void initializeCheriLoopPointerDecanonicalizePass(PassRegistry&);
}

char CheriLoopPointerDecanonicalize::ID;
INITIALIZE_PASS_BEGIN(CheriLoopPointerDecanonicalize, "cheriloopdecanonicalize",
                      "CHERI loop decanonicalize", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(CheriLoopPointerDecanonicalize, "cheriloopdecanonicalize",
                    "CHERI loop decanonicalize", false, false)

FunctionPass *
llvm::createCheriLoopPointerDecanonicalize() {
    return new CheriLoopPointerDecanonicalize();
}
