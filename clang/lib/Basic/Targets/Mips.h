//===--- Mips.h - Declare Mips target feature support -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares Mips TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_MIPS_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_MIPS_H

#include "llvm/Config/config.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Cheri.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY MipsTargetInfo : public TargetInfo {
  void setDataLayout() {
    StringRef Layout;
    // XXXAR: why do we need this here? can't we use the LLVM one?
    if (ABI == "o32") {
      Layout = "m:m-p:32:32-i8:8:32-i16:16:32-i64:64-n32-S64";
      if (IsCHERI)
        llvm::report_fatal_error("Cannot use CHERI with " + ABI + " ABI");
    } else if (ABI == "n32") {
      Layout = "m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32:64-S128";
      if (IsCHERI)
        llvm::report_fatal_error("Cannot use CHERI with " + ABI + " ABI");
    } else if (ABI == "n64") {
      if (IsCHERI) {
        if (CapSize == 64) {
          Layout = "m:e-pf200:64:64:64:32-i8:8:32-i16:16:32-i64:64-n32:64-S128";
        } else if (CapSize == 128) {
          Layout = "m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128";
        } else {
          assert(CapSize == 256);
          Layout = "m:e-pf200:256:256:256:64-i8:8:32-i16:16:32-i64:64-n32:64-S256";
        }
      } else
        Layout = "m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128";
    } else
      llvm_unreachable("Invalid ABI");

    StringRef PurecapOptions = "";
    // Only set globals address space to 200 for cap-table mode
    if (CapabilityABI) {
        PurecapOptions = "-A200-P200-G200";
    }

    if (BigEndian)
      resetDataLayout(
          ("E-" + Layout + (CapabilityABI ? PurecapOptions : "")).str());
    else
      resetDataLayout(
          ("e-" + Layout + (CapabilityABI ? PurecapOptions : "")).str());

    if (IsCHERI) {
      assert(DataLayout->getStackAlignment() >= ((unsigned)CapSize / 8));
    }
  }

  static const Builtin::Info BuiltinInfo[];
  std::string CPU;
  bool IsMips16;
  bool IsMicromips;
  bool IsNan2008;
  bool IsAbs2008;
  bool IsSingleFloat;
  bool IsNoABICalls;
  bool CanUseBSDABICalls;
  enum MipsFloatABI { HardFloat, SoftFloat } FloatABI;
  enum DspRevEnum { NoDSP, DSP1, DSP2 } DspRev;
  bool HasMSA;
  bool DisableMadd4;
  bool UseIndirectJumpHazard;

protected:
  enum FPModeEnum { FPXX, FP32, FP64 } FPMode;
  std::string ABI;
  bool IsCHERI = false;
  int CapSize = -1;

public:
  MipsTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : TargetInfo(Triple), IsMips16(false), IsMicromips(false),
        IsNan2008(false), IsAbs2008(false), IsSingleFloat(false),
        IsNoABICalls(false), CanUseBSDABICalls(false), FloatABI(HardFloat),
        DspRev(NoDSP), HasMSA(false), DisableMadd4(false),
        UseIndirectJumpHazard(false), FPMode(FPXX) {
    TheCXXABI.set(TargetCXXABI::GenericMIPS);

    if (Opts.ABI == "purecap") {
      assert(Triple.getEnvironment() == llvm::Triple::CheriPurecap);
    }

    if (Triple.getEnvironment() == llvm::Triple::CheriPurecap)
      setABI("purecap");
    else if (Triple.isMIPS32())
      setABI("o32");
    else if (Triple.getEnvironment() == llvm::Triple::GNUABIN32)
      setABI("n32");
    else
      setABI("n64");

    CPU = ABI == "o32" ? "mips32r2" : "mips64r2";
    // If we have a CHERI triple, or an explicit CHERI128 CPU, then assume
    // CHERI128.
    CapSize = llvm::StringSwitch<int>(Opts.CPU)
      .Cases("cheri", "cheri128", 128) // If we have a CHERI CPU, default to assuming CHERI128.
      .Case("cheri256", 256)
      .Case("cheri64", 64)
      .Default(-1);
    const int TripleCapSize = [](llvm::Triple::SubArchType SubArch) {
      switch (SubArch) {
      case llvm::Triple::MipsSubArch_cheri64:
        return 64;
      case llvm::Triple::MipsSubArch_cheri128:
        return 128;
      case llvm::Triple::MipsSubArch_cheri256:
        return 256;
      default:
        return -1;
      }
    }(getTriple().getSubArch());
    if (CapSize == -1) {
      CapSize = TripleCapSize;
    } else if (TripleCapSize != -1 && CapSize != TripleCapSize) {
      llvm::report_fatal_error("CPU flag " + Opts.CPU +
                               " is incompatible with triple " +
                               getTriple().str());
    }
    if (CapSize > 0 ||
        getTriple().getEnvironment() == llvm::Triple::CheriPurecap) {
      IsCHERI = true;
    }
    if (IsCHERI) {
      switch (CapSize) {
        default:
        case 128: CPU = "cheri128"; break;
        case 64: CPU = "cheri64"; break;
        case 256: CPU = "cheri256"; break;
      }
      if (CapSize > 0) {
        SuitableAlign = CapSize;
        DefaultAlignForAttributeAligned =
            std::max(DefaultAlignForAttributeAligned, (unsigned short)CapSize);
      }
      if (Triple.isMIPS32())
        llvm::report_fatal_error("Cannot use CHERI with MIPS32 triple");
    }
    CanUseBSDABICalls = Triple.isOSFreeBSD() ||
                        Triple.isOSOpenBSD();
  }

  bool isIEEE754_2008Default() const {
    return CPU == "mips32r6" || CPU == "mips64r6";
  }

  bool isFP64Default() const {
    // XXXAR: CHERI?
    return CPU == "mips32r6" || ABI == "n32" || ABI == "n64" || ABI == "64";
  }

  bool isNan2008() const override { return IsNan2008; }

  bool processorSupportsGPR64() const;

  StringRef getABI() const override { return ABI; }

  bool setABI(const std::string &Name) override {
    if (Name == "o32") {
      setO32ABITypes();
      ABI = Name;
      return true;
    }

    if (Name == "n32") {
      setN32ABITypes();
      ABI = Name;
      return true;
    }
    if (Name == "n64") {
      setN64ABITypes();
      ABI = Name;
      return true;
    }
    if (Name == "purecap") {
      setCapabilityABITypes();
      CapabilityABI = true;
      ABI = "n64";
      return true;
    }
    return false;
  }

  void setO32ABITypes() {
    Int64Type = SignedLongLong;
    IntMaxType = Int64Type;
    LongDoubleFormat = &llvm::APFloat::IEEEdouble();
    LongDoubleWidth = LongDoubleAlign = 64;
    LongWidth = LongAlign = 32;
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 32;
    PointerWidth = PointerAlign = 32;
    PtrDiffType = SignedInt;
    SizeType = UnsignedInt;
    SuitableAlign = 64;
  }

  void setN32N64ABITypes() {
    LongDoubleWidth = LongDoubleAlign = 128;
    LongDoubleFormat = &llvm::APFloat::IEEEquad();
    if (getTriple().isOSFreeBSD()) {
      LongDoubleWidth = LongDoubleAlign = 64;
      LongDoubleFormat = &llvm::APFloat::IEEEdouble();
    }
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    if (IsCHERI && CapSize > 0)
      MaxAtomicInlineWidth = std::max(MaxAtomicInlineWidth, (unsigned short)CapSize);
    SuitableAlign = std::max(CapSize, 128);
  }

  void setN64ABITypes() {
    setN32N64ABITypes();
    if (getTriple().isOSOpenBSD()) {
      Int64Type = SignedLongLong;
    } else {
      Int64Type = SignedLong;
    }
    IntMaxType = Int64Type;
    LongWidth = LongAlign = 64;
    PointerWidth = PointerAlign = 64;
    PtrDiffType = SignedLong;
    SizeType = UnsignedLong;
  }

  void setCapabilityABITypes() {
    setN64ABITypes();
    IntPtrType = TargetInfo::SignedIntCap;
  }

  void setN32ABITypes() {
    setN32N64ABITypes();
    Int64Type = SignedLongLong;
    IntMaxType = Int64Type;
    LongWidth = LongAlign = 32;
    PointerWidth = PointerAlign = 32;
    PtrDiffType = SignedInt;
    SizeType = UnsignedInt;
  }

  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    CPU = Name;
    return isValidCPUName(Name);
  }

  const std::string &getCPU() const { return CPU; }
  bool
  initFeatureMap(llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override {
    if (CPU.empty())
      CPU = getCPU();

    assert(CPU != "cheri");
    // cheri as a CPU type is now an alias for cheri128, so we should never see
    // a raw cheri here.
    if (CPU == "octeon")
      Features["mips64r2"] = Features["cnmips"] = true;
    else if (CPU == "octeon+")
      Features["mips64r2"] = Features["cnmips"] = Features["cnmipsp"] = true;
    else
      Features[CPU] = true;

    if (IsCHERI)
      Features["chericap"] = true;
    return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
  }

  unsigned getISARev() const;

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  bool hasFeature(StringRef Feature) const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        // CPU register names
        // Must match second column of GCCRegAliases
        "$0", "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10",
        "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", "$20",
        "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29", "$30",
        "$31",
        // Floating point register names
        "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "$f8", "$f9",
        "$f10", "$f11", "$f12", "$f13", "$f14", "$f15", "$f16", "$f17", "$f18",
        "$f19", "$f20", "$f21", "$f22", "$f23", "$f24", "$f25", "$f26", "$f27",
        "$f28", "$f29", "$f30", "$f31",
        // Hi/lo and condition register names
        "hi", "lo", "", "$fcc0", "$fcc1", "$fcc2", "$fcc3", "$fcc4", "$fcc5",
        "$fcc6", "$fcc7", "$ac1hi", "$ac1lo", "$ac2hi", "$ac2lo", "$ac3hi",
        "$ac3lo",
        // MSA register names
        "$w0", "$w1", "$w2", "$w3", "$w4", "$w5", "$w6", "$w7", "$w8", "$w9",
        "$w10", "$w11", "$w12", "$w13", "$w14", "$w15", "$w16", "$w17", "$w18",
        "$w19", "$w20", "$w21", "$w22", "$w23", "$w24", "$w25", "$w26", "$w27",
        "$w28", "$w29", "$w30", "$w31",
        // MSA control register names
        "$msair", "$msacsr", "$msaaccess", "$msasave", "$msamodify",
        "$msarequest", "$msamap", "$msaunmap",
        // CHERI register names
        "$cnull", "$c1", "$c2", "$c3", "$c4", "$c5", "$c6", "$c7", "$c8", "$c9",
        "$c10", "$c11", "$c12", "$c13", "$c14", "$c15", "$c16", "$c17", "$c18",
        "$c19", "$c20", "$c21", "$c22", "$c23", "$c24", "$c25", "$c26", "$c27",
        "$c28", "$c29", "$c30", "$c31", "$idc", "$kr1c", "$kr2c", "$kcc", "$kdc",
        "$epcc", "$ddc"
    };
    return llvm::makeArrayRef(GCCRegNames);
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      return false;
    case 'r': // CPU registers.
    case 'd': // Equivalent to "r" unless generating MIPS16 code.
    case 'y': // Equivalent to "r", backward compatibility only.
    case 'f': // floating-point registers.
    case 'c': // $25 for indirect jumps
    case 'l': // lo register
    case 'x': // hilo register pair
      Info.setAllowsRegister();
      return true;
    case 'I': // Signed 16-bit constant
    case 'J': // Integer 0
    case 'K': // Unsigned 16-bit constant
    case 'L': // Signed 32-bit constant, lower 16-bit zeros (for lui)
    case 'M': // Constants not loadable via lui, addiu, or ori
    case 'N': // Constant -1 to -65535
    case 'O': // A signed 15-bit constant
    case 'P': // A constant between 1 go 65535
      return true;
    case 'R': // An address that can be used in a non-macro load or store
      Info.setAllowsMemory();
      return true;
    case 'C': // Capability register
      Info.setAllowsRegister();
      return IsCHERI;
    case 'Z':
      if (Name[1] == 'C') { // An address usable by ll, and sc.
        Info.setAllowsMemory();
        Name++; // Skip over 'Z'.
        return true;
      }
      return false;
    }
  }

  std::string convertConstraint(const char *&Constraint) const override {
    std::string R;
    switch (*Constraint) {
    case 'Z': // Two-character constraint; add "^" hint for later parsing.
      if (Constraint[1] == 'C') {
        R = std::string("^") + std::string(Constraint, 2);
        Constraint++;
        return R;
      }
      break;
    }
    return TargetInfo::convertConstraint(Constraint);
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    return (((CC == CC_CHERICCallee) || (CC == CC_CHERICCallback) ||
             (CC == CC_CHERICCall)) && IsCHERI) ?
        CCCR_OK : CCCR_Warning;
  }

  const char *getClobbers() const override {
    // In GCC, $1 is not widely used in generated code (it's used only in a few
    // specific situations), so there is no real need for users to add it to
    // the clobbers list if they want to use it in their inline assembly code.
    //
    // In LLVM, $1 is treated as a normal GPR and is always allocatable during
    // code generation, so using it in inline assembly without adding it to the
    // clobbers list can cause conflicts between the inline assembly code and
    // the surrounding generated code.
    //
    // Another problem is that LLVM is allowed to choose $1 for inline assembly
    // operands, which will conflict with the ".set at" assembler option (which
    // we use only for inline assembly, in order to maintain compatibility with
    // GCC) and will also conflict with the user's usage of $1.
    //
    // The easiest way to avoid these conflicts and keep $1 as an allocatable
    // register for generated code is to automatically clobber $1 for all inline
    // assembly code.
    //
    // FIXME: We should automatically clobber $1 only for inline assembly code
    // which actually uses it. This would allow LLVM to use $1 for inline
    // assembly operands if the user's assembly code doesn't use it.
    return "~{$1}";
  }

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override {
    IsMips16 = false;
    IsMicromips = false;
    IsNan2008 = isIEEE754_2008Default();
    IsAbs2008 = isIEEE754_2008Default();
    IsSingleFloat = false;
    FloatABI = HardFloat;
    DspRev = NoDSP;
    FPMode = isFP64Default() ? FP64 : FPXX;
    bool CapSizeFeatureFound = false;

    for (const auto &Feature : Features) {
      if (Feature == "+single-float")
        IsSingleFloat = true;
      else if (Feature == "+soft-float")
        FloatABI = SoftFloat;
      else if (Feature == "+mips16")
        IsMips16 = true;
      else if (Feature == "+micromips")
        IsMicromips = true;
      else if (Feature == "+dsp")
        DspRev = std::max(DspRev, DSP1);
      else if (Feature == "+dspr2")
        DspRev = std::max(DspRev, DSP2);
      else if (Feature == "+msa")
        HasMSA = true;
      else if (Feature == "+nomadd4")
        DisableMadd4 = true;
      else if (Feature == "+fp64")
        FPMode = FP64;
      else if (Feature == "-fp64")
        FPMode = FP32;
      else if (Feature == "+fpxx")
        FPMode = FPXX;
      else if (Feature == "+nan2008")
        IsNan2008 = true;
      else if (Feature == "+chericap")
        IsCHERI = true;
      else if (Feature == "+cheri64") {
        CapSizeFeatureFound = true;
        CapSize = 64;
        IsCHERI = true;
      } else if (Feature == "+cheri128") {
        CapSizeFeatureFound = true;
        CapSize = 128;
        IsCHERI = true;
      } else if (Feature == "+cheri256") {
        CapSizeFeatureFound = true;
        CapSize = 256;
        IsCHERI = true;
      } else if (Feature == "-nan2008")
        IsNan2008 = false;
      else if (Feature == "+abs2008")
        IsAbs2008 = true;
      else if (Feature == "-abs2008")
        IsAbs2008 = false;
      else if (Feature == "+noabicalls")
        IsNoABICalls = true;
      else if (Feature == "+use-indirect-jump-hazard")
        UseIndirectJumpHazard = true;
    }

    if (IsCHERI) {
      // If we have an implicit size, assume cheri128
      if (!CapSizeFeatureFound) {
        Features.push_back("+cheri128");
        CapSize = 128;
      }
      assert(CapSize > 0);
      SuitableAlign = std::max(SuitableAlign, (unsigned short)CapSize);
      DefaultAlignForAttributeAligned =
          std::max(DefaultAlignForAttributeAligned, (unsigned short)CapSize);
    }

    setDataLayout();

    return true;
  }

  int getEHDataRegisterNumber(unsigned RegNo) const override {
    if (RegNo == 0)
      return CapabilityABI ? 88 : 4;
    if (RegNo == 1)
      return 5;
    return -1;
  }

  bool isCLZForZeroUndef() const override { return false; }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    static const TargetInfo::GCCRegAlias O32RegAliases[] = {
        {{"at"}, "$1"},  {{"v0"}, "$2"},         {{"v1"}, "$3"},
        {{"a0"}, "$4"},  {{"a1"}, "$5"},         {{"a2"}, "$6"},
        {{"a3"}, "$7"},  {{"t0"}, "$8"},         {{"t1"}, "$9"},
        {{"t2"}, "$10"}, {{"t3"}, "$11"},        {{"t4"}, "$12"},
        {{"t5"}, "$13"}, {{"t6"}, "$14"},        {{"t7"}, "$15"},
        {{"s0"}, "$16"}, {{"s1"}, "$17"},        {{"s2"}, "$18"},
        {{"s3"}, "$19"}, {{"s4"}, "$20"},        {{"s5"}, "$21"},
        {{"s6"}, "$22"}, {{"s7"}, "$23"},        {{"t8"}, "$24"},
        {{"t9"}, "$25"}, {{"k0"}, "$26"},        {{"k1"}, "$27"},
        {{"gp"}, "$28"}, {{"sp", "$sp"}, "$29"}, {{"fp", "$fp"}, "$30"},
        {{"ra"}, "$31"}
    };
    static const TargetInfo::GCCRegAlias NewABIRegAliases[] = {
        {{"at"}, "$1"},  {{"v0"}, "$2"},         {{"v1"}, "$3"},
        {{"a0"}, "$4"},  {{"a1"}, "$5"},         {{"a2"}, "$6"},
        {{"a3"}, "$7"},  {{"a4"}, "$8"},         {{"a5"}, "$9"},
        {{"a6"}, "$10"}, {{"a7"}, "$11"},        {{"t0"}, "$12"},
        {{"t1"}, "$13"}, {{"t2"}, "$14"},        {{"t3"}, "$15"},
        {{"s0"}, "$16"}, {{"s1"}, "$17"},        {{"s2"}, "$18"},
        {{"s3"}, "$19"}, {{"s4"}, "$20"},        {{"s5"}, "$21"},
        {{"s6"}, "$22"}, {{"s7"}, "$23"},        {{"t8"}, "$24"},
        {{"t9"}, "$25"}, {{"k0"}, "$26"},        {{"k1"}, "$27"},
        {{"gp"}, "$28"}, {{"sp", "$sp"}, "$29"}, {{"fp", "$fp"}, "$30"},
        {{"ra"}, "$31"}
    };
    if (ABI == "o32")
      return llvm::makeArrayRef(O32RegAliases);
    return llvm::makeArrayRef(NewABIRegAliases);
  }

  bool hasInt128Type() const override {
    return (getTriple().getArch() == llvm::Triple::mips64 ||
           getTriple().getArch() == llvm::Triple::mips64el ||
           ABI == "n32" || ABI == "n64" || ABI == "purecap") ||
           getTargetOpts().ForceEnableInt128;
  }

  unsigned getIntCapWidth() const override { return CapSize; }
  unsigned getIntCapAlign() const override { return CapSize; }

  uint64_t getCHERICapabilityWidth() const override { return CapSize; }

  uint64_t getCHERICapabilityAlign() const override { return CapSize; }

  uint64_t getPointerRangeForCHERICapability() const override { return 64; }

  uint64_t getPointerWidthV(unsigned AddrSpace) const override {
    return isCheriPointer(AddrSpace, &getDataLayout()) ? CapSize : PointerWidth;
  }

  uint64_t getPointerRangeV(unsigned AddrSpace) const override {
    return isCheriPointer(AddrSpace, &getDataLayout()) ? getPointerRangeForCHERICapability() : PointerWidth;
  }

  uint64_t getPointerAlignV(unsigned AddrSpace) const override {
    return isCheriPointer(AddrSpace, &getDataLayout()) ? CapSize : PointerAlign;
  }

  bool SupportsCapabilities() const override { return IsCHERI; }

  bool hasBuiltinAtomic(uint64_t AtomicSizeInBits,
                        uint64_t AlignmentInBits) const override {
#if 0
    if (SupportsCapabilities() &&
        AtomicSizeInBits == getCHERICapabilityWidth() &&
        AlignmentInBits == getCHERICapabilityAlign()) {
      return true;
    }
#endif
    return TargetInfo::hasBuiltinAtomic(AtomicSizeInBits, AlignmentInBits);
  }

  unsigned getUnwindWordWidth() const override;

  bool validateTarget(DiagnosticsEngine &Diags) const override;
};
} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_MIPS_H
