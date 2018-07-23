//===--- RTEMS.cpp - RTEMS ToolChain Implementations ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RTEMS.h"
#include "CommonArgs.h"
#include "clang/Config/config.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/SanitizerArgs.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

void rtems::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                   const InputInfo &Output,
                                   const InputInfoList &Inputs,
                                   const ArgList &Args,
                                   const char *LinkingOutput) const {
  const toolchains::RTEMS &ToolChain =
      static_cast<const toolchains::RTEMS &>(getToolChain());
  const Driver &D = ToolChain.getDriver();

  ArgStringList CmdArgs;

  // Silence warning for "clang -g foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_g_Group);
  // and "clang -emit-llvm foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_emit_llvm);
  // and for "clang -w foo.o -o foo". Other warning options are already
  // handled somewhere else.
  Args.ClaimAllArgs(options::OPT_w);
  // TODO: warn if rtlib/c++lib is missing: err_drv_missing_argument

  const char *Exec = Args.MakeArgString(ToolChain.GetLinkerPath());
  if (llvm::sys::path::filename(Exec).equals_lower("ld.lld") ||
      llvm::sys::path::stem(Exec).equals_lower("ld.lld")) {
    CmdArgs.push_back("-z");
    CmdArgs.push_back("rodynamic");
  }

  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  if (Args.hasArg(options::OPT_rdynamic))
    CmdArgs.push_back("-export-dynamic");

  if (Args.hasArg(options::OPT_s))
    CmdArgs.push_back("-s");

  if (Args.hasArg(options::OPT_r)) {
    CmdArgs.push_back("-r");
  } else {
    CmdArgs.push_back("--build-id");
    /* LLD rejects gnu hash style for MIPS */
    //CmdArgs.push_back("--hash-style=gnu");
  }

  CmdArgs.push_back("--eh-frame-hdr");

  // No support for -shared or -pie linking
  if (Arg* A = Args.getLastArg(options::OPT_shared, options::OPT_pie)) {
      D.Diag(diag::err_drv_unsupported_opt) << A->getAsString(Args);
  }
  CmdArgs.push_back("-Bstatic");
  CmdArgs.push_back("-static");  // only find .a files

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
    const char* Crt0 = Args.hasArg(options::OPT_q_rtems) ? "start.o" : "crt0.o";
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath(Crt0)));
  }

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  Args.AddAllArgs(CmdArgs, options::OPT_u);

  ToolChain.AddFilePathLibArgs(Args, CmdArgs);

  addSanitizerRuntimes(ToolChain, Args, CmdArgs);

  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);


  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    // Add the rtems libns:
    if (Args.hasArg(options::OPT_q_rtems)) {
      CmdArgs.push_back("-lrtemsbsp");
      CmdArgs.push_back("-lrtemscpu");
    }
    if (D.CCCIsCXX()) {
      if (ToolChain.ShouldLinkCXXStdlib(Args))
        ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);
      CmdArgs.push_back("-lm");
    }

    AddRunTimeLibs(ToolChain, D, CmdArgs, Args);
    // TODO: is there pthread lib?
    if (Args.hasArg(options::OPT_pthread) ||
        Args.hasArg(options::OPT_pthreads))
      CmdArgs.push_back("-lpthread");
    // TODO: is this needed?
    if (Args.hasArg(options::OPT_fsplit_stack))
      CmdArgs.push_back("--wrap=pthread_create");

    CmdArgs.push_back("-lc");
  }
  CmdArgs.push_back("-e");
  CmdArgs.push_back("_start");

  C.addCommand(llvm::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}

/// RTEMS - RTEMS tool chain which can call as(1) and ld(1) directly.

RTEMS::RTEMS(const Driver &D, const llvm::Triple &Triple,
                 const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  getProgramPaths().push_back(getDriver().getInstalledDir());
  if (getDriver().getInstalledDir() != D.Dir)
    getProgramPaths().push_back(D.Dir);
#if 0
  SmallString<128> P(getTargetDir(D, getTriple()));
  llvm::sys::path::append(P, "lib");
  getFilePaths().push_back(P.str());
#endif

  if (!D.SysRoot.empty()) {
    SmallString<128> P(D.SysRoot);
    llvm::sys::path::append(P, "lib");
    getFilePaths().push_back(P.str());
  }
}

Tool *RTEMS::buildLinker() const {
  return new tools::rtems::Linker(*this);
}

ToolChain::RuntimeLibType RTEMS::GetRuntimeLibType(
    const ArgList &Args) const {
  if (Arg *A = Args.getLastArg(clang::driver::options::OPT_rtlib_EQ)) {
    StringRef Value = A->getValue();
    if (Value != "compiler-rt")
      getDriver().Diag(clang::diag::err_drv_invalid_rtlib_name)
          << A->getAsString(Args);
  }
  return ToolChain::RLT_CompilerRT;
}

std::string RTEMS::getCompilerRTPath() const {
  // Pick compiler rt from the sysroot first:
  const Driver &D = getDriver();
  if (!D.SysRoot.empty()) {
    llvm::SmallString<128> Path(D.SysRoot);
    llvm::sys::path::append(Path, "lib");
    return Path.str();
  }
  return ToolChain::getCompilerRTPath();
}

ToolChain::CXXStdlibType
RTEMS::GetCXXStdlibType(const ArgList &Args) const {
  if (Arg *A = Args.getLastArg(options::OPT_stdlib_EQ)) {
    StringRef Value = A->getValue();
    if (Value != "libc++")
      getDriver().Diag(diag::err_drv_invalid_stdlib_name)
        << A->getAsString(Args);
  }
  return ToolChain::CST_Libcxx;
}

void RTEMS::addClangTargetOptions(const ArgList &DriverArgs,
                                    ArgStringList &CC1Args,
                                    Action::OffloadKind) const {
  if (DriverArgs.hasFlag(options::OPT_fuse_init_array,
                         options::OPT_fno_use_init_array, true))
    CC1Args.push_back("-fuse-init-array");
}

void RTEMS::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args) const {

  const Driver &D = getDriver();

  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> P(D.ResourceDir);
    llvm::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  if (!D.SysRoot.empty()) {
    SmallString<128> P(D.SysRoot);
    llvm::sys::path::append(P, "include");
    addExternCSystemInclude(DriverArgs, CC1Args, P.str());
  }
}

void RTEMS::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                           ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdlibinc) ||
      DriverArgs.hasArg(options::OPT_nostdincxx))
    return;
  const Driver &D = getDriver();
  switch (GetCXXStdlibType(DriverArgs)) {
  case ToolChain::CST_Libcxx: {
    SmallString<128> P(!D.SysRoot.empty() ? D.SysRoot : "/");
    llvm::sys::path::append(P, "include", "c++", "v1");
    addSystemInclude(DriverArgs, CC1Args, P.str());
    break;
  }

  default:
    llvm_unreachable("invalid stdlib name");
  }
}

void RTEMS::AddCXXStdlibLibArgs(const ArgList &Args,
                                  ArgStringList &CmdArgs) const {
  switch (GetCXXStdlibType(Args)) {
  case ToolChain::CST_Libcxx:
    // TODO: what is correct here
    CmdArgs.push_back("-lc++");
    // CmdArgs.push_back("-lc++abi");
    CmdArgs.push_back("-lunwind");
    break;

  case ToolChain::CST_Libstdcxx:
    llvm_unreachable("invalid stdlib name");
  }
}
