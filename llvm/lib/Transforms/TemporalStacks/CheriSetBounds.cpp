//===- CheriSetBounds.cpp - Functions to log information on CSetBounds ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This family of functions perform various local transformations to the
// program.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/DebugLoc.h>
#include <CheriSetBounds.h>
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/Local.h"

    std::string llvm::cheri::inferSourceLocation(const DebugLoc &DL,
                                                 StringRef FunctionName) {
  std::string Result;
  raw_string_ostream OS(Result);
  if (DL) {
    DL.print(OS);
    OS.flush();
  }
  if (Result.empty() && !FunctionName.empty()) {
    OS << "<somewhere in " << FunctionName << ">";
    OS.flush();
  }
  return Result;
}

std::string llvm::cheri::inferSourceLocation(Instruction *AI) {
  assert(AI);
  std::string Result = inferSourceLocation(AI->getDebugLoc(), StringRef());
  if (Result.empty()) {
    // some instructions such as alloca instruction don't have a debug loc
    // attached directly so we need to look for calls to llvm.dbg.declare()
    SmallVector<DbgInfoIntrinsic *, 2> DbgVars;
    findDbgUsers(DbgVars, AI);
    for (auto &DbgV : DbgVars) {
      Result = inferSourceLocation(DbgV->getDebugLoc(), StringRef());
      if (!Result.empty())
        break;
    }
  }
  // No debug instructions found -> just fall back to function name
  if (Result.empty()) {
    Result = ("<somewhere in " + AI->getFunction()->getName() + ">").str();
  }
  return Result;
}

std::string llvm::cheri::inferLocalVariableName(AllocaInst *AI) {
  assert(AI);
  // try to find a
  // some instructions such as alloca instruction don't have a debug loc
  // attached directly so we need to look for calls to llvm.dbg.declare()
  SmallVector<DbgInfoIntrinsic *, 2> DbgVars;
  findDbgUsers(DbgVars, AI);
  for (auto &DbgV : DbgVars) {
    if (DbgV->isAddressOfVariable()) {
      return ("local variable " + DbgV->getVariable()->getName()).str();
    }
  }
  // No debug instructions found -> just fall back to alloca name
  if (AI->hasName())
    return ("AllocaInst " + AI->getName()).str();
  std::string Result;
  llvm::raw_string_ostream OS(Result);
  OS << "anonymous AllocaInst of type ";
  AI->getType()->print(OS);
  OS.flush();
  return Result;
}