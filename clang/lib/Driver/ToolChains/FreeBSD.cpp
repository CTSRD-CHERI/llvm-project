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

  // When building 32-bit code on FreeBSD/amd64, we have to explicitly
  // instruct as in the base system to assemble 32-bit code.
  switch (getToolChain().getArch()) {
  default:
    break;
  case llvm::Triple::x86:
    CmdArgs.push_back("--32");
    break;
  case llvm::Triple::ppc:
    CmdArgs.push_back("-a32");
    break;
  case llvm::Triple::mips:
  case llvm::Triple::mipsel:
  case llvm::Triple::mips64:
  case llvm::Triple::mips64el:
  case llvm::Triple::cheri: {
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

    switch (getToolChain().getTriple().getEnvironment()) {
    case llvm::Triple::GNUEABIHF:
    case llvm::Triple::GNUEABI:
    case llvm::Triple::EABI:
      CmdArgs.push_back("-meabi=5");
      break;

    default:
      CmdArgs.push_back("-matpcs");
    }
    break;
  }
  case llvm::Triple::sparc:
  case llvm::Triple::sparcel:
  case llvm::Triple::sparcv9: {
    std::string CPU = getCPUName(Args, getToolChain().getTriple());
    CmdArgs.push_back(sparc::getSparcAsmModeForCPU(CPU, getToolChain().getTriple()));
    AddAssemblerKPIC(getToolChain(), Args, CmdArgs);
    break;
  }
  }

  Args.AddAllArgValues(CmdArgs, options::OPT_Wa_COMMA, options::OPT_Xassembler);

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  for (const auto &II : Inputs)
    CmdArgs.push_back(II.getFilename());

  const char *Exec = Args.MakeArgString(getToolChain().GetProgramPath("as"));
  C.addCommand(std::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
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
  const bool IsPIEDefault = ToolChain.isPIEDefault() || CheriAbiPIEDefault;
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
  Args.ClaimAllArgs(options::OPT_cheri_uintcap_offset);
  Args.ClaimAllArgs(options::OPT_cheri_uintcap_addr);
  // Also pretend that all the captable flags were used
  Args.ClaimAllArgs(options::OPT_cheri_cap_table_abi);
  Args.ClaimAllArgs(options::OPT_cheri_cap_table);
  Args.ClaimAllArgs(options::OPT_no_cheri_cap_table);
  Args.ClaimAllArgs(options::OPT_cheri_large_cap_table);
  Args.ClaimAllArgs(options::OPT_no_cheri_large_cap_table);
  Args.ClaimAllArgs(options::OPT_cheri_cap_tls_abi);
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
    } else {
      CmdArgs.push_back("-dynamic-linker");
      CmdArgs.push_back("/libexec/ld-elf.so.1");
    }
    if (ToolChain.getTriple().getOSMajorVersion() >= 9) {
      if (Arch == llvm::Triple::arm || Arch == llvm::Triple::sparc ||
          Arch == llvm::Triple::x86 || Arch == llvm::Triple::x86_64) {
        CmdArgs.push_back("--hash-style=both");
      }
    }
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
  case llvm::Triple::mips:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf32btsmip_fbsd");
    break;
  case llvm::Triple::mipsel:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf32ltsmip_fbsd");
    break;
  case llvm::Triple::mips64:
  case llvm::Triple::cheri:
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
    break;
  case llvm::Triple::riscv64:
    CmdArgs.push_back("-m");
    CmdArgs.push_back("elf64lriscv");
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

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
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
  Args.AddAllArgs(CmdArgs, options::OPT_e);
  Args.AddAllArgs(CmdArgs, options::OPT_s);
  Args.AddAllArgs(CmdArgs, options::OPT_t);
  Args.AddAllArgs(CmdArgs, options::OPT_Z_Flag);
  Args.AddAllArgs(CmdArgs, options::OPT_r);

  if (D.isUsingLTO()) {
    assert(!Inputs.empty() && "Must have at least one input.");
    AddGoldPlugin(ToolChain, Args, CmdArgs, Output, Inputs[0],
                  D.getLTOMode() == LTOK_Thin);
  }

  bool NeedsSanitizerDeps = addSanitizerRuntimes(ToolChain, Args, CmdArgs);
  bool NeedsXRayDeps = addXRayRuntime(ToolChain, Args, CmdArgs);
  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    // Use the static OpenMP runtime with -static-openmp
    bool StaticOpenMP = Args.hasArg(options::OPT_static_openmp) &&
                        !Args.hasArg(options::OPT_static);
    addOpenMPRuntime(CmdArgs, ToolChain, Args, StaticOpenMP);

    CmdArgs.push_back("--start-group");
    if (D.CCCIsCXX()) {
      if (ToolChain.ShouldLinkCXXStdlib(Args))
        ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);
      if (Args.hasArg(options::OPT_pg))
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
    if (Args.hasArg(options::OPT_pg))
      CmdArgs.push_back("-lgcc_p");
    else
      CmdArgs.push_back("-lgcc");
    if (Args.hasArg(options::OPT_static)) {
      CmdArgs.push_back("-lgcc_eh");
    } else if (Args.hasArg(options::OPT_pg)) {
      CmdArgs.push_back("-lgcc_eh_p");
    } else {
      CmdArgs.push_back("--as-needed");
      CmdArgs.push_back("-lgcc_s");
      CmdArgs.push_back("--no-as-needed");
    }

    if (Args.hasArg(options::OPT_pthread)) {
      if (Args.hasArg(options::OPT_pg))
        CmdArgs.push_back("-lpthread_p");
      else
        CmdArgs.push_back("-lpthread");
    }

    if (Args.hasArg(options::OPT_pg)) {
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
    } else if (Args.hasArg(options::OPT_pg)) {
      CmdArgs.push_back("-lgcc_eh_p");
    } else {
      CmdArgs.push_back("--as-needed");
      CmdArgs.push_back("-lgcc_s");
      CmdArgs.push_back("--no-as-needed");
    }
    CmdArgs.push_back("--end-group");
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
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
  C.addCommand(std::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
  if (Args.hasFlag(options::OPT_external_capsizefix, options::OPT_no_capsizefix,
                   false) &&
      Args.hasFlag(options::OPT_cheri_linker, options::OPT_no_cheri_linker,
                   true)) {
    Exec = Args.MakeArgString(getToolChain().GetProgramPath("capsizefix"));
    ArgStringList SizeFixArgs;
    if (Args.hasArg(options::OPT_verbose_capsizefix))
      SizeFixArgs.push_back("--verbose");
    SizeFixArgs.push_back(Output.getFilename());
    InputInfoList In= {InputInfo(types::TY_Object, Output.getFilename(), Output.getBaseInput())};
    C.addCommand(std::make_unique<Command>(JA, *this, Exec, SizeFixArgs, In));
  }
}

/// FreeBSD - FreeBSD tool chain which can call as(1) and ld(1) directly.

FreeBSD::FreeBSD(const Driver &D, const llvm::Triple &Triple,
                 const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {

  IsCheriPurecap =
      Triple.getEnvironment() == llvm::Triple::CheriPurecap ||
      (Triple.isMIPS() && tools::mips::hasMipsAbiArg(Args, "purecap"));

  // When targeting 32-bit platforms, look for '/usr/lib32/crt1.o' and fall
  // back to '/usr/lib' if it doesn't exist.
  if ((Triple.getArch() == llvm::Triple::x86 || Triple.isMIPS32() ||
       Triple.getArch() == llvm::Triple::ppc) &&
      D.getVFS().exists(getDriver().SysRoot + "/usr/lib32/crt1.o"))
    getFilePaths().push_back(getDriver().SysRoot + "/usr/lib32");
  else if (IsCheriPurecap &&
           D.getVFS().exists(getDriver().SysRoot + "/usr/libcheri/crt1.o"))
    getFilePaths().push_back(getDriver().SysRoot + "/usr/libcheri");
  else
    getFilePaths().push_back(getDriver().SysRoot + "/usr/lib");
}

ToolChain::CXXStdlibType FreeBSD::GetDefaultCXXStdlibType() const {
  // Always use libc++ for CHERI purecap
  // TODO: always use libc++ for MIPS64?
  if (getTriple().getOSMajorVersion() >= 10 || IsCheriPurecap)
    return ToolChain::CST_Libcxx;
  // Always use libc++ for CHERI triples and CHERI subarch:
  if (getTriple().getArch() == llvm::Triple::cheri)
    return ToolChain::CST_Libcxx;
  if (getTriple().isMIPS()) {
    switch (getTriple().getSubArch()) {
    case llvm::Triple::MipsSubArch_cheri64:
    case llvm::Triple::MipsSubArch_cheri128:
    case llvm::Triple::MipsSubArch_cheri256:
      return ToolChain::CST_Libcxx;
    default:
      break;
    }
  }
  return ToolChain::CST_Libstdcxx;
}

unsigned FreeBSD::GetDefaultDwarfVersion() const {
  if (getTriple().getOSMajorVersion() < 12)
    return 2;
  return 4;
}

void FreeBSD::addLibStdCxxIncludePaths(
    const llvm::opt::ArgList &DriverArgs,
    llvm::opt::ArgStringList &CC1Args) const {
  addLibStdCXXIncludePaths(getDriver().SysRoot, "/usr/include/c++/4.2", "", "",
                           "", "", DriverArgs, CC1Args);
}

void FreeBSD::AddCXXStdlibLibArgs(const ArgList &Args,
                                  ArgStringList &CmdArgs) const {
  CXXStdlibType Type = GetCXXStdlibType(Args);
  bool Profiling = Args.hasArg(options::OPT_pg);

  switch (Type) {
  case ToolChain::CST_Libcxx:
    CmdArgs.push_back(Profiling ? "-lc++_p" : "-lc++");
    break;

  case ToolChain::CST_Libstdcxx:
    CmdArgs.push_back(Profiling ? "-lstdc++_p" : "-lstdc++");
    break;
  }
}

void FreeBSD::AddCudaIncludeArgs(const ArgList &DriverArgs,
                                 ArgStringList &CC1Args) const {
  CudaInstallation.AddCudaIncludeArgs(DriverArgs, CC1Args);
}

Tool *FreeBSD::buildAssembler() const {
  return new tools::freebsd::Assembler(*this);
}

Tool *FreeBSD::buildLinker() const { return new tools::freebsd::Linker(*this); }

llvm::ExceptionHandling FreeBSD::GetExceptionModel(const ArgList &Args) const {
  // FreeBSD uses SjLj exceptions on ARM oabi.
  switch (getTriple().getEnvironment()) {
  case llvm::Triple::GNUEABIHF:
  case llvm::Triple::GNUEABI:
  case llvm::Triple::EABI:
    return llvm::ExceptionHandling::None;
  default:
    if (getTriple().getArch() == llvm::Triple::arm ||
        getTriple().getArch() == llvm::Triple::thumb)
      return llvm::ExceptionHandling::SjLj;
    return llvm::ExceptionHandling::None;
  }
}

bool FreeBSD::HasNativeLLVMSupport() const { return true; }

bool FreeBSD::IsUnwindTablesDefault(const ArgList &Args) const { return true; }

bool FreeBSD::isPIEDefault() const { return getSanitizerArgs().requiresPIE(); }

SanitizerMask FreeBSD::getSupportedSanitizers() const {
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
  if (IsX86_64 || IsMIPS64) {
    Res |= SanitizerKind::Leak;
    Res |= SanitizerKind::Thread;
  }
  if (IsX86 || IsX86_64) {
    Res |= SanitizerKind::Function;
    Res |= SanitizerKind::SafeStack;
    Res |= SanitizerKind::Fuzzer;
    Res |= SanitizerKind::FuzzerNoLink;
  } else if (IsMIPS64) {
    Res |= SanitizerKind::Fuzzer;
    Res |= SanitizerKind::FuzzerNoLink;
  }
  if (IsX86_64)
    Res |= SanitizerKind::Memory;
  return Res;
}
