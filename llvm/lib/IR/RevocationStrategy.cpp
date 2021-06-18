#include "llvm/IR/RevocationStrategy.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"

#define METADATA_NAME "revocation-strategy"

#define CONDITIONAL_REVOKE "conditional-revoke"
#define UNCONDITIONAL_REVOKE "unconditional-revoke"
#define NO_REVOKE "no-revoke"

namespace llvm {
namespace cheri {

namespace {

RevocationStrategy decodeRevocationStrategy(StringRef Encoded) {
  return StringSwitch<RevocationStrategy>(Encoded)
      .Case(CONDITIONAL_REVOKE, RevocationStrategy::ConditionalRevocation)
      .Case(UNCONDITIONAL_REVOKE, RevocationStrategy::UnconditionalRevocation)
      .Case(NO_REVOKE, RevocationStrategy::NoRevocation)
      .Default(RevocationStrategy::NoRevocation);
}

StringRef encodeRevocationStrategy(RevocationStrategy Strategy) {
  switch (Strategy) {
  case RevocationStrategy::ConditionalRevocation:
    return CONDITIONAL_REVOKE;
  case RevocationStrategy::UnconditionalRevocation:
    return UNCONDITIONAL_REVOKE;
  case RevocationStrategy::NoRevocation:
    return NO_REVOKE;
  }
}

} // anonymous namespace

void setFunctionRevocationStrategy(Function &F, RevocationStrategy Strategy) {
  LLVMContext &Context = F.getContext();
  StringRef Encoded = encodeRevocationStrategy(Strategy);
  MDString *AsMDString = MDString::get(Context, Encoded);
  MDNode *Node = MDNode::get(Context, AsMDString);
  F.setMetadata(METADATA_NAME, Node);
}

RevocationStrategy getFunctionRevocationStrategy(const Function &F) {
  if (MDNode *Node = F.getMetadata(METADATA_NAME)) {
    if (MDString *NodeString = dyn_cast<MDString>(Node->getOperand(0))) {
      return decodeRevocationStrategy(NodeString->getString());
    }
  }
  llvm_unreachable("Called getFunctionRevocationStrategy() on a function with "
                   "no revocation strategy metadata");
}

} // namespace cheri
} // namespace llvm
