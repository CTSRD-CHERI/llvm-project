//===-- MipsMCTargetDesc.cpp - Mips Target Descriptions -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Mips specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "MipsMCTargetDesc.h"
#include "InstPrinter/MipsInstPrinter.h"
#include "MipsMCAsmInfo.h"
#include "MipsTargetStreamer.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "MipsGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MipsGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MipsGenRegisterInfo.inc"

using namespace llvm;

static std::string ParseMipsTriple(StringRef TT, StringRef CPU) {
  std::string MipsArchFeature;
  size_t DashPosition = 0;
  StringRef TheTriple;

  // Let's see if there is a dash, like mips-unknown-linux.
  DashPosition = TT.find('-');

  if (DashPosition == StringRef::npos) {
    // No dash, we check the string size.
    TheTriple = TT.substr(0);
  } else {
    // We are only interested in substring before dash.
    TheTriple = TT.substr(0,DashPosition);
  }

  if (TheTriple == "mips" || TheTriple == "mipsel") {
    if (CPU.empty() || CPU == "mips32") {
      MipsArchFeature = "+mips32";
    } else if (CPU == "mips32r2") {
      MipsArchFeature = "+mips32r2";
    }
  } else {
      if (CPU.empty() || CPU == "mips64") {
        MipsArchFeature = "+mips64";
      } else if (CPU == "mips64r2") {
        MipsArchFeature = "+mips64r2";
      } else if (CPU == "cheri") {
        MipsArchFeature = "+cheri";
      }
  }
  return MipsArchFeature;
}

static MCInstrInfo *createMipsMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMipsMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMipsMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMipsMCRegisterInfo(X, Mips::RA);
  return X;
}

static MCSubtargetInfo *createMipsMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                  StringRef FS) {
  std::string ArchFS = ParseMipsTriple(TT,CPU);
  if (!FS.empty()) {
    if (!ArchFS.empty())
      ArchFS = ArchFS + "," + FS.str();
    else
      ArchFS = FS;
  }
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitMipsMCSubtargetInfo(X, TT, CPU, ArchFS);
  return X;
}

static MCAsmInfo *createMipsMCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
  MCAsmInfo *MAI = new MipsMCAsmInfo(TT);

  unsigned SP = MRI.getDwarfRegNum(Mips::SP, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(0, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCCodeGenInfo *createMipsMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                              CodeModel::Model CM,
                                              CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (CM == CodeModel::JITDefault)
    RM = Reloc::Static;
  else if (RM == Reloc::Default)
    RM = Reloc::PIC_;
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createMipsMCInstPrinter(const Target &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI,
                                              const MCSubtargetInfo &STI) {
  return new MipsInstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
                                    MCContext &Context, MCAsmBackend &MAB,
                                    raw_ostream &OS, MCCodeEmitter *Emitter,
                                    bool RelaxAll, bool NoExecStack) {
  MipsTargetELFStreamer *S = new MipsTargetELFStreamer();
  return createELFStreamer(Context, S, MAB, OS, Emitter, RelaxAll, NoExecStack);
}

static MCStreamer *
createMCAsmStreamer(MCContext &Ctx, formatted_raw_ostream &OS,
                    bool isVerboseAsm, bool useLoc, bool useCFI,
                    bool useDwarfDirectory, MCInstPrinter *InstPrint,
                    MCCodeEmitter *CE, MCAsmBackend *TAB, bool ShowInst) {
  MipsTargetAsmStreamer *S = new MipsTargetAsmStreamer(OS);

  return llvm::createAsmStreamer(Ctx, S, OS, isVerboseAsm, useLoc, useCFI,
                                 useDwarfDirectory, InstPrint, CE, TAB,
                                 ShowInst);
}

static void registerTarget(Target &T, bool isBigEndian, bool is64Bit) {
  // Register the MC asm info.
  RegisterMCAsmInfoFn AsmInfo(T, createMipsMCAsmInfo);
  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(T, createMipsMCCodeGenInfo);
  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(T, createMipsMCInstrInfo);
  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(T, createMipsMCRegisterInfo);
  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(T,
      isBigEndian ?  createMipsMCCodeEmitterEB : createMipsMCCodeEmitterEL);
  // Register the object streamer.
  TargetRegistry::RegisterMCObjectStreamer(T, createMCStreamer);
  // Register the asm streamer.
  TargetRegistry::RegisterAsmStreamer(T, createMCAsmStreamer);
  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(T,
   is64Bit ?
     (isBigEndian ? createMipsAsmBackendEB64 : createMipsAsmBackendEL64) :
     (isBigEndian ? createMipsAsmBackendEB32 : createMipsAsmBackendEL32));
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(T, createMipsMCSubtargetInfo);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(T, createMipsMCInstPrinter);
}

extern "C" void LLVMInitializeMipsTargetMC() {
  registerTarget(TheMipsTarget, /* isBigEndian */true, /*is64Bit*/false);
  registerTarget(TheMipselTarget, /* isBigEndian */false, /*is64Bit*/false);
  registerTarget(TheMips4Target, /* isBigEndian */true, /*is64Bit*/true);
  registerTarget(TheMips64Target, /* isBigEndian */true, /*is64Bit*/true);
  registerTarget(TheMips64elTarget, /* isBigEndian */false, /*is64Bit*/true);
  registerTarget(TheMipsCheriTarget, /* isBigEndian */true, /*is64Bit*/true);
}
