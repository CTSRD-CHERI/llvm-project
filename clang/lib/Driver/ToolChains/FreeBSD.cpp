//===--- FreeBSD.cpp - FreeBSD ToolChain Implementations --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FreeBSD.h"
#include "Arch/ARM.h"
#include "Arch/Mips.h"
#include "Arch/Sparc.h"
#include "CommonArgs.h"
#include "clang/Config/config.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/SanitizerArgs.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

void freebsd::Assembler::ConstructJob(Compilation &C, const JobAction &JA,
                                      const InputInfo &Output,
                                      const InputInfoList &Inputs,
                                      const ArgList &Args,
                                      const char *LinkingOutput) const {
  claimNoWarnArgs(Args);
  ArgStringList CmdArgs;
  const auto &D = getToolChain().getDriver();

  // When building 32-bit code on FreeBSD/amd64, we have to explicitly
  // instruct as in the base system to assemble 32-bit code.
  switch (getToolChain().getArch()) {
  default:
    break;
  case llvm::Triple::x86:
    CmdArgs.push_back("--32");
    break;
  case llvm::Triple::ppc:
  case llvm::Triple::ppcle:
    CmdArgs.push_back("-a32");
    break;
  case llvm::Triple::mips:
  case llvm::Triple::mipsel:
  case llvm::Triple::mips64:
  case llvm::Triple::mips64el: {
    StringRef CPUName;
    StringRef ABIName;
    mips::getMipsCPUAndABI(Args, getToolChain().getTriple(), CPUName, ABIName);

    CmdArgs.push_back("-march");
    CmdArgs.push_back(CPUName.data());

    CmdArgs.push_back("-mabi");
    CmdArgs.push_back(mips::getGnuCompatibleMipsABIName(ABIName).data());

    if (getToolChain().getTriple().isLittleEndian())
      CmdArgs.push_back("-EL");
    else
      CmdArgs.push_back("-EB");

    if (Arg *A = Args.getLastArg(options::OPT_G)) {
      StringRef v = A->getValue();
      CmdArgs.push_back(Args.MakeArgString("-G" + v));
      A->claim();
    }

    AddAssemblerKPIC(getToolChain(), Args, CmdArgs);
    break;
  }
  case llvm::Triple::arm:
  case llvm::Triple::armeb:
  case llvm::Triple::thumb:
  case llvm::Triple::thumbeb: {
    arm::FloatABI ABI = arm::getARMFloatABI(getToolChain(), Args);

    if (ABI == arm::FloatABI::Hard)
      CmdArgs.push_back("-mfpu=vfp");
    else
      CmdArgs.push_back("-mfpu=softvfp");

    CmdArgs.push_back("-meabi=5");
    break;
  }
  case llvm::Triple::sparc:
  case llvm::Triple::sparcel:
  case llvm::Triple::sparcv9: {
    std::string CPU = getCPUName(D, Args, getToolChain().getTriple());
    CmdArgs.push_back(
        sparc::getSparcAsmModeForCPU(CPU, getToolChain().getTriple()));
    AddAssemblerKPIC(getToolChain(), Args, CmdArgs);
    break;
  }
  }

  for (const Arg *A : Args.filtered(options::OPT_ffile_prefix_map_EQ,
                                    options::OPT_fdebug_prefix_map_EQ)) {
    StringRef Map = A->getValue();
    if (!Map.contains('='))
      D.Diag(diag::err_drv_invalid_argument_to_option)
          << Map << A->getOption().getName();
    else {
      CmdArgs.push_back(Args.MakeArgString("--debug-prefix-map"));
      CmdArgs.push_back(Args.MakeArgString(Map));
    }
    A->claim();
  }

  Args.AddAllArgValues(CmdArgs, options::OPT_Wa_COMMA, options::OPT_Xassembler);

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  for (const auto &II : Inputs)
    CmdArgs.push_back(II.getFilename());

  const char *Exec = Args.MakeArgString(getToolChain().GetProgramPath("as"));
  C.addCommand(std::make_unique<Command>(JA, *this,
                                         ResponseFileSupport::AtFileCurCP(),
                                         Exec, CmdArgs, Inputs, Output));
}

void freebsd::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                   const InputInfo &Output,
                                   const InputInfoList &Inputs,
                                   const ArgList &Args,
                                   const char *LinkingOutput) const {
  const toolchains::FreeBSD &ToolChain =
      static_cast<const toolchains::FreeBSD &>(getToolChain());
  const Driver &D = ToolChain.getDriver();
  const llvm::Triple::ArchType Arch = ToolChain.getArch();
  bool IsCHERIPureCapABI = ToolChain.isCheriPurecap();
  // For CheriABI default to -pie unless -static is also passed
  // TODO: enable static PIE?
  const bool CheriAbiPIEDefault =
      IsCHERIPureCapABI && !Args.hasArg(options::OPT_static);
  const bool IsPIEDefault = ToolChain.isPIEDefault(Args) || CheriAbiPIEDefault;
  // We can't pass -pie to the linker if any of -shared,-r,-no-pie,-no-pie are
  // set
  Arg *ConflictsWithPie = Args.getLastArg(options::OPT_r, options::OPT_shared);
  // Have to negate here to handle the no-pie and nopie aliases
  Arg *LastPIEArg = Args.getLastArg(options::OPT_pie, options::OPT_no_pie,
                                    options::OPT_nopie);
  const bool ExplicitPIE =
      LastPIEArg && LastPIEArg->getOption().matches(options::OPT_pie);
  if (ExplicitPIE && ConflictsWithPie) {
    getToolChain().getDriver().Diag(diag::err_drv_argument_not_allowed_with)
        << LastPIEArg->getAsString(Args) << ConflictsWithPie->getAsString(Args);
  }
  const bool IsPIE =
      (LastPIEArg ? ExplicitPIE : IsPIEDefault) && !ConflictsWithPie;

  ArgStringList CmdArgs;

  // Silence warning for -cheri=NNN
  Args.ClaimAllArgs(options::OPT_cheri_EQ);
  Args.ClaimAllArgs(options::OPT_cheri);
  // And -cheri-uintcap=
  Args.ClaimAllArgs(options::OPT_cheri_uintcap_EQ);
  // Also pretend that all the captable flags were used
  Args.ClaimAllArgs(options::OPT_cheri_cap_table_abi);
  Args.ClaimAllArgs(options::OPT_cheri_large_cap_table);
  Args.ClaimAllArgs(options::OPT_no_cheri_large_cap_table);
  // Various other CHERI flags can also be passed to the linker without warning:
  Args.ClaimAllArgs(options::OPT_cheri_data_dependent_provenance);
  Args.ClaimAllArgs(options::OPT_cheri_bounds_EQ);

  // Silence warning for "clang -g foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_g_Group);
  // and "clang -emit-llvm foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_emit_llvm);
  // and for "clang -w foo.o -o foo". Other warning options are already
  // handled somewhere else.
  Args.ClaimAllArgs(options::OPT_w);

  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  if (IsPIE)
    CmdArgs.push_back("-pie");

  CmdArgs.push_back("--eh-frame-hdr");
  if (Args.hasArg(options::OPT_static)) {
    CmdArgs.push_back("-Bstatic");
  } else {
    if (Args.hasArg(options::OPT_rdynamic))
      CmdArgs.push_back("-export-dynamic");
    if (Args.hasArg(options::OPT_shared)) {
      CmdArgs.push_back("-Bshareable");
    } else if (!Args.hasArg(options::OPT_r)) {
      CmdArgs.push_back("-dynamic-linker");
      CmdArgs.push_back("/libexec/ld-elf.so.1");
    }
    const llvm::Triple &T = ToolChain.getTriple();
    if (Arch == llvm::Triple::arm || Arch == llvm::Triple::sparc || T.isX86())
      CmdArgs.push_back("--hash-style=both");
    CmdArgs.push_back("--enable-new-dtags");
  }

  // Explicitly set the linker emulation for platforms that might not
  // be the default emulation for the linker.
  switch (Arch) {
  case llvm::Triple::x86:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf_i386_fbsd");
    break;
  case llvm::Triple::ppc:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf32ppc_fbsd");
    break;
  case llvm::Triple::ppcle:
    CmdArgs.push_back("-m");
    // Use generic -- only usage is for freestanding.
    CmdArgs.push_back("elf32lppc");
    break;
  case llvm::Triple::mips:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf32btsmip_fbsd");
    break;
  case llvm::Triple::mipsel:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf32ltsmip_fbsd");
    break;
  case llvm::Triple::mips64:
    CmdArgs.push_back("-m");
    if (IsCHERIPureCapABI)
      CmdArgs.push_back("elf64btsmip_cheri_fbsd");
    else if (tools::mips::hasMipsAbiArg(Args, "n32"))
      CmdArgs.push_back("elf32btsmipn32_fbsd");
    else
      CmdArgs.push_back("elf64btsmip_fbsd");
    break;
  case llvm::Triple::mips64el:
    CmdArgs.push_back("-m");
    if (tools::mips::hasMipsAbiArg(Args, "n32"))
      CmdArgs.push_back("elf32ltsmipn32_fbsd");
    else
      CmdArgs.push_back("elf64ltsmip_fbsd");
    break;
  case llvm::Triple::riscv32:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf32lriscv");
    CmdArgs.push_back("-X");
    break;
  case llvm::Triple::riscv64:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf64lriscv");
    CmdArgs.push_back("-X");
    break;
  default:
    break;
  }

  if (Arg *A = Args.getLastArg(options::OPT_G)) {
    if (ToolChain.getTriple().isMIPS()) {
      StringRef v = A->getValue();
      CmdArgs.push_back(Args.MakeArgString("-G" + v));
      A->claim();
    }
  }

  // The FreeBSD/MIPS version of GNU ld is horribly buggy and errors out
  // complaining about linking 32-bit and 64-bit code when linking CHERI code.
  if (IsCHERIPureCapABI)
    CmdArgs.push_back(Args.MakeArgString("--no-warn-mismatch"));

  if (Output.isFilename()) {
    CmdArgs.push_back("-o");
    CmdArgs.push_back(Output.getFilename());
  } else {
    assert(Output.isNothing() && "Invalid output.");
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles,
                   options::OPT_r)) {
    const char *crt1 = nullptr;
    if (!Args.hasArg(options::OPT_shared)) {
      if (Args.hasArg(options::OPT_pg))
        crt1 = "gcrt1.o";
      else if (IsPIE)
        crt1 = "Scrt1.o";
      else
        crt1 = "crt1.o";
    }
    if (crt1)
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath(crt1)));

    // Don't support .init and .fini sections for CheriABI.
    if (!IsCHERIPureCapABI)
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crti.o")));

    const char *crtbegin = nullptr;
    if (Args.hasArg(options::OPT_shared) || IsPIE)
      crtbegin = "crtbeginS.o";
    else if (Args.hasArg(options::OPT_static))
      crtbegin = "crtbeginT.o";
    else
      crtbegin = "crtbegin.o";

    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath(crtbegin)));
  }

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  ToolChain.AddFilePathLibArgs(Args, CmdArgs);
  Args.AddAllArgs(CmdArgs, options::OPT_T_Group);
  Args.AddAllArgs(CmdArgs, options::OPT_s);
  Args.AddAllArgs(CmdArgs, options::OPT_t);
  Args.AddAllArgs(CmdArgs, options::OPT_Z_Flag);
  Args.AddAllArgs(CmdArgs, options::OPT_r);

  if (D.isUsingLTO()) {
    assert(!Inputs.empty() && "Must have at least one input.");
    addLTOOptions(ToolChain, Args, CmdArgs, Output, Inputs[0],
                  D.getLTOMode() == LTOK_Thin);
  }

  bool NeedsSanitizerDeps = addSanitizerRuntimes(ToolChain, Args, CmdArgs);
  bool NeedsXRayDeps = addXRayRuntime(ToolChain, Args, CmdArgs);
  addLinkerCompressDebugSectionsOption(ToolChain, Args, CmdArgs);
  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  unsigned Major = ToolChain.getTriple().getOSMajorVersion();
  bool Profiling = Args.hasArg(options::OPT_pg) && Major != 0 && Major < 14;
  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs,
                   options::OPT_r)) {
    // Use the static OpenMP runtime with -static-openmp
    bool StaticOpenMP = Args.hasArg(options::OPT_static_openmp) &&
                        !Args.hasArg(options::OPT_static);
    addOpenMPRuntime(CmdArgs, ToolChain, Args, StaticOpenMP);

    CmdArgs.push_back("--start-group");
    if (D.CCCIsCXX()) {
      if (ToolChain.ShouldLinkCXXStdlib(Args))
        ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);
      if (Profiling)
        CmdArgs.push_back("-lm_p");
      else
        CmdArgs.push_back("-lm");
    }
    if (NeedsSanitizerDeps)
      linkSanitizerRuntimeDeps(ToolChain, CmdArgs);
    if (NeedsXRayDeps)
      linkXRayRuntimeDeps(ToolChain, CmdArgs);
    // FIXME: For some reason GCC passes -lgcc and -lgcc_s before adding
    // the default system libraries. Just mimic this for now.
    if (Profiling)
      CmdArgs.push_back("-lgcc_p");
    else
      CmdArgs.push_back("-lgcc");
    if (Args.hasArg(options::OPT_static)) {
      CmdArgs.push_back("-lgcc_eh");
    } else if (Profiling) {
      CmdArgs.push_back("-lgcc_eh_p");
    } else {
      CmdArgs.push_back("--as-needed");
      CmdArgs.push_back("-lgcc_s");
      CmdArgs.push_back("--no-as-needed");
    }

    if (Args.hasArg(options::OPT_pthread)) {
      if (Profiling)
        CmdArgs.push_back("-lpthread_p");
      else
        CmdArgs.push_back("-lpthread");
    }

    if (Profiling) {
      if (Args.hasArg(options::OPT_shared))
        CmdArgs.push_back("-lc");
      else
        CmdArgs.push_back("-lc_p");
      CmdArgs.push_back("-lgcc_p");
    } else {
      CmdArgs.push_back("-lc");
      CmdArgs.push_back("-lgcc");
    }

    if (Args.hasArg(options::OPT_static)) {
      CmdArgs.push_back("-lgcc_eh");
    } else if (Profiling) {
      CmdArgs.push_back("-lgcc_eh_p");
    } else {
      CmdArgs.push_back("--as-needed");
      CmdArgs.push_back("-lgcc_s");
      CmdArgs.push_back("--no-as-needed");
    }
    CmdArgs.push_back("--end-group");
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles,
                   options::OPT_r)) {
    if (Args.hasArg(options::OPT_shared) || IsPIE)
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtendS.o")));
    else
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtend.o")));

    // Don't support .init and .fini sections for CheriABI.
    if (!IsCHERIPureCapABI)
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtn.o")));
  }

  ToolChain.addProfileRTLibs(Args, CmdArgs);

  const char *Exec = Args.MakeArgString(getToolChain().GetLinkerPath());
  C.addCommand(std::make_unique<Command>(JA, *this,
                                         ResponseFileSupport::AtFileCurCP(),
                                         Exec, CmdArgs, Inputs, Output));
}

/// FreeBSD - FreeBSD tool chain which can call as(1) and ld(1) directly.

FreeBSD::FreeBSD(const Driver &D, const llvm::Triple &Triple,
                 const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {

  // When targeting 32-bit platforms, look for '/usr/lib32(c)/crt1.o' and fall
  // back to '/usr/lib' if it doesn't exist. We also do the same for 64-bit
  // targets with '/usr/lib64(c)'.
  std::string CompatLib = (Twine("lib") + (Triple.isArch32Bit() ? "32" : "64") +
                           (IsCheriPurecap ? "c" : "")).str();
  if (D.getVFS().exists(concat(getDriver().SysRoot, "/usr/" + CompatLib + "/crt1.o")))
    getFilePaths().push_back(concat(getDriver().SysRoot, "/usr/" + CompatLib));
  else
    getFilePaths().push_back(concat(getDriver().SysRoot, "/usr/lib"));
}

unsigned FreeBSD::GetDefaultDwarfVersion() const {
  unsigned Major = getTriple().getOSMajorVersion();
  if (Major >= 12 || Major == 0)
    return 4;
  return 2;
}

void FreeBSD::AddClangSystemIncludeArgs(
    const llvm::opt::ArgList &DriverArgs,
    llvm::opt::ArgStringList &CC1Args) const {
  const Driver &D = getDriver();

  if (DriverArgs.hasArg(clang::driver::options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(D.ResourceDir);
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // Check for configure-time C include directories.
  StringRef CIncludeDirs(C_INCLUDE_DIRS);
  if (CIncludeDirs != "") {
    SmallVector<StringRef, 5> dirs;
    CIncludeDirs.split(dirs, ":");
    for (StringRef dir : dirs) {
      StringRef Prefix =
          llvm::sys::path::is_absolute(dir) ? StringRef(D.SysRoot) : "";
      addExternCSystemInclude(DriverArgs, CC1Args, Prefix + dir);
    }
    return;
  }

  addExternCSystemInclude(DriverArgs, CC1Args,
                          concat(D.SysRoot, "/usr/include"));
}

void FreeBSD::addLibCxxIncludePaths(const llvm::opt::ArgList &DriverArgs,
                                    llvm::opt::ArgStringList &CC1Args) const {
  addSystemInclude(DriverArgs, CC1Args,
                   concat(getDriver().SysRoot, "/usr/include/c++/v1"));
}

void FreeBSD::AddCXXStdlibLibArgs(const ArgList &Args,
                                  ArgStringList &CmdArgs) const {
  Generic_ELF::AddCXXStdlibLibArgs(Args, CmdArgs);
  unsigned Major = getTriple().getOSMajorVersion();
  bool SuffixedLib = Args.hasArg(options::OPT_pg) && Major != 0 && Major < 14;
  if (SuffixedLib && GetCXXStdlibType(Args) == CST_Libcxx)
    std::replace_if(
        CmdArgs.begin(), CmdArgs.end(),
        [](const char *S) { return StringRef(S) == "-lc++"; }, "-lc++_p");
}

void FreeBSD::AddCudaIncludeArgs(const ArgList &DriverArgs,
                                 ArgStringList &CC1Args) const {
  CudaInstallation->AddCudaIncludeArgs(DriverArgs, CC1Args);
}

void FreeBSD::AddHIPIncludeArgs(const ArgList &DriverArgs,
                                ArgStringList &CC1Args) const {
  RocmInstallation->AddHIPIncludeArgs(DriverArgs, CC1Args);
}

Tool *FreeBSD::buildAssembler() const {
  return new tools::freebsd::Assembler(*this);
}

Tool *FreeBSD::buildLinker() const { return new tools::freebsd::Linker(*this); }

bool FreeBSD::HasNativeLLVMSupport() const { return true; }

ToolChain::UnwindTableLevel
FreeBSD::getDefaultUnwindTableLevel(const ArgList &Args) const {
  return UnwindTableLevel::Asynchronous;
}

bool FreeBSD::isPIEDefault(const llvm::opt::ArgList &Args) const {
  return getSanitizerArgs(Args).requiresPIE();
}

SanitizerMask FreeBSD::getSupportedSanitizers() const {
  const bool IsAArch64 = getTriple().getArch() == llvm::Triple::aarch64;
  const bool IsX86 = getTriple().getArch() == llvm::Triple::x86;
  const bool IsX86_64 = getTriple().getArch() == llvm::Triple::x86_64;
  const bool IsMIPS64 = getTriple().isMIPS64();
  SanitizerMask Res = ToolChain::getSupportedSanitizers();
  if (getTriple().getEnvironment() != llvm::Triple::CheriPurecap) {
    // ASAN currently crashes when enabled for purecap
    Res |= SanitizerKind::Address;
  }
  Res |= SanitizerKind::PointerCompare;
  Res |= SanitizerKind::PointerSubtract;
  Res |= SanitizerKind::Vptr;
  if (IsAArch64 || IsX86_64 || IsMIPS64) {
    Res |= SanitizerKind::Leak;
    Res |= SanitizerKind::Thread;
  }
  if (IsAArch64 || IsX86 || IsX86_64) {
    Res |= SanitizerKind::SafeStack;
    Res |= SanitizerKind::Fuzzer;
    Res |= SanitizerKind::FuzzerNoLink;
  } else if (IsMIPS64) {
    Res |= SanitizerKind::Fuzzer;
    Res |= SanitizerKind::FuzzerNoLink;
  }
  if (IsAArch64 || IsX86_64) {
    Res |= SanitizerKind::KernelAddress;
    Res |= SanitizerKind::KernelMemory;
    Res |= SanitizerKind::Memory;
  }
  return Res;
}

void FreeBSD::addClangTargetOptions(const ArgList &DriverArgs,
                                    ArgStringList &CC1Args,
                                    Action::OffloadKind) const {
  unsigned Major = getTriple().getOSMajorVersion();
  if (!DriverArgs.hasFlag(options::OPT_fuse_init_array,
                          options::OPT_fno_use_init_array,
                          (Major >= 12 || Major == 0 || getTriple().isMIPS() ||
                           isCheriPurecap())))
    CC1Args.push_back("-fno-use-init-array");
}
