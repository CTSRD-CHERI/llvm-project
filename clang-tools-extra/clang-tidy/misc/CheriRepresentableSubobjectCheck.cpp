//===--- CheriRepresentableSubobjectCheck.cpp - clang-tidy ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CheriRepresentableSubobjectCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecordLayout.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/CHERI/cheri-compressed-cap/cheri_compressed_cap.h"
#include "llvm/Support/Debug.h"

#include <limits>
#include <utility>

#define DEBUG_TYPE "cheri-representable-subobject-check"

using namespace clang::ast_matchers;

namespace {

template <typename CC>
std::pair<uint64_t, uint64_t>
getSubobjectRangeImpl(typename CC::addr_t ReqBase,
                      typename CC::addr_t ReqSize) {
  typename CC::offset_t MaxTop =
      static_cast<typename CC::offset_t>(
          std::numeric_limits<typename CC::addr_t>::max()) +
      1;
  typename CC::cap_t TmpCap = CC::make_max_perms_cap(0, 0, MaxTop);
  CC::setbounds(&TmpCap, ReqBase, ReqBase + ReqSize);

  return {TmpCap.base(), TmpCap.top64()};
}

std::pair<uint64_t, uint64_t> getSubobjectRange(const clang::TargetInfo &TI,
                                                uint64_t ReqBase,
                                                uint64_t ReqSize) {
  const uint64_t CapabilityWidth = TI.getCHERICapabilityWidth();

  if (CapabilityWidth == 128) {
    // XXX Need to support Morello as well
    return getSubobjectRangeImpl<CompressedCap128>(ReqBase, ReqSize);
  } else if (CapabilityWidth == 64) {
    return getSubobjectRangeImpl<CompressedCap64>(ReqBase, ReqSize);
  } else {
    // Unsupported / unexpected
    assert(false && "Should not be reached, unsupported capability width");
  }
}

} // namespace

namespace clang {
namespace tidy {
namespace misc {

bool CheriRepresentableSubobjectCheck::isLanguageVersionSupported(
    const LangOptions &LangOpts) const {
  return (LangOpts.getCheriBounds() > LangOptions::CBM_Conservative);
}

void CheriRepresentableSubobjectCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(recordDecl(isDefinition()).bind("r"), this);
}

void CheriRepresentableSubobjectCheck::check(
    const MatchFinder::MatchResult &Result) {
  const TargetInfo &TI = Result.Context->getTargetInfo();
  const auto *MatchedDecl = Result.Nodes.getNodeAs<RecordDecl>("r");
  if (!MatchedDecl->getIdentifier())
    return;
  if (MatchedDecl->isInvalidDecl())
    return;

  const ASTRecordLayout &Layout =
      Result.Context->getASTRecordLayout(MatchedDecl);
  // For each field, determine whether it is representable
  for (const FieldDecl *Field : MatchedDecl->fields()) {
    // Not sure whether we care about distinguishing bitfields
    uint64_t Offset = Layout.getFieldOffset(Field->getFieldIndex()) / 8;
    TypeInfo Info = Result.Context->getTypeInfo(Field->getType());
    uint64_t Size = Info.Width / 8;

    auto BaseAndTop = getSubobjectRange(TI, Offset, Size);

    if (BaseAndTop.first != Offset) {
      diag(Field->getLocation(),
           "Field %0 (%1) at %2 in %3 with size %4 may not be representable: "
           "offset will be imprecisely aligned to %5")
          << Field << Field->getType() << Offset << MatchedDecl << Size
          << BaseAndTop.first;
    }
    if (Offset + Size != BaseAndTop.second) {
      diag(Field->getLocation(),
           "Field %0 (%1) at %2 in %3 with size %4 may not be representable: "
           "top will be imprecisely aligned to %5")
          << Field << Field->getType() << Offset << MatchedDecl << Size
          << BaseAndTop.second;
    }
  }
}

} // namespace misc
} // namespace tidy
} // namespace clang
