//===- AsmPrinter.cpp - Common AsmPrinter code ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the AsmPrinter class.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/AsmPrinter.h"
#include "CodeViewDebug.h"
#include "DwarfDebug.h"
#include "DwarfException.h"
#include "PseudoProbePrinter.h"
#include "WasmException.h"
#include "WinCFGuard.h"
#include "WinException.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/CodeGen/GCMetadata.h"
#include "llvm/CodeGen/GCMetadataPrinter.h"
#include "llvm/CodeGen/LazyMachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBundle.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/StackMaps.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Config/config.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Cheri.h"
#include "llvm/IR/Comdat.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/EHPersonalities.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GCStrategy.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalIFunc.h"
#include "llvm/IR/GlobalObject.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PseudoProbe.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/SectionKind.h"
#include "llvm/Object/ELFTypes.h"
#include "llvm/Pass.h"
#include "llvm/Remarks/RemarkStreamer.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

static cl::opt<std::string> BasicBlockProfileDump(
    "mbb-profile-dump", cl::Hidden,
    cl::desc("Basic block profile dump for external cost modelling. If "
             "matching up BBs with afterwards, the compilation must be "
             "performed with -basic-block-sections=labels. Enabling this "
             "flag during in-process ThinLTO is not supported."));

const char DWARFGroupName[] = "dwarf";
const char DWARFGroupDescription[] = "DWARF Emission";
const char DbgTimerName[] = "emit";
const char DbgTimerDescription[] = "Debug Info Emission";
const char EHTimerName[] = "write_exception";
const char EHTimerDescription[] = "DWARF Exception Writer";
const char CFGuardName[] = "Control Flow Guard";
const char CFGuardDescription[] = "Control Flow Guard";
const char CodeViewLineTablesGroupName[] = "linetables";
const char CodeViewLineTablesGroupDescription[] = "CodeView Line Tables";
const char PPTimerName[] = "emit";
const char PPTimerDescription[] = "Pseudo Probe Emission";
const char PPGroupName[] = "pseudo probe";
const char PPGroupDescription[] = "Pseudo Probe Emission";

STATISTIC(EmittedInsts, "Number of machine instrs printed");

char AsmPrinter::ID = 0;

namespace {
class AddrLabelMapCallbackPtr final : CallbackVH {
  AddrLabelMap *Map = nullptr;

public:
  AddrLabelMapCallbackPtr() = default;
  AddrLabelMapCallbackPtr(Value *V) : CallbackVH(V) {}

  void setPtr(BasicBlock *BB) {
    ValueHandleBase::operator=(BB);
  }

  void setMap(AddrLabelMap *map) { Map = map; }

  void deleted() override;
  void allUsesReplacedWith(Value *V2) override;
};
} // namespace

class llvm::AddrLabelMap {
  MCContext &Context;
  struct AddrLabelSymEntry {
    /// The symbols for the label.
    TinyPtrVector<MCSymbol *> Symbols;

    Function *Fn;   // The containing function of the BasicBlock.
    unsigned Index; // The index in BBCallbacks for the BasicBlock.
  };

  DenseMap<AssertingVH<BasicBlock>, AddrLabelSymEntry> AddrLabelSymbols;

  /// Callbacks for the BasicBlock's that we have entries for.  We use this so
  /// we get notified if a block is deleted or RAUWd.
  std::vector<AddrLabelMapCallbackPtr> BBCallbacks;

  /// This is a per-function list of symbols whose corresponding BasicBlock got
  /// deleted.  These symbols need to be emitted at some point in the file, so
  /// AsmPrinter emits them after the function body.
  DenseMap<AssertingVH<Function>, std::vector<MCSymbol *>>
      DeletedAddrLabelsNeedingEmission;

public:
  AddrLabelMap(MCContext &context) : Context(context) {}

  ~AddrLabelMap() {
    assert(DeletedAddrLabelsNeedingEmission.empty() &&
           "Some labels for deleted blocks never got emitted");
  }

  ArrayRef<MCSymbol *> getAddrLabelSymbolToEmit(BasicBlock *BB);

  void takeDeletedSymbolsForFunction(Function *F,
                                     std::vector<MCSymbol *> &Result);

  void UpdateForDeletedBlock(BasicBlock *BB);
  void UpdateForRAUWBlock(BasicBlock *Old, BasicBlock *New);
};

ArrayRef<MCSymbol *> AddrLabelMap::getAddrLabelSymbolToEmit(BasicBlock *BB) {
  assert(BB->hasAddressTaken() &&
         "Shouldn't get label for block without address taken");
  AddrLabelSymEntry &Entry = AddrLabelSymbols[BB];

  // If we already had an entry for this block, just return it.
  if (!Entry.Symbols.empty()) {
    assert(BB->getParent() == Entry.Fn && "Parent changed");
    return Entry.Symbols;
  }

  // Otherwise, this is a new entry, create a new symbol for it and add an
  // entry to BBCallbacks so we can be notified if the BB is deleted or RAUWd.
  BBCallbacks.emplace_back(BB);
  BBCallbacks.back().setMap(this);
  Entry.Index = BBCallbacks.size() - 1;
  Entry.Fn = BB->getParent();
  MCSymbol *Sym = BB->hasAddressTaken() ? Context.createNamedTempSymbol()
                                        : Context.createTempSymbol();
  Entry.Symbols.push_back(Sym);
  return Entry.Symbols;
}

/// If we have any deleted symbols for F, return them.
void AddrLabelMap::takeDeletedSymbolsForFunction(
    Function *F, std::vector<MCSymbol *> &Result) {
  DenseMap<AssertingVH<Function>, std::vector<MCSymbol *>>::iterator I =
      DeletedAddrLabelsNeedingEmission.find(F);

  // If there are no entries for the function, just return.
  if (I == DeletedAddrLabelsNeedingEmission.end())
    return;

  // Otherwise, take the list.
  std::swap(Result, I->second);
  DeletedAddrLabelsNeedingEmission.erase(I);
}

//===- Address of Block Management ----------------------------------------===//

ArrayRef<MCSymbol *>
AsmPrinter::getAddrLabelSymbolToEmit(const BasicBlock *BB) {
  // Lazily create AddrLabelSymbols.
  if (!AddrLabelSymbols)
    AddrLabelSymbols = std::make_unique<AddrLabelMap>(OutContext);
  return AddrLabelSymbols->getAddrLabelSymbolToEmit(
      const_cast<BasicBlock *>(BB));
}

void AsmPrinter::takeDeletedSymbolsForFunction(
    const Function *F, std::vector<MCSymbol *> &Result) {
  // If no blocks have had their addresses taken, we're done.
  if (!AddrLabelSymbols)
    return;
  return AddrLabelSymbols->takeDeletedSymbolsForFunction(
      const_cast<Function *>(F), Result);
}

void AddrLabelMap::UpdateForDeletedBlock(BasicBlock *BB) {
  // If the block got deleted, there is no need for the symbol.  If the symbol
  // was already emitted, we can just forget about it, otherwise we need to
  // queue it up for later emission when the function is output.
  AddrLabelSymEntry Entry = std::move(AddrLabelSymbols[BB]);
  AddrLabelSymbols.erase(BB);
  assert(!Entry.Symbols.empty() && "Didn't have a symbol, why a callback?");
  BBCallbacks[Entry.Index] = nullptr; // Clear the callback.

#if !LLVM_MEMORY_SANITIZER_BUILD
  // BasicBlock is destroyed already, so this access is UB detectable by msan.
  assert((BB->getParent() == nullptr || BB->getParent() == Entry.Fn) &&
         "Block/parent mismatch");
#endif

  for (MCSymbol *Sym : Entry.Symbols) {
    if (Sym->isDefined())
      return;

    // If the block is not yet defined, we need to emit it at the end of the
    // function.  Add the symbol to the DeletedAddrLabelsNeedingEmission list
    // for the containing Function.  Since the block is being deleted, its
    // parent may already be removed, we have to get the function from 'Entry'.
    DeletedAddrLabelsNeedingEmission[Entry.Fn].push_back(Sym);
  }
}

void AddrLabelMap::UpdateForRAUWBlock(BasicBlock *Old, BasicBlock *New) {
  // Get the entry for the RAUW'd block and remove it from our map.
  AddrLabelSymEntry OldEntry = std::move(AddrLabelSymbols[Old]);
  AddrLabelSymbols.erase(Old);
  assert(!OldEntry.Symbols.empty() && "Didn't have a symbol, why a callback?");

  AddrLabelSymEntry &NewEntry = AddrLabelSymbols[New];

  // If New is not address taken, just move our symbol over to it.
  if (NewEntry.Symbols.empty()) {
    BBCallbacks[OldEntry.Index].setPtr(New); // Update the callback.
    NewEntry = std::move(OldEntry);          // Set New's entry.
    return;
  }

  BBCallbacks[OldEntry.Index] = nullptr; // Update the callback.

  // Otherwise, we need to add the old symbols to the new block's set.
  llvm::append_range(NewEntry.Symbols, OldEntry.Symbols);
}

void AddrLabelMapCallbackPtr::deleted() {
  Map->UpdateForDeletedBlock(cast<BasicBlock>(getValPtr()));
}

void AddrLabelMapCallbackPtr::allUsesReplacedWith(Value *V2) {
  Map->UpdateForRAUWBlock(cast<BasicBlock>(getValPtr()), cast<BasicBlock>(V2));
}

/// getGVAlignment - Return the alignment to use for the specified global
/// value.  This rounds up to the preferred alignment if possible and legal.
Align AsmPrinter::getGVAlignment(const GlobalObject *GV, const DataLayout &DL,
                                 Align InAlign) {
  Align Alignment;
  if (const GlobalVariable *GVar = dyn_cast<GlobalVariable>(GV))
    Alignment = DL.getPreferredAlign(GVar);

  // If InAlign is specified, round it to it.
  if (InAlign > Alignment)
    Alignment = InAlign;

  // If the GV has a specified alignment, take it into account.
  const MaybeAlign GVAlign(GV->getAlign());
  if (!GVAlign)
    return Alignment;

  assert(GVAlign && "GVAlign must be set");

  // If the GVAlign is larger than NumBits, or if we are required to obey
  // NumBits because the GV has an assigned section, obey it.
  if (*GVAlign > Alignment || GV->hasSection())
    Alignment = *GVAlign;
  return Alignment;
}

AsmPrinter::AsmPrinter(TargetMachine &tm, std::unique_ptr<MCStreamer> Streamer)
    : MachineFunctionPass(ID), TM(tm), MAI(tm.getMCAsmInfo()),
      OutContext(Streamer->getContext()), OutStreamer(std::move(Streamer)),
      SM(*this) {
  VerboseAsm = OutStreamer->isVerboseAsm();
  DwarfUsesRelocationsAcrossSections =
      MAI->doesDwarfUseRelocationsAcrossSections();
}

AsmPrinter::~AsmPrinter() {
  assert(!DD && Handlers.size() == NumUserHandlers &&
         "Debug/EH info didn't get finalized");
}

bool AsmPrinter::isPositionIndependent() const {
  return TM.isPositionIndependent();
}

/// getFunctionNumber - Return a unique ID for the current function.
unsigned AsmPrinter::getFunctionNumber() const {
  return MF->getFunctionNumber();
}

const TargetLoweringObjectFile &AsmPrinter::getObjFileLowering() const {
  return *TM.getObjFileLowering();
}

const DataLayout &AsmPrinter::getDataLayout() const {
  return MMI->getModule()->getDataLayout();
}

// Do not use the cached DataLayout because some client use it without a Module
// (dsymutil, llvm-dwarfdump).
unsigned AsmPrinter::getPointerSize() const {
  return TM.getPointerSize(0); // FIXME: Default address space
}

const MCSubtargetInfo &AsmPrinter::getSubtargetInfo() const {
  assert(MF && "getSubtargetInfo requires a valid MachineFunction!");
  return MF->getSubtarget<MCSubtargetInfo>();
}

void AsmPrinter::EmitToStreamer(MCStreamer &S, const MCInst &Inst) {
  S.emitInstruction(Inst, getSubtargetInfo());
}

void AsmPrinter::emitInitialRawDwarfLocDirective(const MachineFunction &MF) {
  if (DD) {
    assert(OutStreamer->hasRawTextSupport() &&
           "Expected assembly output mode.");
    // This is NVPTX specific and it's unclear why.
    // PR51079: If we have code without debug information we need to give up.
    DISubprogram *MFSP = MF.getFunction().getSubprogram();
    if (!MFSP)
      return;
    (void)DD->emitInitialLocDirective(MF, /*CUID=*/0);
  }
}

/// getCurrentSection() - Return the current section we are emitting to.
const MCSection *AsmPrinter::getCurrentSection() const {
  return OutStreamer->getCurrentSectionOnly();
}

void AsmPrinter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
  AU.addRequired<MachineOptimizationRemarkEmitterPass>();
  AU.addRequired<GCModuleInfo>();
  AU.addRequired<LazyMachineBlockFrequencyInfoPass>();
}

bool AsmPrinter::doInitialization(Module &M) {
  auto *MMIWP = getAnalysisIfAvailable<MachineModuleInfoWrapperPass>();
  MMI = MMIWP ? &MMIWP->getMMI() : nullptr;
  HasSplitStack = false;
  HasNoSplitStack = false;

  AddrLabelSymbols = nullptr;

  // Initialize TargetLoweringObjectFile.
  const_cast<TargetLoweringObjectFile&>(getObjFileLowering())
    .Initialize(OutContext, TM);

  const_cast<TargetLoweringObjectFile &>(getObjFileLowering())
      .getModuleMetadata(M);

  OutStreamer->initSections(false, *TM.getMCSubtargetInfo());

  // Emit the version-min deployment target directive if needed.
  //
  // FIXME: If we end up with a collection of these sorts of Darwin-specific
  // or ELF-specific things, it may make sense to have a platform helper class
  // that will work with the target helper class. For now keep it here, as the
  // alternative is duplicated code in each of the target asm printers that
  // use the directive, where it would need the same conditionalization
  // anyway.
  const Triple &Target = TM.getTargetTriple();
  Triple TVT(M.getDarwinTargetVariantTriple());
  OutStreamer->emitVersionForTarget(
      Target, M.getSDKVersion(),
      M.getDarwinTargetVariantTriple().empty() ? nullptr : &TVT,
      M.getDarwinTargetVariantSDKVersion());

  // Allow the target to emit any magic that it wants at the start of the file.
  emitStartOfAsmFile(M);

  // Very minimal debug info. It is ignored if we emit actual debug info. If we
  // don't, this at least helps the user find where a global came from.
  if (MAI->hasSingleParameterDotFile()) {
    // .file "foo.c"

    SmallString<128> FileName;
    if (MAI->hasBasenameOnlyForFileDirective())
      FileName = llvm::sys::path::filename(M.getSourceFileName());
    else
      FileName = M.getSourceFileName();
    if (MAI->hasFourStringsDotFile()) {
#ifdef PACKAGE_VENDOR
      const char VerStr[] =
          PACKAGE_VENDOR " " PACKAGE_NAME " version " PACKAGE_VERSION;
#else
      const char VerStr[] = PACKAGE_NAME " version " PACKAGE_VERSION;
#endif
      // TODO: Add timestamp and description.
      OutStreamer->emitFileDirective(FileName, VerStr, "", "");
    } else {
      OutStreamer->emitFileDirective(FileName);
    }
  }

  // On AIX, emit bytes for llvm.commandline metadata after .file so that the
  // C_INFO symbol is preserved if any csect is kept by the linker.
  if (TM.getTargetTriple().isOSBinFormatXCOFF())
    emitModuleCommandLines(M);

  GCModuleInfo *MI = getAnalysisIfAvailable<GCModuleInfo>();
  assert(MI && "AsmPrinter didn't require GCModuleInfo?");
  for (const auto &I : *MI)
    if (GCMetadataPrinter *MP = getOrCreateGCPrinter(*I))
      MP->beginAssembly(M, *MI, *this);

  // Emit module-level inline asm if it exists.
  if (!M.getModuleInlineAsm().empty()) {
    OutStreamer->AddComment("Start of file scope inline assembly");
    OutStreamer->addBlankLine();
    emitInlineAsm(M.getModuleInlineAsm() + "\n", *TM.getMCSubtargetInfo(),
                  TM.Options.MCOptions);
    OutStreamer->AddComment("End of file scope inline assembly");
    OutStreamer->addBlankLine();
  }

  if (MAI->doesSupportDebugInformation()) {
    bool EmitCodeView = M.getCodeViewFlag();
    if (EmitCodeView && TM.getTargetTriple().isOSWindows()) {
      Handlers.emplace_back(std::make_unique<CodeViewDebug>(this),
                            DbgTimerName, DbgTimerDescription,
                            CodeViewLineTablesGroupName,
                            CodeViewLineTablesGroupDescription);
    }
    if (!EmitCodeView || M.getDwarfVersion()) {
      if (MMI->hasDebugInfo()) {
        DD = new DwarfDebug(this);
        Handlers.emplace_back(std::unique_ptr<DwarfDebug>(DD), DbgTimerName,
                              DbgTimerDescription, DWARFGroupName,
                              DWARFGroupDescription);
      }
    }
  }

  if (M.getNamedMetadata(PseudoProbeDescMetadataName)) {
    PP = new PseudoProbeHandler(this);
    Handlers.emplace_back(std::unique_ptr<PseudoProbeHandler>(PP), PPTimerName,
                          PPTimerDescription, PPGroupName, PPGroupDescription);
  }

  switch (MAI->getExceptionHandlingType()) {
  case ExceptionHandling::None:
    // We may want to emit CFI for debug.
    [[fallthrough]];
  case ExceptionHandling::SjLj:
  case ExceptionHandling::DwarfCFI:
  case ExceptionHandling::ARM:
    for (auto &F : M.getFunctionList()) {
      if (getFunctionCFISectionType(F) != CFISection::None)
        ModuleCFISection = getFunctionCFISectionType(F);
      // If any function needsUnwindTableEntry(), it needs .eh_frame and hence
      // the module needs .eh_frame. If we have found that case, we are done.
      if (ModuleCFISection == CFISection::EH)
        break;
    }
    assert(MAI->getExceptionHandlingType() == ExceptionHandling::DwarfCFI ||
           usesCFIWithoutEH() || ModuleCFISection != CFISection::EH);
    break;
  default:
    break;
  }

  EHStreamer *ES = nullptr;
  switch (MAI->getExceptionHandlingType()) {
  case ExceptionHandling::None:
    if (!usesCFIWithoutEH())
      break;
    [[fallthrough]];
  case ExceptionHandling::SjLj:
  case ExceptionHandling::DwarfCFI:
    ES = new DwarfCFIException(this);
    break;
  case ExceptionHandling::ARM:
    ES = new ARMException(this);
    break;
  case ExceptionHandling::WinEH:
    switch (MAI->getWinEHEncodingType()) {
    default: llvm_unreachable("unsupported unwinding information encoding");
    case WinEH::EncodingType::Invalid:
      break;
    case WinEH::EncodingType::X86:
    case WinEH::EncodingType::Itanium:
      ES = new WinException(this);
      break;
    }
    break;
  case ExceptionHandling::Wasm:
    ES = new WasmException(this);
    break;
  case ExceptionHandling::AIX:
    ES = new AIXException(this);
    break;
  }
  if (ES)
    Handlers.emplace_back(std::unique_ptr<EHStreamer>(ES), EHTimerName,
                          EHTimerDescription, DWARFGroupName,
                          DWARFGroupDescription);

  // Emit tables for any value of cfguard flag (i.e. cfguard=1 or cfguard=2).
  if (mdconst::extract_or_null<ConstantInt>(M.getModuleFlag("cfguard")))
    Handlers.emplace_back(std::make_unique<WinCFGuard>(this), CFGuardName,
                          CFGuardDescription, DWARFGroupName,
                          DWARFGroupDescription);

  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                       HI.TimerGroupDescription, TimePassesIsEnabled);
    HI.Handler->beginModule(&M);
  }

  if (!BasicBlockProfileDump.empty()) {
    std::error_code PossibleFileError;
    MBBProfileDumpFileOutput = std::make_unique<raw_fd_ostream>(
        BasicBlockProfileDump, PossibleFileError);
    if (PossibleFileError) {
      M.getContext().emitError("Failed to open file for MBB Profile Dump: " +
                               PossibleFileError.message() + "\n");
    }
  }

  return false;
}

static bool canBeHidden(const GlobalValue *GV, const MCAsmInfo &MAI) {
  if (!MAI.hasWeakDefCanBeHiddenDirective())
    return false;

  return GV->canBeOmittedFromSymbolTable();
}

void AsmPrinter::emitLinkage(const GlobalValue *GV, MCSymbol *GVSym) const {
  GlobalValue::LinkageTypes Linkage = GV->getLinkage();
  switch (Linkage) {
  case GlobalValue::CommonLinkage:
  case GlobalValue::LinkOnceAnyLinkage:
  case GlobalValue::LinkOnceODRLinkage:
  case GlobalValue::WeakAnyLinkage:
  case GlobalValue::WeakODRLinkage:
    if (MAI->hasWeakDefDirective()) {
      // .globl _foo
      OutStreamer->emitSymbolAttribute(GVSym, MCSA_Global);

      if (!canBeHidden(GV, *MAI))
        // .weak_definition _foo
        OutStreamer->emitSymbolAttribute(GVSym, MCSA_WeakDefinition);
      else
        OutStreamer->emitSymbolAttribute(GVSym, MCSA_WeakDefAutoPrivate);
    } else if (MAI->avoidWeakIfComdat() && GV->hasComdat()) {
      // .globl _foo
      OutStreamer->emitSymbolAttribute(GVSym, MCSA_Global);
      //NOTE: linkonce is handled by the section the symbol was assigned to.
    } else {
      // .weak _foo
      OutStreamer->emitSymbolAttribute(GVSym, MCSA_Weak);
    }
    return;
  case GlobalValue::ExternalLinkage:
    OutStreamer->emitSymbolAttribute(GVSym, MCSA_Global);
    return;
  case GlobalValue::PrivateLinkage:
  case GlobalValue::InternalLinkage:
    return;
  case GlobalValue::ExternalWeakLinkage:
  case GlobalValue::AvailableExternallyLinkage:
  case GlobalValue::AppendingLinkage:
    llvm_unreachable("Should never emit this");
  }
  llvm_unreachable("Unknown linkage type!");
}

void AsmPrinter::getNameWithPrefix(SmallVectorImpl<char> &Name,
                                   const GlobalValue *GV) const {
  TM.getNameWithPrefix(Name, GV, getObjFileLowering().getMangler());
}

MCSymbol *AsmPrinter::getSymbol(const GlobalValue *GV) const {
  return TM.getSymbol(GV);
}

MCSymbol *AsmPrinter::getSymbolPreferLocal(const GlobalValue &GV,
                                           bool Force) const {
  // On ELF, use .Lfoo$local if GV is a non-interposable GlobalObject with an
  // exact definion (intersection of GlobalValue::hasExactDefinition() and
  // !isInterposable()). These linkages include: external, appending, internal,
  // private. It may be profitable to use a local alias for external. The
  // assembler would otherwise be conservative and assume a global default
  // visibility symbol can be interposable, even if the code generator already
  // assumed it.
  if (Force)
    return getSymbolWithGlobalValueBase(&GV, "$local");
  if (TM.getTargetTriple().isOSBinFormatELF() &&
      GV.canBenefitFromLocalAlias()) {
    const Module &M = *GV.getParent();
    if (TM.getRelocationModel() != Reloc::Static &&
        M.getPIELevel() == PIELevel::Default && GV.isDSOLocal())
      return getSymbolWithGlobalValueBase(&GV, "$local");
  }
  return TM.getSymbol(&GV);
}

/// EmitGlobalVariable - Emit the specified global variable to the .s file.
void AsmPrinter::emitGlobalVariable(const GlobalVariable *GV) {
  bool IsEmuTLSVar = TM.useEmulatedTLS() && GV->isThreadLocal();
  assert(!(IsEmuTLSVar && GV->hasCommonLinkage()) &&
         "No emulated TLS variables in the common section");

  // Never emit TLS variable xyz in emulated TLS model.
  // The initialization value is in __emutls_t.xyz instead of xyz.
  if (IsEmuTLSVar)
    return;

  if (GV->hasInitializer()) {
    // Check to see if this is a special global used by LLVM, if so, emit it.
    if (emitSpecialLLVMGlobal(GV))
      return;

    // Skip the emission of global equivalents. The symbol can be emitted later
    // on by emitGlobalGOTEquivs in case it turns out to be needed.
    if (GlobalGOTEquivs.count(getSymbol(GV)))
      return;

    if (isVerbose()) {
      // When printing the control variable __emutls_v.*,
      // we don't need to print the original TLS variable name.
      GV->printAsOperand(OutStreamer->getCommentOS(),
                         /*PrintType=*/false, GV->getParent());
      OutStreamer->getCommentOS() << '\n';
    }
  }

  MCSymbol *GVSym = getSymbol(GV);
  MCSymbol *EmittedSym = GVSym;

  // getOrCreateEmuTLSControlSym only creates the symbol with name and default
  // attributes.
  // GV's or GVSym's attributes will be used for the EmittedSym.
  emitVisibility(EmittedSym, GV->getVisibility(), !GV->isDeclaration());

  if (GV->isTagged()) {
    Triple T = TM.getTargetTriple();

    if (T.getArch() != Triple::aarch64 || !T.isAndroid())
      OutContext.reportError(SMLoc(),
                             "tagged symbols (-fsanitize=memtag-globals) are "
                             "only supported on AArch64 Android");
    OutStreamer->emitSymbolAttribute(EmittedSym, MAI->getMemtagAttr());
  }

  if (!GV->hasInitializer())   // External globals require no extra code.
    return;

  GVSym->redefineIfPossible();
  if (GVSym->isDefined() || GVSym->isVariable())
    OutContext.reportError(SMLoc(), "symbol '" + Twine(GVSym->getName()) +
                                        "' is already defined");

  if (MAI->hasDotTypeDotSizeDirective())
    OutStreamer->emitSymbolAttribute(EmittedSym, MCSA_ELF_TypeObject);

  SectionKind GVKind = TargetLoweringObjectFile::getKindForGlobal(GV, TM);

  const DataLayout &DL = GV->getParent()->getDataLayout();
  uint64_t Size = DL.getTypeAllocSize(GV->getValueType());

  // If the alignment is specified, we *must* obey it.  Overaligning a global
  // with a specified alignment is a prompt way to break globals emitted to
  // sections and expected to be contiguous (e.g. ObjC metadata).
  Align Alignment = getGVAlignment(GV, DL);

  const TailPaddingAmount TailPadding =
      getObjFileLowering().getTailPaddingForPreciseBounds(Size, TM);
  const Align PreciseAlignment =
      getObjFileLowering().getAlignmentForPreciseBounds(Size, TM);

  if (PreciseAlignment > Alignment) {
    LLVM_DEBUG(dbgs() << "\nIncreased alignment for global from "
                      << Alignment.value() << " to " << PreciseAlignment.value() << ": ";
               GV->dump(););
    // Don't increase alignment if a custom section has been specified:
    if (!GV->hasSection()) {
      Alignment = PreciseAlignment;
    } else {
      // TODO: add some attribute, emit a sensible warning?
      errs() << "Not overriding global variable alignment for " << GV->getName()
             << " since it has a section assigned.";
    }
  }

  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription,
                       HI.TimerGroupName, HI.TimerGroupDescription,
                       TimePassesIsEnabled);
    HI.Handler->setSymbolSize(GVSym, Size);
  }

  // Handle common symbols
  if (GVKind.isCommon()) {
    if (Size == 0) Size = 1;   // .comm Foo, 0 is undefined, avoid it.
    // .comm _foo, 42, 4
    OutStreamer->emitCommonSymbol(GVSym, Size, Alignment, TailPadding);
    return;
  }

  // Determine to which section this global should be emitted.
  MCSection *TheSection = getObjFileLowering().SectionForGlobal(GV, GVKind, TM);

  // If we have a bss global going to a section that supports the
  // zerofill directive, do so here.
  if (GVKind.isBSS() && MAI->hasMachoZeroFillDirective() &&
      TheSection->isVirtualSection()) {
    if (Size == 0)
      Size = 1; // zerofill of 0 bytes is undefined.
    emitLinkage(GV, GVSym);
    // .zerofill __DATA, __bss, _foo, 400, 5
    OutStreamer->emitZerofill(TheSection, GVSym, Size, Alignment, TailPadding);
    return;
  }

  // If this is a BSS local symbol and we are emitting in the BSS
  // section use .lcomm/.comm directive.
  if (GVKind.isBSSLocal() &&
      getObjFileLowering().getBSSSection() == TheSection) {
    if (Size == 0)
      Size = 1; // .comm Foo, 0 is undefined, avoid it.

    // Use .lcomm only if it supports user-specified alignment.
    // Otherwise, while it would still be correct to use .lcomm in some
    // cases (e.g. when Align == 1), the external assembler might enfore
    // some -unknown- default alignment behavior, which could cause
    // spurious differences between external and integrated assembler.
    // Prefer to simply fall back to .local / .comm in this case.
    if (MAI->getLCOMMDirectiveAlignmentType() != LCOMM::NoAlignment) {
      // .lcomm _foo, 42
      OutStreamer->emitLocalCommonSymbol(GVSym, Size, Alignment, TailPadding);
      return;
    }

    // .local _foo
    OutStreamer->emitSymbolAttribute(GVSym, MCSA_Local);
    // .comm _foo, 42, 4
    OutStreamer->emitCommonSymbol(GVSym, Size, Alignment, TailPadding);
    return;
  }

  // Handle thread local data for mach-o which requires us to output an
  // additional structure of data and mangle the original symbol so that we
  // can reference it later.
  //
  // TODO: This should become an "emit thread local global" method on TLOF.
  // All of this macho specific stuff should be sunk down into TLOFMachO and
  // stuff like "TLSExtraDataSection" should no longer be part of the parent
  // TLOF class.  This will also make it more obvious that stuff like
  // MCStreamer::EmitTBSSSymbol is macho specific and only called from macho
  // specific code.
  if (GVKind.isThreadLocal() && MAI->hasMachoTBSSDirective()) {
    // Emit the .tbss symbol
    MCSymbol *MangSym =
        OutContext.getOrCreateSymbol(GVSym->getName() + Twine("$tlv$init"));

    if (GVKind.isThreadBSS()) {
      TheSection = getObjFileLowering().getTLSBSSSection();
      OutStreamer->emitTBSSSymbol(TheSection, MangSym, Size, Alignment, TailPadding);
    } else if (GVKind.isThreadData()) {
      OutStreamer->switchSection(TheSection);

      emitAlignment(Alignment, GV);
      OutStreamer->emitLabel(MangSym);

      emitGlobalConstant(GV->getParent()->getDataLayout(), GV->getInitializer(),
                         static_cast<uint64_t>(TailPadding));
    }

    OutStreamer->addBlankLine();

    // Emit the variable struct for the runtime.
    MCSection *TLVSect = getObjFileLowering().getTLSExtraDataSection();

    OutStreamer->switchSection(TLVSect);
    // Emit the linkage here.
    emitLinkage(GV, GVSym);
    OutStreamer->emitLabel(GVSym);

    // Three pointers in size:
    //   - __tlv_bootstrap - used to make sure support exists
    //   - spare pointer, used when mapped by the runtime
    //   - pointer to mangled symbol above with initializer
    unsigned PtrSize = DL.getPointerTypeSize(GV->getType());
    OutStreamer->emitSymbolValue(GetExternalSymbolSymbol("_tlv_bootstrap"),
                                PtrSize);
    OutStreamer->emitIntValue(0, PtrSize);
    OutStreamer->emitSymbolValue(MangSym, PtrSize);

    OutStreamer->addBlankLine();
    return;
  }

  MCSymbol *EmittedInitSym = GVSym;

  OutStreamer->switchSection(TheSection);

  emitLinkage(GV, EmittedInitSym);
  emitAlignment(Alignment, GV);

  OutStreamer->emitLabel(EmittedInitSym);
  MCSymbol *LocalAlias = getSymbolPreferLocal(*GV);
  if (LocalAlias != EmittedInitSym)
    OutStreamer->emitLabel(LocalAlias);

  emitGlobalConstant(GV->getParent()->getDataLayout(), GV->getInitializer(),
                     static_cast<uint64_t>(TailPadding));

  if (MAI->hasDotTypeDotSizeDirective())
    // .size foo, 42
    OutStreamer->emitELFSize(EmittedInitSym,
                             MCConstantExpr::create(Size, OutContext));

  OutStreamer->addBlankLine();
}

/// Emit the directive and value for debug thread local expression
///
/// \p Value - The value to emit.
/// \p Size - The size of the integer (in bytes) to emit.
void AsmPrinter::emitDebugValue(const MCExpr *Value, unsigned Size) const {
  OutStreamer->emitValue(Value, Size);
}

void AsmPrinter::emitFunctionHeaderComment() {}

/// EmitFunctionHeader - This method emits the header for the current
/// function.
void AsmPrinter::emitFunctionHeader() {
  const Function &F = MF->getFunction();

  // FIXME: comment should probably be emitted after constant pool?
  //  or at least before .section .sdata
  if (isVerbose())
    OutStreamer->getCommentOS()
        << "-- Begin function "
        << GlobalValue::dropLLVMManglingEscape(F.getName()) << '\n';

  // Print out constants referenced by the function
  emitConstantPool();

  // Print the 'header' of function.
  // If basic block sections are desired, explicitly request a unique section
  // for this function's entry block.
  if (MF->front().isBeginSection())
    MF->setSection(getObjFileLowering().getUniqueSectionForFunction(F, TM));
  else
    MF->setSection(getObjFileLowering().SectionForGlobal(&F, TM));
  OutStreamer->switchSection(MF->getSection());

  if (!MAI->hasVisibilityOnlyWithLinkage())
    emitVisibility(CurrentFnSym, F.getVisibility());

  if (MAI->needsFunctionDescriptors())
    emitLinkage(&F, CurrentFnDescSym);

  emitLinkage(&F, CurrentFnSym);
  if (MAI->hasFunctionAlignment())
    emitAlignment(MF->getAlignment(), &F);

  if (MAI->hasDotTypeDotSizeDirective()) {
    OutStreamer->emitSymbolAttribute(CurrentFnSym, MCSA_ELF_TypeFunction);
  }

  if (F.hasFnAttribute(Attribute::Cold))
    OutStreamer->emitSymbolAttribute(CurrentFnSym, MCSA_Cold);

  // Emit the prefix data.
  if (F.hasPrefixData()) {
    if (MAI->hasSubsectionsViaSymbols()) {
      // Preserving prefix data on platforms which use subsections-via-symbols
      // is a bit tricky. Here we introduce a symbol for the prefix data
      // and use the .alt_entry attribute to mark the function's real entry point
      // as an alternative entry point to the prefix-data symbol.
      MCSymbol *PrefixSym = OutContext.createLinkerPrivateTempSymbol();
      OutStreamer->emitLabel(PrefixSym);

      emitGlobalConstant(F.getParent()->getDataLayout(), F.getPrefixData(), 0);

      // Emit an .alt_entry directive for the actual function symbol.
      OutStreamer->emitSymbolAttribute(CurrentFnSym, MCSA_AltEntry);
    } else {
      emitGlobalConstant(F.getParent()->getDataLayout(), F.getPrefixData(), 0);
    }
  }

  // Emit KCFI type information before patchable-function-prefix nops.
  emitKCFITypeId(*MF);

  // Emit M NOPs for -fpatchable-function-entry=N,M where M>0. We arbitrarily
  // place prefix data before NOPs.
  unsigned PatchableFunctionPrefix = 0;
  unsigned PatchableFunctionEntry = 0;
  (void)F.getFnAttribute("patchable-function-prefix")
      .getValueAsString()
      .getAsInteger(10, PatchableFunctionPrefix);
  (void)F.getFnAttribute("patchable-function-entry")
      .getValueAsString()
      .getAsInteger(10, PatchableFunctionEntry);
  if (PatchableFunctionPrefix) {
    CurrentPatchableFunctionEntrySym =
        OutContext.createLinkerPrivateTempSymbol();
    OutStreamer->emitLabel(CurrentPatchableFunctionEntrySym);
    emitNops(PatchableFunctionPrefix);
  } else if (PatchableFunctionEntry) {
    // May be reassigned when emitting the body, to reference the label after
    // the initial BTI (AArch64) or endbr32/endbr64 (x86).
    CurrentPatchableFunctionEntrySym = CurrentFnBegin;
  }

  // Emit the function prologue data for the indirect call sanitizer.
  if (const MDNode *MD = F.getMetadata(LLVMContext::MD_func_sanitize)) {
    assert(MD->getNumOperands() == 2);

    auto *PrologueSig = mdconst::extract<Constant>(MD->getOperand(0));
    auto *TypeHash = mdconst::extract<Constant>(MD->getOperand(1));
    emitGlobalConstant(F.getParent()->getDataLayout(), PrologueSig, 0);
    emitGlobalConstant(F.getParent()->getDataLayout(), TypeHash, 0);
  }

  if (isVerbose()) {
    F.printAsOperand(OutStreamer->getCommentOS(),
                     /*PrintType=*/false, F.getParent());
    emitFunctionHeaderComment();
    OutStreamer->getCommentOS() << '\n';
  }

  // Emit the function descriptor. This is a virtual function to allow targets
  // to emit their specific function descriptor. Right now it is only used by
  // the AIX target. The PowerPC 64-bit V1 ELF target also uses function
  // descriptors and should be converted to use this hook as well.
  if (MAI->needsFunctionDescriptors())
    emitFunctionDescriptor();

  // Emit the CurrentFnSym. This is a virtual function to allow targets to do
  // their wild and crazy things as required.
  emitFunctionEntryLabel();

  // If the function had address-taken blocks that got deleted, then we have
  // references to the dangling symbols.  Emit them at the start of the function
  // so that we don't get references to undefined symbols.
  std::vector<MCSymbol*> DeadBlockSyms;
  takeDeletedSymbolsForFunction(&F, DeadBlockSyms);
  for (MCSymbol *DeadBlockSym : DeadBlockSyms) {
    OutStreamer->AddComment("Address taken block that was later removed");
    OutStreamer->emitLabel(DeadBlockSym);
  }

  if (CurrentFnBegin) {
    if (MAI->useAssignmentForEHBegin()) {
      MCSymbol *CurPos = OutContext.createTempSymbol();
      OutStreamer->emitLabel(CurPos);
      OutStreamer->emitAssignment(CurrentFnBegin,
                                 MCSymbolRefExpr::create(CurPos, OutContext));
    } else {
      OutStreamer->emitLabel(CurrentFnBegin);
    }
  }

  // Emit pre-function debug and/or EH information.
  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                       HI.TimerGroupDescription, TimePassesIsEnabled);
    HI.Handler->beginFunction(MF);
  }
  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                       HI.TimerGroupDescription, TimePassesIsEnabled);
    HI.Handler->beginBasicBlockSection(MF->front());
  }

  // Emit the prologue data.
  if (F.hasPrologueData())
    emitGlobalConstant(F.getParent()->getDataLayout(), F.getPrologueData(), 0);
}

static bool needFuncLabelsForEH(const MachineFunction &MF);

/// Returns true if function begin label should be emitted due to a
/// BlockAddress used as an initializer for a GlobalVariable.
static bool needFuncLabelForBlockAddress(const MachineFunction &MF) {
  SmallVector<const Value *, 4> Users;

  for (const auto &MBB : MF) {
    if (!MBB.isIRBlockAddressTaken())
      continue;

    const auto *BB = MBB.getAddressTakenIRBlock();
    for (const auto &U : BB->users()) {
      if (isa<BlockAddress>(U))
        Users.push_back(U);
    }
  }

  while (!Users.empty()) {
    const Value *V = Users.pop_back_val();
    if (isa<GlobalVariable>(V))
      return true;
    if (isa<Instruction>(V))
      continue;
    for (const auto &U : V->users())
      Users.push_back(U);
  }

  return false;
}

/// EmitFunctionEntryLabel - Emit the label that is the entrypoint for the
/// function.  This can be overridden by targets as required to do custom stuff.
void AsmPrinter::emitFunctionEntryLabel() {
  CurrentFnSym->redefineIfPossible();

  // The function label could have already been emitted if two symbols end up
  // conflicting due to asm renaming.  Detect this and emit an error.
  if (CurrentFnSym->isVariable())
    report_fatal_error("'" + Twine(CurrentFnSym->getName()) +
                       "' is a protected alias");

  OutStreamer->emitLabel(CurrentFnSym);

  if (TM.getTargetTriple().isOSBinFormatELF()) {
    // For CHERI purecap exception handling, we always have to use a local
    // alias even if the function is not dso_local.
    bool ForceLocal =
        MAI->isCheriPurecapABI() &&
        (needFuncLabelsForEH(*MF) || needFuncLabelForBlockAddress(*MF));
    MCSymbol *Sym = getSymbolPreferLocal(MF->getFunction(), ForceLocal);
    if (Sym != CurrentFnSym) {
      cast<MCSymbolELF>(Sym)->setType(ELF::STT_FUNC);
      CurrentFnBeginLocal = Sym;
      OutStreamer->emitLabel(Sym);
      if (MAI->hasDotTypeDotSizeDirective())
        OutStreamer->emitSymbolAttribute(Sym, MCSA_ELF_TypeFunction);
    }
  }
}

/// emitComments - Pretty-print comments for instructions.
static void emitComments(const MachineInstr &MI, raw_ostream &CommentOS) {
  const MachineFunction *MF = MI.getMF();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();

  // Check for spills and reloads

  // We assume a single instruction only has a spill or reload, not
  // both.
  std::optional<unsigned> Size;
  if ((Size = MI.getRestoreSize(TII))) {
    CommentOS << *Size << "-byte Reload\n";
  } else if ((Size = MI.getFoldedRestoreSize(TII))) {
    if (*Size) {
      if (*Size == unsigned(MemoryLocation::UnknownSize))
        CommentOS << "Unknown-size Folded Reload\n";
      else
        CommentOS << *Size << "-byte Folded Reload\n";
    }
  } else if ((Size = MI.getSpillSize(TII))) {
    CommentOS << *Size << "-byte Spill\n";
  } else if ((Size = MI.getFoldedSpillSize(TII))) {
    if (*Size) {
      if (*Size == unsigned(MemoryLocation::UnknownSize))
        CommentOS << "Unknown-size Folded Spill\n";
      else
        CommentOS << *Size << "-byte Folded Spill\n";
    }
  }

  // Check for spill-induced copies
  if (MI.getAsmPrinterFlag(MachineInstr::ReloadReuse))
    CommentOS << " Reload Reuse\n";
}

/// emitImplicitDef - This method emits the specified machine instruction
/// that is an implicit def.
void AsmPrinter::emitImplicitDef(const MachineInstr *MI) const {
  Register RegNo = MI->getOperand(0).getReg();

  SmallString<128> Str;
  raw_svector_ostream OS(Str);
  OS << "implicit-def: "
     << printReg(RegNo, MF->getSubtarget().getRegisterInfo());

  OutStreamer->AddComment(OS.str());
  OutStreamer->addBlankLine();
}

static void emitKill(const MachineInstr *MI, AsmPrinter &AP) {
  std::string Str;
  raw_string_ostream OS(Str);
  OS << "kill:";
  for (const MachineOperand &Op : MI->operands()) {
    assert(Op.isReg() && "KILL instruction must have only register operands");
    OS << ' ' << (Op.isDef() ? "def " : "killed ")
       << printReg(Op.getReg(), AP.MF->getSubtarget().getRegisterInfo());
  }
  AP.OutStreamer->AddComment(OS.str());
  AP.OutStreamer->addBlankLine();
}

/// emitDebugValueComment - This method handles the target-independent form
/// of DBG_VALUE, returning true if it was able to do so.  A false return
/// means the target will need to handle MI in EmitInstruction.
static bool emitDebugValueComment(const MachineInstr *MI, AsmPrinter &AP) {
  // This code handles only the 4-operand target-independent form.
  if (MI->isNonListDebugValue() && MI->getNumOperands() != 4)
    return false;

  SmallString<128> Str;
  raw_svector_ostream OS(Str);
  OS << "DEBUG_VALUE: ";

  const DILocalVariable *V = MI->getDebugVariable();
  if (auto *SP = dyn_cast<DISubprogram>(V->getScope())) {
    StringRef Name = SP->getName();
    if (!Name.empty())
      OS << Name << ":";
  }
  OS << V->getName();
  OS << " <- ";

  const DIExpression *Expr = MI->getDebugExpression();
  // First convert this to a non-variadic expression if possible, to simplify
  // the output.
  if (auto NonVariadicExpr = DIExpression::convertToNonVariadicExpression(Expr))
    Expr = *NonVariadicExpr;
  // Then, output the possibly-simplified expression.
  if (Expr->getNumElements()) {
    OS << '[';
    ListSeparator LS;
    for (auto &Op : Expr->expr_ops()) {
      OS << LS << dwarf::OperationEncodingString(Op.getOp());
      for (unsigned I = 0; I < Op.getNumArgs(); ++I)
        OS << ' ' << Op.getArg(I);
    }
    OS << "] ";
  }

  // Register or immediate value. Register 0 means undef.
  for (const MachineOperand &Op : MI->debug_operands()) {
    if (&Op != MI->debug_operands().begin())
      OS << ", ";
    switch (Op.getType()) {
    case MachineOperand::MO_FPImmediate: {
      APFloat APF = APFloat(Op.getFPImm()->getValueAPF());
      Type *ImmTy = Op.getFPImm()->getType();
      if (ImmTy->isBFloatTy() || ImmTy->isHalfTy() || ImmTy->isFloatTy() ||
          ImmTy->isDoubleTy()) {
        OS << APF.convertToDouble();
      } else {
        // There is no good way to print long double.  Convert a copy to
        // double.  Ah well, it's only a comment.
        bool ignored;
        APF.convert(APFloat::IEEEdouble(), APFloat::rmNearestTiesToEven,
                    &ignored);
        OS << "(long double) " << APF.convertToDouble();
      }
      break;
    }
    case MachineOperand::MO_Immediate: {
      OS << Op.getImm();
      break;
    }
    case MachineOperand::MO_CImmediate: {
      Op.getCImm()->getValue().print(OS, false /*isSigned*/);
      break;
    }
    case MachineOperand::MO_TargetIndex: {
      OS << "!target-index(" << Op.getIndex() << "," << Op.getOffset() << ")";
      break;
    }
    case MachineOperand::MO_Register:
    case MachineOperand::MO_FrameIndex: {
      Register Reg;
      std::optional<StackOffset> Offset;
      if (Op.isReg()) {
        Reg = Op.getReg();
      } else {
        const TargetFrameLowering *TFI =
            AP.MF->getSubtarget().getFrameLowering();
        Offset = TFI->getFrameIndexReference(*AP.MF, Op.getIndex(), Reg);
      }
      if (!Reg) {
        // Suppress offset, it is not meaningful here.
        OS << "undef";
        break;
      }
      // The second operand is only an offset if it's an immediate.
      if (MI->isIndirectDebugValue())
        Offset = StackOffset::getFixed(MI->getDebugOffset().getImm());
      if (Offset)
        OS << '[';
      OS << printReg(Reg, AP.MF->getSubtarget().getRegisterInfo());
      if (Offset)
        OS << '+' << Offset->getFixed() << ']';
      break;
    }
    default:
      llvm_unreachable("Unknown operand type");
    }
  }

  // NOTE: Want this comment at start of line, don't emit with AddComment.
  AP.OutStreamer->emitRawComment(OS.str());
  return true;
}

/// This method handles the target-independent form of DBG_LABEL, returning
/// true if it was able to do so.  A false return means the target will need
/// to handle MI in EmitInstruction.
static bool emitDebugLabelComment(const MachineInstr *MI, AsmPrinter &AP) {
  if (MI->getNumOperands() != 1)
    return false;

  SmallString<128> Str;
  raw_svector_ostream OS(Str);
  OS << "DEBUG_LABEL: ";

  const DILabel *V = MI->getDebugLabel();
  if (auto *SP = dyn_cast<DISubprogram>(
          V->getScope()->getNonLexicalBlockFileScope())) {
    StringRef Name = SP->getName();
    if (!Name.empty())
      OS << Name << ":";
  }
  OS << V->getName();

  // NOTE: Want this comment at start of line, don't emit with AddComment.
  AP.OutStreamer->emitRawComment(OS.str());
  return true;
}

AsmPrinter::CFISection
AsmPrinter::getFunctionCFISectionType(const Function &F) const {
  // Ignore functions that won't get emitted.
  if (F.isDeclarationForLinker())
    return CFISection::None;

  if (MAI->getExceptionHandlingType() == ExceptionHandling::DwarfCFI &&
      F.needsUnwindTableEntry())
    return CFISection::EH;

  if (MAI->usesCFIWithoutEH() && F.hasUWTable())
    return CFISection::EH;

  assert(MMI != nullptr && "Invalid machine module info");
  if (MMI->hasDebugInfo() || TM.Options.ForceDwarfFrameSection)
    return CFISection::Debug;

  return CFISection::None;
}

AsmPrinter::CFISection
AsmPrinter::getFunctionCFISectionType(const MachineFunction &MF) const {
  return getFunctionCFISectionType(MF.getFunction());
}

bool AsmPrinter::needsSEHMoves() {
  return MAI->usesWindowsCFI() && MF->getFunction().needsUnwindTableEntry();
}

bool AsmPrinter::usesCFIWithoutEH() const {
  return MAI->usesCFIWithoutEH() && ModuleCFISection != CFISection::None;
}

void AsmPrinter::emitCFIInstruction(const MachineInstr &MI) {
  ExceptionHandling ExceptionHandlingType = MAI->getExceptionHandlingType();
  if (!usesCFIWithoutEH() &&
      ExceptionHandlingType != ExceptionHandling::DwarfCFI &&
      ExceptionHandlingType != ExceptionHandling::ARM)
    return;

  if (getFunctionCFISectionType(*MF) == CFISection::None)
    return;

  // If there is no "real" instruction following this CFI instruction, skip
  // emitting it; it would be beyond the end of the function's FDE range.
  auto *MBB = MI.getParent();
  auto I = std::next(MI.getIterator());
  while (I != MBB->end() && I->isTransient())
    ++I;
  if (I == MBB->instr_end() &&
      MBB->getReverseIterator() == MBB->getParent()->rbegin())
    return;

  const std::vector<MCCFIInstruction> &Instrs = MF->getFrameInstructions();
  unsigned CFIIndex = MI.getOperand(0).getCFIIndex();
  const MCCFIInstruction &CFI = Instrs[CFIIndex];
  emitCFIInstruction(CFI);
}

void AsmPrinter::emitFrameAlloc(const MachineInstr &MI) {
  // The operands are the MCSymbol and the frame offset of the allocation.
  MCSymbol *FrameAllocSym = MI.getOperand(0).getMCSymbol();
  int FrameOffset = MI.getOperand(1).getImm();

  // Emit a symbol assignment.
  OutStreamer->emitAssignment(FrameAllocSym,
                             MCConstantExpr::create(FrameOffset, OutContext));
}

/// Returns the BB metadata to be emitted in the SHT_LLVM_BB_ADDR_MAP section
/// for a given basic block. This can be used to capture more precise profile
/// information.
static uint32_t getBBAddrMapMetadata(const MachineBasicBlock &MBB) {
  const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
  return object::BBAddrMap::BBEntry::Metadata{
      MBB.isReturnBlock(), !MBB.empty() && TII->isTailCall(MBB.back()),
      MBB.isEHPad(), const_cast<MachineBasicBlock &>(MBB).canFallThrough(),
      !MBB.empty() && MBB.rbegin()->isIndirectBranch()}
      .encode();
}

void AsmPrinter::emitBBAddrMapSection(const MachineFunction &MF) {
  MCSection *BBAddrMapSection =
      getObjFileLowering().getBBAddrMapSection(*MF.getSection());
  assert(BBAddrMapSection && ".llvm_bb_addr_map section is not initialized.");

  const MCSymbol *FunctionSymbol = getFunctionBegin();

  OutStreamer->pushSection();
  OutStreamer->switchSection(BBAddrMapSection);
  OutStreamer->AddComment("version");
  uint8_t BBAddrMapVersion = OutStreamer->getContext().getBBAddrMapVersion();
  OutStreamer->emitInt8(BBAddrMapVersion);
  OutStreamer->AddComment("feature");
  OutStreamer->emitInt8(0);
  OutStreamer->AddComment("function address");
  OutStreamer->emitSymbolValue(FunctionSymbol, getPointerSize());
  OutStreamer->AddComment("number of basic blocks");
  OutStreamer->emitULEB128IntValue(MF.size());
  const MCSymbol *PrevMBBEndSymbol = FunctionSymbol;
  // Emit BB Information for each basic block in the function.
  for (const MachineBasicBlock &MBB : MF) {
    const MCSymbol *MBBSymbol =
        MBB.isEntryBlock() ? FunctionSymbol : MBB.getSymbol();
    // TODO: Remove this check when version 1 is deprecated.
    if (BBAddrMapVersion > 1) {
      OutStreamer->AddComment("BB id");
      // Emit the BB ID for this basic block.
      OutStreamer->emitULEB128IntValue(*MBB.getBBID());
    }
    // Emit the basic block offset relative to the end of the previous block.
    // This is zero unless the block is padded due to alignment.
    emitLabelDifferenceAsULEB128(MBBSymbol, PrevMBBEndSymbol);
    // Emit the basic block size. When BBs have alignments, their size cannot
    // always be computed from their offsets.
    emitLabelDifferenceAsULEB128(MBB.getEndSymbol(), MBBSymbol);
    // Emit the Metadata.
    OutStreamer->emitULEB128IntValue(getBBAddrMapMetadata(MBB));
    PrevMBBEndSymbol = MBB.getEndSymbol();
  }
  OutStreamer->popSection();
}

void AsmPrinter::emitKCFITrapEntry(const MachineFunction &MF,
                                   const MCSymbol *Symbol) {
  MCSection *Section =
      getObjFileLowering().getKCFITrapSection(*MF.getSection());
  if (!Section)
    return;

  OutStreamer->pushSection();
  OutStreamer->switchSection(Section);

  MCSymbol *Loc = OutContext.createLinkerPrivateTempSymbol();
  OutStreamer->emitLabel(Loc);
  OutStreamer->emitAbsoluteSymbolDiff(Symbol, Loc, 4);

  OutStreamer->popSection();
}

void AsmPrinter::emitKCFITypeId(const MachineFunction &MF) {
  const Function &F = MF.getFunction();
  if (const MDNode *MD = F.getMetadata(LLVMContext::MD_kcfi_type))
    emitGlobalConstant(F.getParent()->getDataLayout(),
                       mdconst::extract<ConstantInt>(MD->getOperand(0)), 0);
}

void AsmPrinter::emitPseudoProbe(const MachineInstr &MI) {
  if (PP) {
    auto GUID = MI.getOperand(0).getImm();
    auto Index = MI.getOperand(1).getImm();
    auto Type = MI.getOperand(2).getImm();
    auto Attr = MI.getOperand(3).getImm();
    DILocation *DebugLoc = MI.getDebugLoc();
    PP->emitPseudoProbe(GUID, Index, Type, Attr, DebugLoc);
  }
}

void AsmPrinter::emitStackSizeSection(const MachineFunction &MF) {
  if (!MF.getTarget().Options.EmitStackSizeSection)
    return;

  MCSection *StackSizeSection =
      getObjFileLowering().getStackSizesSection(*getCurrentSection());
  if (!StackSizeSection)
    return;

  const MachineFrameInfo &FrameInfo = MF.getFrameInfo();
  // Don't emit functions with dynamic stack allocations.
  if (FrameInfo.hasVarSizedObjects())
    return;

  OutStreamer->pushSection();
  OutStreamer->switchSection(StackSizeSection);

  const MCSymbol *FunctionSymbol = getFunctionBegin();
  uint64_t StackSize =
      FrameInfo.getStackSize() + FrameInfo.getUnsafeStackSize();
  OutStreamer->emitSymbolValue(FunctionSymbol, TM.getProgramPointerSize());
  OutStreamer->emitULEB128IntValue(StackSize);

  OutStreamer->popSection();
}

void AsmPrinter::emitStackUsage(const MachineFunction &MF) {
  const std::string &OutputFilename = MF.getTarget().Options.StackUsageOutput;

  // OutputFilename empty implies -fstack-usage is not passed.
  if (OutputFilename.empty())
    return;

  const MachineFrameInfo &FrameInfo = MF.getFrameInfo();
  uint64_t StackSize =
      FrameInfo.getStackSize() + FrameInfo.getUnsafeStackSize();

  if (StackUsageStream == nullptr) {
    std::error_code EC;
    StackUsageStream =
        std::make_unique<raw_fd_ostream>(OutputFilename, EC, sys::fs::OF_Text);
    if (EC) {
      errs() << "Could not open file: " << EC.message();
      return;
    }
  }

  *StackUsageStream << MF.getFunction().getParent()->getName();
  if (const DISubprogram *DSP = MF.getFunction().getSubprogram())
    *StackUsageStream << ':' << DSP->getLine();

  *StackUsageStream << ':' << MF.getName() << '\t' << StackSize << '\t';
  if (FrameInfo.hasVarSizedObjects())
    *StackUsageStream << "dynamic\n";
  else
    *StackUsageStream << "static\n";
}

void AsmPrinter::emitPCSectionsLabel(const MachineFunction &MF,
                                     const MDNode &MD) {
  MCSymbol *S = MF.getContext().createTempSymbol("pcsection");
  OutStreamer->emitLabel(S);
  PCSectionsSymbols[&MD].emplace_back(S);
}

void AsmPrinter::emitPCSections(const MachineFunction &MF) {
  const Function &F = MF.getFunction();
  if (PCSectionsSymbols.empty() && !F.hasMetadata(LLVMContext::MD_pcsections))
    return;

  const CodeModel::Model CM = MF.getTarget().getCodeModel();
  const unsigned RelativeRelocSize =
      (CM == CodeModel::Medium || CM == CodeModel::Large) ? getPointerSize()
                                                          : 4;

  // Switch to PCSection, short-circuiting the common case where the current
  // section is still valid (assume most MD_pcsections contain just 1 section).
  auto SwitchSection = [&, Prev = StringRef()](const StringRef &Sec) mutable {
    if (Sec == Prev)
      return;
    MCSection *S = getObjFileLowering().getPCSection(Sec, MF.getSection());
    assert(S && "PC section is not initialized");
    OutStreamer->switchSection(S);
    Prev = Sec;
  };
  // Emit symbols into sections and data as specified in the pcsections MDNode.
  auto EmitForMD = [&](const MDNode &MD, ArrayRef<const MCSymbol *> Syms,
                       bool Deltas) {
    // Expect the first operand to be a section name. After that, a tuple of
    // constants may appear, which will simply be emitted into the current
    // section (the user of MD_pcsections decides the format of encoded data).
    assert(isa<MDString>(MD.getOperand(0)) && "first operand not a string");
    bool ConstULEB128 = false;
    for (const MDOperand &MDO : MD.operands()) {
      if (auto *S = dyn_cast<MDString>(MDO)) {
        // Found string, start of new section!
        // Find options for this section "<section>!<opts>" - supported options:
        //   C = Compress constant integers of size 2-8 bytes as ULEB128.
        const StringRef SecWithOpt = S->getString();
        const size_t OptStart = SecWithOpt.find('!'); // likely npos
        const StringRef Sec = SecWithOpt.substr(0, OptStart);
        const StringRef Opts = SecWithOpt.substr(OptStart); // likely empty
        ConstULEB128 = Opts.find('C') != StringRef::npos;
#ifndef NDEBUG
        for (char O : Opts)
          assert((O == '!' || O == 'C') && "Invalid !pcsections options");
#endif
        SwitchSection(Sec);
        const MCSymbol *Prev = Syms.front();
        for (const MCSymbol *Sym : Syms) {
          if (Sym == Prev || !Deltas) {
            // Use the entry itself as the base of the relative offset.
            MCSymbol *Base = MF.getContext().createTempSymbol("pcsection_base");
            OutStreamer->emitLabel(Base);
            // Emit relative relocation `addr - base`, which avoids a dynamic
            // relocation in the final binary. User will get the address with
            // `base + addr`.
            emitLabelDifference(Sym, Base, RelativeRelocSize);
          } else {
            // Emit delta between symbol and previous symbol.
            if (ConstULEB128)
              emitLabelDifferenceAsULEB128(Sym, Prev);
            else
              emitLabelDifference(Sym, Prev, 4);
          }
          Prev = Sym;
        }
      } else {
        // Emit auxiliary data after PC.
        assert(isa<MDNode>(MDO) && "expecting either string or tuple");
        const auto *AuxMDs = cast<MDNode>(MDO);
        for (const MDOperand &AuxMDO : AuxMDs->operands()) {
          assert(isa<ConstantAsMetadata>(AuxMDO) && "expecting a constant");
          const Constant *C = cast<ConstantAsMetadata>(AuxMDO)->getValue();
          const DataLayout &DL = F.getParent()->getDataLayout();
          const uint64_t Size = DL.getTypeStoreSize(C->getType());

          if (auto *CI = dyn_cast<ConstantInt>(C);
              CI && ConstULEB128 && Size > 1 && Size <= 8) {
            emitULEB128(CI->getZExtValue());
          } else {
            emitGlobalConstant(DL, C, 0);
          }
        }
      }
    }
  };

  OutStreamer->pushSection();
  // Emit PCs for function start and function size.
  if (const MDNode *MD = F.getMetadata(LLVMContext::MD_pcsections))
    EmitForMD(*MD, {getFunctionBegin(), getFunctionEnd()}, true);
  // Emit PCs for instructions collected.
  for (const auto &MS : PCSectionsSymbols)
    EmitForMD(*MS.first, MS.second, false);
  OutStreamer->popSection();
  PCSectionsSymbols.clear();
}

/// Returns true if function begin and end labels should be emitted.
static bool needFuncLabelsForEH(const MachineFunction &MF) {
  if (!MF.getLandingPads().empty() || MF.hasEHFunclets() ||
      MF.getFunction().hasMetadata(LLVMContext::MD_pcsections))
    return true;

  // We might emit an EH table that uses function begin and end labels even if
  // we don't have any landingpads.
  if (!MF.getFunction().hasPersonalityFn())
    return false;
  return !isNoOpWithoutInvoke(
      classifyEHPersonality(MF.getFunction().getPersonalityFn()));
}

static bool needFuncLabels(const MachineFunction &MF){
  return MF.getMMI().hasDebugInfo() || needFuncLabelsForEH(MF);
}

/// EmitFunctionBody - This method emits the body and trailer for a
/// function.
void AsmPrinter::emitFunctionBody() {
  emitFunctionHeader();

  // Emit target-specific gunk before the function body.
  emitFunctionBodyStart();

  if (isVerbose()) {
    // Get MachineDominatorTree or compute it on the fly if it's unavailable
    MDT = getAnalysisIfAvailable<MachineDominatorTree>();
    if (!MDT) {
      OwnedMDT = std::make_unique<MachineDominatorTree>();
      OwnedMDT->getBase().recalculate(*MF);
      MDT = OwnedMDT.get();
    }

    // Get MachineLoopInfo or compute it on the fly if it's unavailable
    MLI = getAnalysisIfAvailable<MachineLoopInfo>();
    if (!MLI) {
      OwnedMLI = std::make_unique<MachineLoopInfo>();
      OwnedMLI->getBase().analyze(MDT->getBase());
      MLI = OwnedMLI.get();
    }
  }

  // Print out code for the function.
  bool HasAnyRealCode = false;
  int NumInstsInFunction = 0;
  bool IsEHa = MMI->getModule()->getModuleFlag("eh-asynch");

  bool CanDoExtraAnalysis = ORE->allowExtraAnalysis(DEBUG_TYPE);
  for (auto &MBB : *MF) {
    // Print a label for the basic block.
    emitBasicBlockStart(MBB);
    DenseMap<StringRef, unsigned> MnemonicCounts;
    for (auto &MI : MBB) {
      // Print the assembly for the instruction.
      if (!MI.isPosition() && !MI.isImplicitDef() && !MI.isKill() &&
          !MI.isDebugInstr()) {
        HasAnyRealCode = true;
        ++NumInstsInFunction;
      }

      // If there is a pre-instruction symbol, emit a label for it here.
      if (MCSymbol *S = MI.getPreInstrSymbol())
        OutStreamer->emitLabel(S);

      if (MDNode *MD = MI.getPCSections())
        emitPCSectionsLabel(*MF, *MD);

      for (const HandlerInfo &HI : Handlers) {
        NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                           HI.TimerGroupDescription, TimePassesIsEnabled);
        HI.Handler->beginInstruction(&MI);
      }

      if (isVerbose())
        emitComments(MI, OutStreamer->getCommentOS());

      switch (MI.getOpcode()) {
      case TargetOpcode::CFI_INSTRUCTION:
        emitCFIInstruction(MI);
        break;
      case TargetOpcode::LOCAL_ESCAPE:
        emitFrameAlloc(MI);
        break;
      case TargetOpcode::ANNOTATION_LABEL:
      case TargetOpcode::GC_LABEL:
        OutStreamer->emitLabel(MI.getOperand(0).getMCSymbol());
        break;
      case TargetOpcode::EH_LABEL:
        OutStreamer->emitLabel(MI.getOperand(0).getMCSymbol());
        // For AsynchEH, insert a Nop if followed by a trap inst
        //   Or the exception won't be caught.
        //   (see MCConstantExpr::create(1,..) in WinException.cpp)
        //  Ignore SDiv/UDiv because a DIV with Const-0 divisor
        //    must have being turned into an UndefValue.
        //  Div with variable opnds won't be the first instruction in
        //  an EH region as it must be led by at least a Load
        {
          auto MI2 = std::next(MI.getIterator());
          if (IsEHa && MI2 != MBB.end() &&
              (MI2->mayLoadOrStore() || MI2->mayRaiseFPException()))
            emitNops(1);
        }
        break;
      case TargetOpcode::INLINEASM:
      case TargetOpcode::INLINEASM_BR:
        emitInlineAsm(&MI);
        break;
      case TargetOpcode::DBG_VALUE:
      case TargetOpcode::DBG_VALUE_LIST:
        if (isVerbose()) {
          if (!emitDebugValueComment(&MI, *this))
            emitInstruction(&MI);
        }
        break;
      case TargetOpcode::DBG_INSTR_REF:
        // This instruction reference will have been resolved to a machine
        // location, and a nearby DBG_VALUE created. We can safely ignore
        // the instruction reference.
        break;
      case TargetOpcode::DBG_PHI:
        // This instruction is only used to label a program point, it's purely
        // meta information.
        break;
      case TargetOpcode::DBG_LABEL:
        if (isVerbose()) {
          if (!emitDebugLabelComment(&MI, *this))
            emitInstruction(&MI);
        }
        break;
      case TargetOpcode::IMPLICIT_DEF:
        if (isVerbose()) emitImplicitDef(&MI);
        break;
      case TargetOpcode::KILL:
        if (isVerbose()) emitKill(&MI, *this);
        break;
      case TargetOpcode::PSEUDO_PROBE:
        emitPseudoProbe(MI);
        break;
      case TargetOpcode::ARITH_FENCE:
        if (isVerbose())
          OutStreamer->emitRawComment("ARITH_FENCE");
        break;
      case TargetOpcode::MEMBARRIER:
        OutStreamer->emitRawComment("MEMBARRIER");
        break;
      default:
        emitInstruction(&MI);
        if (CanDoExtraAnalysis) {
          MCInst MCI;
          MCI.setOpcode(MI.getOpcode());
          auto Name = OutStreamer->getMnemonic(MCI);
          auto I = MnemonicCounts.insert({Name, 0u});
          I.first->second++;
        }
        break;
      }

      // If there is a post-instruction symbol, emit a label for it here.
      if (MCSymbol *S = MI.getPostInstrSymbol())
        OutStreamer->emitLabel(S);

      for (const HandlerInfo &HI : Handlers) {
        NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                           HI.TimerGroupDescription, TimePassesIsEnabled);
        HI.Handler->endInstruction();
      }
    }

    // We must emit temporary symbol for the end of this basic block, if either
    // we have BBLabels enabled or if this basic blocks marks the end of a
    // section.
    if (MF->hasBBLabels() ||
        (MAI->hasDotTypeDotSizeDirective() && MBB.isEndSection()))
      OutStreamer->emitLabel(MBB.getEndSymbol());

    if (MBB.isEndSection()) {
      // The size directive for the section containing the entry block is
      // handled separately by the function section.
      if (!MBB.sameSection(&MF->front())) {
        if (MAI->hasDotTypeDotSizeDirective()) {
          // Emit the size directive for the basic block section.
          const MCExpr *SizeExp = MCBinaryExpr::createSub(
              MCSymbolRefExpr::create(MBB.getEndSymbol(), OutContext),
              MCSymbolRefExpr::create(CurrentSectionBeginSym, OutContext),
              OutContext);
          OutStreamer->emitELFSize(CurrentSectionBeginSym, SizeExp);
        }
        MBBSectionRanges[MBB.getSectionIDNum()] =
            MBBSectionRange{CurrentSectionBeginSym, MBB.getEndSymbol()};
      }
    }
    emitBasicBlockEnd(MBB);

    if (CanDoExtraAnalysis) {
      // Skip empty blocks.
      if (MBB.empty())
        continue;

      MachineOptimizationRemarkAnalysis R(DEBUG_TYPE, "InstructionMix",
                                          MBB.begin()->getDebugLoc(), &MBB);

      // Generate instruction mix remark. First, sort counts in descending order
      // by count and name.
      SmallVector<std::pair<StringRef, unsigned>, 128> MnemonicVec;
      for (auto &KV : MnemonicCounts)
        MnemonicVec.emplace_back(KV.first, KV.second);

      sort(MnemonicVec, [](const std::pair<StringRef, unsigned> &A,
                           const std::pair<StringRef, unsigned> &B) {
        if (A.second > B.second)
          return true;
        if (A.second == B.second)
          return StringRef(A.first) < StringRef(B.first);
        return false;
      });
      R << "BasicBlock: " << ore::NV("BasicBlock", MBB.getName()) << "\n";
      for (auto &KV : MnemonicVec) {
        auto Name = (Twine("INST_") + getToken(KV.first.trim()).first).str();
        R << KV.first << ": " << ore::NV(Name, KV.second) << "\n";
      }
      ORE->emit(R);
    }
  }

  EmittedInsts += NumInstsInFunction;
  MachineOptimizationRemarkAnalysis R(DEBUG_TYPE, "InstructionCount",
                                      MF->getFunction().getSubprogram(),
                                      &MF->front());
  R << ore::NV("NumInstructions", NumInstsInFunction)
    << " instructions in function";
  ORE->emit(R);

  // If the function is empty and the object file uses .subsections_via_symbols,
  // then we need to emit *something* to the function body to prevent the
  // labels from collapsing together.  Just emit a noop.
  // Similarly, don't emit empty functions on Windows either. It can lead to
  // duplicate entries (two functions with the same RVA) in the Guard CF Table
  // after linking, causing the kernel not to load the binary:
  // https://developercommunity.visualstudio.com/content/problem/45366/vc-linker-creates-invalid-dll-with-clang-cl.html
  // FIXME: Hide this behind some API in e.g. MCAsmInfo or MCTargetStreamer.
  const Triple &TT = TM.getTargetTriple();
  if (!HasAnyRealCode && (MAI->hasSubsectionsViaSymbols() ||
                          (TT.isOSWindows() && TT.isOSBinFormatCOFF()))) {
    MCInst Noop = MF->getSubtarget().getInstrInfo()->getNop();

    // Targets can opt-out of emitting the noop here by leaving the opcode
    // unspecified.
    if (Noop.getOpcode()) {
      OutStreamer->AddComment("avoids zero-length function");
      emitNops(1);
    }
  }

  // Switch to the original section in case basic block sections was used.
  OutStreamer->switchSection(MF->getSection());

  const Function &F = MF->getFunction();
  for (const auto &BB : F) {
    if (!BB.hasAddressTaken())
      continue;
    MCSymbol *Sym = GetBlockAddressSymbol(&BB);
    if (Sym->isDefined())
      continue;
    OutStreamer->AddComment("Address of block that was removed by CodeGen");
    OutStreamer->emitLabel(Sym);
  }

  // Emit target-specific gunk after the function body.
  emitFunctionBodyEnd();

  // Even though wasm supports .type and .size in general, function symbols
  // are automatically sized.
  bool EmitFunctionSize = MAI->hasDotTypeDotSizeDirective() && !TT.isWasm();

  if (needFuncLabels(*MF) || EmitFunctionSize) {
    // Create a symbol for the end of function.
    CurrentFnEnd = createTempSymbol("func_end");
    OutStreamer->emitLabel(CurrentFnEnd);
  }

  // If the target wants a .size directive for the size of the function, emit
  // it.
  if (EmitFunctionSize) {
    // We can get the size as difference between the function label and the
    // temp label.
    const MCExpr *SizeExp = MCBinaryExpr::createSub(
        MCSymbolRefExpr::create(CurrentFnEnd, OutContext),
        MCSymbolRefExpr::create(CurrentFnSymForSize, OutContext), OutContext);
    OutStreamer->emitELFSize(CurrentFnSym, SizeExp);
    if (CurrentFnBeginLocal)
      OutStreamer->emitELFSize(CurrentFnBeginLocal, SizeExp);
  }

  // Call endBasicBlockSection on the last block now, if it wasn't already
  // called.
  if (!MF->back().isEndSection()) {
    for (const HandlerInfo &HI : Handlers) {
      NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                         HI.TimerGroupDescription, TimePassesIsEnabled);
      HI.Handler->endBasicBlockSection(MF->back());
    }
  }
  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                       HI.TimerGroupDescription, TimePassesIsEnabled);
    HI.Handler->markFunctionEnd();
  }

  MBBSectionRanges[MF->front().getSectionIDNum()] =
      MBBSectionRange{CurrentFnBegin, CurrentFnEnd};

  // Print out jump tables referenced by the function.
  emitJumpTableInfo();

  // Emit post-function debug and/or EH information.
  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                       HI.TimerGroupDescription, TimePassesIsEnabled);
    HI.Handler->endFunction(MF);
  }

  // Emit section containing BB address offsets and their metadata, when
  // BB labels are requested for this function. Skip empty functions.
  if (MF->hasBBLabels() && HasAnyRealCode)
    emitBBAddrMapSection(*MF);

  // Emit sections containing instruction and function PCs.
  emitPCSections(*MF);

  // Emit section containing stack size metadata.
  emitStackSizeSection(*MF);

  // Emit .su file containing function stack size information.
  emitStackUsage(*MF);

  emitPatchableFunctionEntries();

  if (isVerbose())
    OutStreamer->getCommentOS() << "-- End function\n";

  OutStreamer->addBlankLine();

  // Output MBB ids, function names, and frequencies if the flag to dump
  // MBB profile information has been set
  if (MBBProfileDumpFileOutput) {
    if (!MF->hasBBLabels())
      MF->getContext().reportError(
          SMLoc(),
          "Unable to find BB labels for MBB profile dump. -mbb-profile-dump "
          "must be called with -basic-block-sections=labels");
    MachineBlockFrequencyInfo &MBFI =
        getAnalysis<LazyMachineBlockFrequencyInfoPass>().getBFI();
    for (const auto &MBB : *MF) {
      *MBBProfileDumpFileOutput.get()
          << MF->getName() << "," << MBB.getBBID() << ","
          << MBFI.getBlockFreqRelativeToEntryBlock(&MBB) << "\n";
    }
  }
}

/// Compute the number of Global Variables that uses a Constant.
static unsigned getNumGlobalVariableUses(const Constant *C) {
  if (!C)
    return 0;

  if (isa<GlobalVariable>(C))
    return 1;

  unsigned NumUses = 0;
  for (const auto *CU : C->users())
    NumUses += getNumGlobalVariableUses(dyn_cast<Constant>(CU));

  return NumUses;
}

/// Only consider global GOT equivalents if at least one user is a
/// cstexpr inside an initializer of another global variables. Also, don't
/// handle cstexpr inside instructions. During global variable emission,
/// candidates are skipped and are emitted later in case at least one cstexpr
/// isn't replaced by a PC relative GOT entry access.
static bool isGOTEquivalentCandidate(const GlobalVariable *GV,
                                     unsigned &NumGOTEquivUsers) {
  // Global GOT equivalents are unnamed private globals with a constant
  // pointer initializer to another global symbol. They must point to a
  // GlobalVariable or Function, i.e., as GlobalValue.
  if (!GV->hasGlobalUnnamedAddr() || !GV->hasInitializer() ||
      !GV->isConstant() || !GV->isDiscardableIfUnused() ||
      !isa<GlobalValue>(GV->getOperand(0)))
    return false;

  // To be a got equivalent, at least one of its users need to be a constant
  // expression used by another global variable.
  for (const auto *U : GV->users())
    NumGOTEquivUsers += getNumGlobalVariableUses(dyn_cast<Constant>(U));

  return NumGOTEquivUsers > 0;
}

/// Unnamed constant global variables solely contaning a pointer to
/// another globals variable is equivalent to a GOT table entry; it contains the
/// the address of another symbol. Optimize it and replace accesses to these
/// "GOT equivalents" by using the GOT entry for the final global instead.
/// Compute GOT equivalent candidates among all global variables to avoid
/// emitting them if possible later on, after it use is replaced by a GOT entry
/// access.
void AsmPrinter::computeGlobalGOTEquivs(Module &M) {
  if (!getObjFileLowering().supportIndirectSymViaGOTPCRel())
    return;

  for (const auto &G : M.globals()) {
    unsigned NumGOTEquivUsers = 0;
    if (!isGOTEquivalentCandidate(&G, NumGOTEquivUsers))
      continue;

    const MCSymbol *GOTEquivSym = getSymbol(&G);
    GlobalGOTEquivs[GOTEquivSym] = std::make_pair(&G, NumGOTEquivUsers);
  }
}

/// Constant expressions using GOT equivalent globals may not be eligible
/// for PC relative GOT entry conversion, in such cases we need to emit such
/// globals we previously omitted in EmitGlobalVariable.
void AsmPrinter::emitGlobalGOTEquivs() {
  if (!getObjFileLowering().supportIndirectSymViaGOTPCRel())
    return;

  SmallVector<const GlobalVariable *, 8> FailedCandidates;
  for (auto &I : GlobalGOTEquivs) {
    const GlobalVariable *GV = I.second.first;
    unsigned Cnt = I.second.second;
    if (Cnt)
      FailedCandidates.push_back(GV);
  }
  GlobalGOTEquivs.clear();

  for (const auto *GV : FailedCandidates)
    emitGlobalVariable(GV);
}

void AsmPrinter::emitGlobalAlias(Module &M, const GlobalAlias &GA) {
  MCSymbol *Name = getSymbol(&GA);
  bool IsFunction = GA.getValueType()->isFunctionTy();
  // Treat bitcasts of functions as functions also. This is important at least
  // on WebAssembly where object and function addresses can't alias each other.
  if (!IsFunction)
    IsFunction = isa<Function>(GA.getAliasee()->stripPointerCasts());

  // AIX's assembly directive `.set` is not usable for aliasing purpose,
  // so AIX has to use the extra-label-at-definition strategy. At this
  // point, all the extra label is emitted, we just have to emit linkage for
  // those labels.
  if (TM.getTargetTriple().isOSBinFormatXCOFF()) {
    assert(MAI->hasVisibilityOnlyWithLinkage() &&
           "Visibility should be handled with emitLinkage() on AIX.");

    // Linkage for alias of global variable has been emitted.
    if (isa<GlobalVariable>(GA.getAliaseeObject()))
      return;

    emitLinkage(&GA, Name);
    // If it's a function, also emit linkage for aliases of function entry
    // point.
    if (IsFunction)
      emitLinkage(&GA,
                  getObjFileLowering().getFunctionEntryPointSymbol(&GA, TM));
    return;
  }

  if (GA.hasExternalLinkage() || !MAI->getWeakRefDirective())
    OutStreamer->emitSymbolAttribute(Name, MCSA_Global);
  else if (GA.hasWeakLinkage() || GA.hasLinkOnceLinkage())
    OutStreamer->emitSymbolAttribute(Name, MCSA_WeakReference);
  else
    assert(GA.hasLocalLinkage() && "Invalid alias linkage");

  // Set the symbol type to function if the alias has a function type.
  // This affects codegen when the aliasee is not a function.
  if (IsFunction) {
    OutStreamer->emitSymbolAttribute(Name, MCSA_ELF_TypeFunction);
    if (TM.getTargetTriple().isOSBinFormatCOFF()) {
      OutStreamer->beginCOFFSymbolDef(Name);
      OutStreamer->emitCOFFSymbolStorageClass(
          GA.hasLocalLinkage() ? COFF::IMAGE_SYM_CLASS_STATIC
                               : COFF::IMAGE_SYM_CLASS_EXTERNAL);
      OutStreamer->emitCOFFSymbolType(COFF::IMAGE_SYM_DTYPE_FUNCTION
                                      << COFF::SCT_COMPLEX_TYPE_SHIFT);
      OutStreamer->endCOFFSymbolDef();
    }
  }

  emitVisibility(Name, GA.getVisibility());

  const MCExpr *Expr = lowerConstant(GA.getAliasee());

  if (MAI->hasAltEntry() && isa<MCBinaryExpr>(Expr))
    OutStreamer->emitSymbolAttribute(Name, MCSA_AltEntry);

  // Emit the directives as assignments aka .set:
  OutStreamer->emitAssignment(Name, Expr);
  MCSymbol *LocalAlias = getSymbolPreferLocal(GA);
  if (LocalAlias != Name)
    OutStreamer->emitAssignment(LocalAlias, Expr);

  // If the aliasee does not correspond to a symbol in the output, i.e. the
  // alias is not of an object or the aliased object is private, then set the
  // size of the alias symbol from the type of the alias. We don't do this in
  // other situations as the alias and aliasee having differing types but same
  // size may be intentional.
  const GlobalObject *BaseObject = GA.getAliaseeObject();
  const MCExpr *SizeExpr = nullptr;
  int64_t Offset = 0;
  const DataLayout &DL = M.getDataLayout();
  if (MAI->hasDotTypeDotSizeDirective() && GA.getValueType()->isSized() &&
      (!BaseObject || BaseObject->hasPrivateLinkage())) {
    uint64_t Size = DL.getTypeAllocSize(GA.getValueType());
    OutStreamer->emitELFSize(Name, MCConstantExpr::create(Size, OutContext));
  } else if (GetPointerBaseWithConstantOffset(GA.getAliasee(), Offset, DL,
                                              false) == BaseObject) {
    // If the base symbol has a defined ELF size and we are defining a simple
    // alias (no non-zero GEPs), we also apply that size to the alias symbol.
    // This is required for architectures that set bounds on globals (e.g.
    // CHERI) and use the st_size information for those bounds.
    if (auto *BaseSymELF = dyn_cast<MCSymbolELF>(getSymbol(BaseObject)))
      SizeExpr = BaseSymELF->getSize();
    if (SizeExpr && Offset != 0) {
      int64_t AbsSize = 0;
      if (SizeExpr->evaluateAsAbsolute(AbsSize)) {
        SizeExpr = MCConstantExpr::create(AbsSize - Offset, OutContext);
      } else {
        SizeExpr = MCBinaryExpr::createSub(
            SizeExpr, MCConstantExpr::create(Offset, OutContext), OutContext);
      }
    }
  }
  if (SizeExpr) {
    OutStreamer->emitELFSize(Name, SizeExpr);
  }
}

void AsmPrinter::emitGlobalIFunc(Module &M, const GlobalIFunc &GI) {
  assert(!TM.getTargetTriple().isOSBinFormatXCOFF() &&
         "IFunc is not supported on AIX.");

  MCSymbol *Name = getSymbol(&GI);

  if (GI.hasExternalLinkage() || !MAI->getWeakRefDirective())
    OutStreamer->emitSymbolAttribute(Name, MCSA_Global);
  else if (GI.hasWeakLinkage() || GI.hasLinkOnceLinkage())
    OutStreamer->emitSymbolAttribute(Name, MCSA_WeakReference);
  else
    assert(GI.hasLocalLinkage() && "Invalid ifunc linkage");

  OutStreamer->emitSymbolAttribute(Name, MCSA_ELF_TypeIndFunction);
  emitVisibility(Name, GI.getVisibility());

  // Emit the directives as assignments aka .set:
  const MCExpr *Expr = lowerConstant(GI.getResolver());
  OutStreamer->emitAssignment(Name, Expr);
  MCSymbol *LocalAlias = getSymbolPreferLocal(GI);
  if (LocalAlias != Name)
    OutStreamer->emitAssignment(LocalAlias, Expr);
}

void AsmPrinter::emitRemarksSection(remarks::RemarkStreamer &RS) {
  if (!RS.needsSection())
    return;

  remarks::RemarkSerializer &RemarkSerializer = RS.getSerializer();

  std::optional<SmallString<128>> Filename;
  if (std::optional<StringRef> FilenameRef = RS.getFilename()) {
    Filename = *FilenameRef;
    sys::fs::make_absolute(*Filename);
    assert(!Filename->empty() && "The filename can't be empty.");
  }

  std::string Buf;
  raw_string_ostream OS(Buf);
  std::unique_ptr<remarks::MetaSerializer> MetaSerializer =
      Filename ? RemarkSerializer.metaSerializer(OS, Filename->str())
               : RemarkSerializer.metaSerializer(OS);
  MetaSerializer->emit();

  // Switch to the remarks section.
  MCSection *RemarksSection =
      OutContext.getObjectFileInfo()->getRemarksSection();
  OutStreamer->switchSection(RemarksSection);

  OutStreamer->emitBinaryData(OS.str());
}

bool AsmPrinter::doFinalization(Module &M) {
  // Set the MachineFunction to nullptr so that we can catch attempted
  // accesses to MF specific features at the module level and so that
  // we can conditionalize accesses based on whether or not it is nullptr.
  MF = nullptr;

  // Gather all GOT equivalent globals in the module. We really need two
  // passes over the globals: one to compute and another to avoid its emission
  // in EmitGlobalVariable, otherwise we would not be able to handle cases
  // where the got equivalent shows up before its use.
  computeGlobalGOTEquivs(M);

  // Emit global variables.
  for (const auto &G : M.globals())
    emitGlobalVariable(&G);

  // Emit remaining GOT equivalent globals.
  emitGlobalGOTEquivs();

  const TargetLoweringObjectFile &TLOF = getObjFileLowering();

  // Emit linkage(XCOFF) and visibility info for declarations
  for (const Function &F : M) {
    if (!F.isDeclarationForLinker())
      continue;

    MCSymbol *Name = getSymbol(&F);
    // Function getSymbol gives us the function descriptor symbol for XCOFF.

    if (!TM.getTargetTriple().isOSBinFormatXCOFF()) {
      GlobalValue::VisibilityTypes V = F.getVisibility();
      if (V == GlobalValue::DefaultVisibility)
        continue;

      emitVisibility(Name, V, false);
      continue;
    }

    if (F.isIntrinsic())
      continue;

    // Handle the XCOFF case.
    // Variable `Name` is the function descriptor symbol (see above). Get the
    // function entry point symbol.
    MCSymbol *FnEntryPointSym = TLOF.getFunctionEntryPointSymbol(&F, TM);
    // Emit linkage for the function entry point.
    emitLinkage(&F, FnEntryPointSym);

    // Emit linkage for the function descriptor.
    emitLinkage(&F, Name);
  }

  // Emit the remarks section contents.
  // FIXME: Figure out when is the safest time to emit this section. It should
  // not come after debug info.
  if (remarks::RemarkStreamer *RS = M.getContext().getMainRemarkStreamer())
    emitRemarksSection(*RS);

  TLOF.emitModuleMetadata(*OutStreamer, M);

  if (TM.getTargetTriple().isOSBinFormatELF()) {
    MachineModuleInfoELF &MMIELF = MMI->getObjFileInfo<MachineModuleInfoELF>();

    // Output stubs for external and common global variables.
    MachineModuleInfoELF::SymbolListTy Stubs = MMIELF.GetGVStubList();
    if (!Stubs.empty()) {
      OutStreamer->switchSection(TLOF.getDataSection());
      const DataLayout &DL = M.getDataLayout();
      unsigned AS = DL.getProgramAddressSpace();
      unsigned Size = DL.getPointerSize(AS);

      emitAlignment(Align(Size));
      for (const auto &Stub : Stubs) {
        OutStreamer->emitLabel(Stub.first);
        if (DL.isFatPointer(AS))
          OutStreamer->EmitCheriCapability(Stub.second.getPointer(), Size);
        else
          OutStreamer->emitSymbolValue(Stub.second.getPointer(), Size);
      }
    }
  }

  if (TM.getTargetTriple().isOSBinFormatCOFF()) {
    MachineModuleInfoCOFF &MMICOFF =
        MMI->getObjFileInfo<MachineModuleInfoCOFF>();

    // Output stubs for external and common global variables.
    MachineModuleInfoCOFF::SymbolListTy Stubs = MMICOFF.GetGVStubList();
    if (!Stubs.empty()) {
      const DataLayout &DL = M.getDataLayout();

      for (const auto &Stub : Stubs) {
        SmallString<256> SectionName = StringRef(".rdata$");
        SectionName += Stub.first->getName();
        OutStreamer->switchSection(OutContext.getCOFFSection(
            SectionName,
            COFF::IMAGE_SCN_CNT_INITIALIZED_DATA | COFF::IMAGE_SCN_MEM_READ |
                COFF::IMAGE_SCN_LNK_COMDAT,
            SectionKind::getReadOnly(), Stub.first->getName(),
            COFF::IMAGE_COMDAT_SELECT_ANY));
        emitAlignment(Align(DL.getPointerSize()));
        OutStreamer->emitSymbolAttribute(Stub.first, MCSA_Global);
        OutStreamer->emitLabel(Stub.first);
        OutStreamer->emitSymbolValue(Stub.second.getPointer(),
                                     DL.getPointerSize());
      }
    }
  }

  // This needs to happen before emitting debug information since that can end
  // arbitrary sections.
  if (auto *TS = OutStreamer->getTargetStreamer())
    TS->emitConstantPools();

  // Emit Stack maps before any debug info. Mach-O requires that no data or
  // text sections come after debug info has been emitted. This matters for
  // stack maps as they are arbitrary data, and may even have a custom format
  // through user plugins.
  emitStackMaps();

  // Finalize debug and EH information.
  for (const HandlerInfo &HI : Handlers) {
    NamedRegionTimer T(HI.TimerName, HI.TimerDescription, HI.TimerGroupName,
                       HI.TimerGroupDescription, TimePassesIsEnabled);
    HI.Handler->endModule();
  }

  // This deletes all the ephemeral handlers that AsmPrinter added, while
  // keeping all the user-added handlers alive until the AsmPrinter is
  // destroyed.
  Handlers.erase(Handlers.begin() + NumUserHandlers, Handlers.end());
  DD = nullptr;

  // If the target wants to know about weak references, print them all.
  if (MAI->getWeakRefDirective()) {
    // FIXME: This is not lazy, it would be nice to only print weak references
    // to stuff that is actually used.  Note that doing so would require targets
    // to notice uses in operands (due to constant exprs etc).  This should
    // happen with the MC stuff eventually.

    // Print out module-level global objects here.
    for (const auto &GO : M.global_objects()) {
      if (!GO.hasExternalWeakLinkage())
        continue;
      OutStreamer->emitSymbolAttribute(getSymbol(&GO), MCSA_WeakReference);
    }
    if (shouldEmitWeakSwiftAsyncExtendedFramePointerFlags()) {
      auto SymbolName = "swift_async_extendedFramePointerFlags";
      auto Global = M.getGlobalVariable(SymbolName);
      if (!Global) {
        auto Int8PtrTy = Type::getInt8PtrTy(M.getContext());
        Global = new GlobalVariable(M, Int8PtrTy, false,
                                    GlobalValue::ExternalWeakLinkage, nullptr,
                                    SymbolName);
        OutStreamer->emitSymbolAttribute(getSymbol(Global), MCSA_WeakReference);
      }
    }
  }

  // Print aliases in topological order, that is, for each alias a = b,
  // b must be printed before a.
  // This is because on some targets (e.g. PowerPC) linker expects aliases in
  // such an order to generate correct TOC information.
  SmallVector<const GlobalAlias *, 16> AliasStack;
  SmallPtrSet<const GlobalAlias *, 16> AliasVisited;
  for (const auto &Alias : M.aliases()) {
    if (Alias.hasAvailableExternallyLinkage())
      continue;
    for (const GlobalAlias *Cur = &Alias; Cur;
         Cur = dyn_cast<GlobalAlias>(Cur->getAliasee())) {
      if (!AliasVisited.insert(Cur).second)
        break;
      AliasStack.push_back(Cur);
    }
    for (const GlobalAlias *AncestorAlias : llvm::reverse(AliasStack))
      emitGlobalAlias(M, *AncestorAlias);
    AliasStack.clear();
  }
  for (const auto &IFunc : M.ifuncs())
    emitGlobalIFunc(M, IFunc);

  GCModuleInfo *MI = getAnalysisIfAvailable<GCModuleInfo>();
  assert(MI && "AsmPrinter didn't require GCModuleInfo?");
  for (GCModuleInfo::iterator I = MI->end(), E = MI->begin(); I != E; )
    if (GCMetadataPrinter *MP = getOrCreateGCPrinter(**--I))
      MP->finishAssembly(M, *MI, *this);

  // Emit llvm.ident metadata in an '.ident' directive.
  emitModuleIdents(M);

  // Emit bytes for llvm.commandline metadata.
  // The command line metadata is emitted earlier on XCOFF.
  if (!TM.getTargetTriple().isOSBinFormatXCOFF())
    emitModuleCommandLines(M);

  // Emit .note.GNU-split-stack and .note.GNU-no-split-stack sections if
  // split-stack is used.
  if (TM.getTargetTriple().isOSBinFormatELF() && HasSplitStack) {
    OutStreamer->switchSection(OutContext.getELFSection(".note.GNU-split-stack",
                                                        ELF::SHT_PROGBITS, 0));
    if (HasNoSplitStack)
      OutStreamer->switchSection(OutContext.getELFSection(
          ".note.GNU-no-split-stack", ELF::SHT_PROGBITS, 0));
  }

  // If we don't have any trampolines, then we don't require stack memory
  // to be executable. Some targets have a directive to declare this.
  Function *InitTrampolineIntrinsic = M.getFunction("llvm.init.trampoline");
  if (!InitTrampolineIntrinsic || InitTrampolineIntrinsic->use_empty())
    if (MCSection *S = MAI->getNonexecutableStackSection(OutContext))
      OutStreamer->switchSection(S);

  if (TM.Options.EmitAddrsig) {
    // Emit address-significance attributes for all globals.
    OutStreamer->emitAddrsig();
    for (const GlobalValue &GV : M.global_values()) {
      if (!GV.use_empty() && !GV.isThreadLocal() &&
          !GV.hasDLLImportStorageClass() && !GV.getName().startswith("llvm.") &&
          !GV.hasAtLeastLocalUnnamedAddr())
        OutStreamer->emitAddrsigSym(getSymbol(&GV));
    }
  }

  // Emit symbol partition specifications (ELF only).
  if (TM.getTargetTriple().isOSBinFormatELF()) {
    unsigned UniqueID = 0;
    for (const GlobalValue &GV : M.global_values()) {
      if (!GV.hasPartition() || GV.isDeclarationForLinker() ||
          GV.getVisibility() != GlobalValue::DefaultVisibility)
        continue;

      OutStreamer->switchSection(
          OutContext.getELFSection(".llvm_sympart", ELF::SHT_LLVM_SYMPART, 0, 0,
                                   "", false, ++UniqueID, nullptr));
      OutStreamer->emitBytes(GV.getPartition());
      OutStreamer->emitZeros(1);
      OutStreamer->emitValue(
          MCSymbolRefExpr::create(getSymbol(&GV), OutContext),
          MAI->getCodePointerSize());
    }
  }

  // Allow the target to emit any magic that it wants at the end of the file,
  // after everything else has gone out.
  emitEndOfAsmFile(M);

  MMI = nullptr;
  AddrLabelSymbols = nullptr;

  OutStreamer->finish();
  OutStreamer->reset();
  OwnedMLI.reset();
  OwnedMDT.reset();

  return false;
}

MCSymbol *AsmPrinter::getMBBExceptionSym(const MachineBasicBlock &MBB) {
  auto Res = MBBSectionExceptionSyms.try_emplace(MBB.getSectionIDNum());
  if (Res.second)
    Res.first->second = createTempSymbol("exception");
  return Res.first->second;
}

void AsmPrinter::SetupMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  const Function &F = MF.getFunction();

  // Record that there are split-stack functions, so we will emit a special
  // section to tell the linker.
  if (MF.shouldSplitStack()) {
    HasSplitStack = true;

    if (!MF.getFrameInfo().needsSplitStackProlog())
      HasNoSplitStack = true;
  } else
    HasNoSplitStack = true;

  // Get the function symbol.
  if (!MAI->needsFunctionDescriptors()) {
    CurrentFnSym = getSymbol(&MF.getFunction());
  } else {
    assert(TM.getTargetTriple().isOSAIX() &&
           "Only AIX uses the function descriptor hooks.");
    // AIX is unique here in that the name of the symbol emitted for the
    // function body does not have the same name as the source function's
    // C-linkage name.
    assert(CurrentFnDescSym && "The function descriptor symbol needs to be"
                               " initalized first.");

    // Get the function entry point symbol.
    CurrentFnSym = getObjFileLowering().getFunctionEntryPointSymbol(&F, TM);
  }

  CurrentFnSymForSize = CurrentFnSym;
  CurrentFnBegin = nullptr;
  CurrentFnBeginLocal = nullptr;
  CurrentSectionBeginSym = nullptr;
  MBBSectionRanges.clear();
  MBBSectionExceptionSyms.clear();
  bool NeedsLocalForSize = MAI->needsLocalForSize();
  if (F.hasFnAttribute("patchable-function-entry") ||
      F.hasFnAttribute("function-instrument") ||
      F.hasFnAttribute("xray-instruction-threshold") ||
      needFuncLabels(MF) || NeedsLocalForSize ||
      MF.getTarget().Options.EmitStackSizeSection || MF.hasBBLabels()) {
    CurrentFnBegin = createTempSymbol("func_begin");
    if (NeedsLocalForSize)
      CurrentFnSymForSize = CurrentFnBegin;
  }

  ORE = &getAnalysis<MachineOptimizationRemarkEmitterPass>().getORE();
}

namespace {

// Keep track the alignment, constpool entries per Section.
  struct SectionCPs {
    MCSection *S;
    Align Alignment;
    SmallVector<unsigned, 4> CPEs;

    SectionCPs(MCSection *s, Align a) : S(s), Alignment(a) {}
  };

} // end anonymous namespace

/// EmitConstantPool - Print to the current output stream assembly
/// representations of the constants in the constant pool MCP. This is
/// used to print out constants which have been "spilled to memory" by
/// the code generator.
void AsmPrinter::emitConstantPool() {
  const MachineConstantPool *MCP = MF->getConstantPool();
  const std::vector<MachineConstantPoolEntry> &CP = MCP->getConstants();
  if (CP.empty()) return;

  // Calculate sections for constant pool entries. We collect entries to go into
  // the same section together to reduce amount of section switch statements.
  SmallVector<SectionCPs, 4> CPSections;
  for (unsigned i = 0, e = CP.size(); i != e; ++i) {
    const MachineConstantPoolEntry &CPE = CP[i];
    Align Alignment = CPE.getAlign();

    SectionKind Kind = CPE.getSectionKind(&getDataLayout());

    const Constant *C = nullptr;
    if (!CPE.isMachineConstantPoolEntry())
      C = CPE.Val.ConstVal;

    MCSection *S = getObjFileLowering().getSectionForConstant(
        getDataLayout(), Kind, C, Alignment);

    // The number of sections are small, just do a linear search from the
    // last section to the first.
    bool Found = false;
    unsigned SecIdx = CPSections.size();
    while (SecIdx != 0) {
      if (CPSections[--SecIdx].S == S) {
        Found = true;
        break;
      }
    }
    if (!Found) {
      SecIdx = CPSections.size();
      CPSections.push_back(SectionCPs(S, Alignment));
    }

    if (Alignment > CPSections[SecIdx].Alignment)
      CPSections[SecIdx].Alignment = Alignment;
    CPSections[SecIdx].CPEs.push_back(i);
  }

  // Now print stuff into the calculated sections.
  const MCSection *CurSection = nullptr;
  unsigned Offset = 0;
  for (unsigned i = 0, e = CPSections.size(); i != e; ++i) {
    for (unsigned j = 0, ee = CPSections[i].CPEs.size(); j != ee; ++j) {
      unsigned CPI = CPSections[i].CPEs[j];
      MCSymbol *Sym = GetCPISymbol(CPI);
      if (!Sym->isUndefined())
        continue;

      if (CurSection != CPSections[i].S) {
        OutStreamer->switchSection(CPSections[i].S);
        emitAlignment(Align(CPSections[i].Alignment));
        CurSection = CPSections[i].S;
        Offset = 0;
      }

      MachineConstantPoolEntry CPE = CP[CPI];

      // Emit inter-object padding for alignment.
      unsigned NewOffset = alignTo(Offset, CPE.getAlign());
      OutStreamer->emitZeros(NewOffset - Offset);

      const auto Size = CPE.getSizeInBytes(getDataLayout());
      Offset = NewOffset + Size;

      OutStreamer->emitLabel(Sym);
      if (CPE.isMachineConstantPoolEntry())
        emitMachineConstantPoolValue(CPE.Val.MachineCPVal);
      else
        emitGlobalConstant(getDataLayout(), CPE.Val.ConstVal, 0);

      if (MAI->hasDotTypeDotSizeDirective())
        OutStreamer->emitELFSize(Sym, MCConstantExpr::create(Size, OutContext));
    }
  }
}

// Print assembly representations of the jump tables used by the current
// function.
void AsmPrinter::emitJumpTableInfo() {
  const DataLayout &DL = MF->getDataLayout();
  const MachineJumpTableInfo *MJTI = MF->getJumpTableInfo();
  if (!MJTI) return;
  if (MJTI->getEntryKind() == MachineJumpTableInfo::EK_Inline) return;
  const std::vector<MachineJumpTableEntry> &JT = MJTI->getJumpTables();
  if (JT.empty()) return;

  // Pick the directive to use to print the jump table entries, and switch to
  // the appropriate section.
  const Function &F = MF->getFunction();
  const TargetLoweringObjectFile &TLOF = getObjFileLowering();
  bool JTInDiffSection = !TLOF.shouldPutJumpTableInFunctionSection(
      MJTI->getEntryKind() == MachineJumpTableInfo::EK_LabelDifference32,
      F);
  if (JTInDiffSection) {
    // Drop it in the readonly section.
    MCSection *ReadOnlySection = TLOF.getSectionForJumpTable(F, TM);
    OutStreamer->switchSection(ReadOnlySection);
  }

  emitAlignment(Align(MJTI->getEntryAlignment(DL)));

  // Jump tables in code sections are marked with a data_region directive
  // where that's supported.
  if (!JTInDiffSection)
    OutStreamer->emitDataRegion(MCDR_DataRegionJT32);

  for (unsigned JTI = 0, e = JT.size(); JTI != e; ++JTI) {
    const std::vector<MachineBasicBlock*> &JTBBs = JT[JTI].MBBs;

    // If this jump table was deleted, ignore it.
    if (JTBBs.empty()) continue;

    // For the EK_LabelDifference32 entry, if using .set avoids a relocation,
    /// emit a .set directive for each unique entry.
    if (MJTI->getEntryKind() == MachineJumpTableInfo::EK_LabelDifference32 &&
        MAI->doesSetDirectiveSuppressReloc()) {
      SmallPtrSet<const MachineBasicBlock*, 16> EmittedSets;
      const TargetLowering *TLI = MF->getSubtarget().getTargetLowering();
      const MCExpr *Base = TLI->getPICJumpTableRelocBaseExpr(MF,JTI,OutContext);
      for (const MachineBasicBlock *MBB : JTBBs) {
        if (!EmittedSets.insert(MBB).second)
          continue;

        // .set LJTSet, LBB32-base
        const MCExpr *LHS =
          MCSymbolRefExpr::create(MBB->getSymbol(), OutContext);
        OutStreamer->emitAssignment(GetJTSetSymbol(JTI, MBB->getNumber()),
                                    MCBinaryExpr::createSub(LHS, Base,
                                                            OutContext));
      }
    }

    // On some targets (e.g. Darwin) we want to emit two consecutive labels
    // before each jump table.  The first label is never referenced, but tells
    // the assembler and linker the extents of the jump table object.  The
    // second label is actually referenced by the code.
    if (JTInDiffSection && DL.hasLinkerPrivateGlobalPrefix())
      // FIXME: This doesn't have to have any specific name, just any randomly
      // named and numbered local label started with 'l' would work.  Simplify
      // GetJTISymbol.
      OutStreamer->emitLabel(GetJTISymbol(JTI, true));

    MCSymbol* JTISymbol = GetJTISymbol(JTI);
    OutStreamer->emitLabel(JTISymbol);

    for (const MachineBasicBlock *MBB : JTBBs)
      emitJumpTableEntry(MJTI, MBB, JTI);

    if (MAI->hasDotTypeDotSizeDirective()) {
      auto JTEnd = createTempSymbol(JTISymbol->getName() + "_end");
      OutStreamer->emitLabel(JTEnd);
      const MCExpr *SizeExp = MCBinaryExpr::createSub(
        MCSymbolRefExpr::create(JTEnd, OutContext),
        MCSymbolRefExpr::create(JTISymbol, OutContext), OutContext);
      OutStreamer->emitELFSize(JTISymbol, SizeExp);
    }
  }
  if (!JTInDiffSection)
    OutStreamer->emitDataRegion(MCDR_DataRegionEnd);
}

/// EmitJumpTableEntry - Emit a jump table entry for the specified MBB to the
/// current stream.
void AsmPrinter::emitJumpTableEntry(const MachineJumpTableInfo *MJTI,
                                    const MachineBasicBlock *MBB,
                                    unsigned UID) const {
  assert(MBB && MBB->getNumber() >= 0 && "Invalid basic block");
  const MCExpr *Value = nullptr;
  switch (MJTI->getEntryKind()) {
  case MachineJumpTableInfo::EK_Inline:
    llvm_unreachable("Cannot emit EK_Inline jump table entry");
  case MachineJumpTableInfo::EK_Custom32:
    Value = MF->getSubtarget().getTargetLowering()->LowerCustomJumpTableEntry(
        MJTI, MBB, UID, OutContext);
    break;
  case MachineJumpTableInfo::EK_BlockAddress:
    // EK_BlockAddress - Each entry is a plain address of block, e.g.:
    //     .word LBB123
    Value = MCSymbolRefExpr::create(MBB->getSymbol(), OutContext);
    break;
  case MachineJumpTableInfo::EK_GPRel32BlockAddress: {
    // EK_GPRel32BlockAddress - Each entry is an address of block, encoded
    // with a relocation as gp-relative, e.g.:
    //     .gprel32 LBB123
    MCSymbol *MBBSym = MBB->getSymbol();
    OutStreamer->emitGPRel32Value(MCSymbolRefExpr::create(MBBSym, OutContext));
    return;
  }

  case MachineJumpTableInfo::EK_GPRel64BlockAddress: {
    // EK_GPRel64BlockAddress - Each entry is an address of block, encoded
    // with a relocation as gp-relative, e.g.:
    //     .gpdword LBB123
    MCSymbol *MBBSym = MBB->getSymbol();
    OutStreamer->emitGPRel64Value(MCSymbolRefExpr::create(MBBSym, OutContext));
    return;
  }

  case MachineJumpTableInfo::EK_LabelDifference32: {
    // Each entry is the address of the block minus the address of the jump
    // table. This is used for PIC jump tables where gprel32 is not supported.
    // e.g.:
    //      .word LBB123 - LJTI1_2
    // If the .set directive avoids relocations, this is emitted as:
    //      .set L4_5_set_123, LBB123 - LJTI1_2
    //      .word L4_5_set_123
    if (MAI->doesSetDirectiveSuppressReloc()) {
      Value = MCSymbolRefExpr::create(GetJTSetSymbol(UID, MBB->getNumber()),
                                      OutContext);
      break;
    }
    Value = MCSymbolRefExpr::create(MBB->getSymbol(), OutContext);
    const TargetLowering *TLI = MF->getSubtarget().getTargetLowering();
    const MCExpr *Base = TLI->getPICJumpTableRelocBaseExpr(MF, UID, OutContext);
    Value = MCBinaryExpr::createSub(Value, Base, OutContext);
    break;
  }
  }

  assert(Value && "Unknown entry kind!");

  unsigned EntrySize = MJTI->getEntrySize(getDataLayout());
  OutStreamer->emitValue(Value, EntrySize);
}

/// EmitSpecialLLVMGlobal - Check to see if the specified global is a
/// special global used by LLVM.  If so, emit it and return true, otherwise
/// do nothing and return false.
bool AsmPrinter::emitSpecialLLVMGlobal(const GlobalVariable *GV) {
  if (GV->getName() == "llvm.used") {
    if (MAI->hasNoDeadStrip())    // No need to emit this at all.
      emitLLVMUsedList(cast<ConstantArray>(GV->getInitializer()));
    return true;
  }

  // Ignore debug and non-emitted data.  This handles llvm.compiler.used.
  if (GV->getSection() == "llvm.metadata" ||
      GV->hasAvailableExternallyLinkage())
    return true;

  if (!GV->hasAppendingLinkage()) return false;

  assert(GV->hasInitializer() && "Not a special LLVM global!");

  if (GV->getName() == "llvm.global_ctors") {
    emitXXStructorList(GV->getParent()->getDataLayout(), GV->getInitializer(),
                       /* isCtor */ true);

    return true;
  }

  if (GV->getName() == "llvm.global_dtors") {
    emitXXStructorList(GV->getParent()->getDataLayout(), GV->getInitializer(),
                       /* isCtor */ false);

    return true;
  }

  report_fatal_error("unknown special variable");
}

/// EmitLLVMUsedList - For targets that define a MAI::UsedDirective, mark each
/// global in the specified llvm.used list.
void AsmPrinter::emitLLVMUsedList(const ConstantArray *InitList) {
  // Should be an array of 'i8*'.
  for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i) {
    const GlobalValue *GV =
      dyn_cast<GlobalValue>(InitList->getOperand(i)->stripPointerCasts());
    if (GV)
      OutStreamer->emitSymbolAttribute(getSymbol(GV), MCSA_NoDeadStrip);
  }
}

void AsmPrinter::preprocessXXStructorList(const DataLayout &DL,
                                          const Constant *List,
                                          SmallVector<Structor, 8> &Structors) {
  // Should be an array of '{ i32, void ()*, i8* }' structs.  The first value is
  // the init priority.
  if (!isa<ConstantArray>(List))
    return;

  // Gather the structors in a form that's convenient for sorting by priority.
  for (Value *O : cast<ConstantArray>(List)->operands()) {
    auto *CS = cast<ConstantStruct>(O);
    if (CS->getOperand(1)->isNullValue())
      break; // Found a null terminator, skip the rest.
    ConstantInt *Priority = dyn_cast<ConstantInt>(CS->getOperand(0));
    if (!Priority)
      continue; // Malformed.
    Structors.push_back(Structor());
    Structor &S = Structors.back();
    S.Priority = Priority->getLimitedValue(65535);
    S.Func = CS->getOperand(1);
    if (!CS->getOperand(2)->isNullValue()) {
      if (TM.getTargetTriple().isOSAIX())
        llvm::report_fatal_error(
            "associated data of XXStructor list is not yet supported on AIX");
      S.ComdatKey =
          dyn_cast<GlobalValue>(CS->getOperand(2)->stripPointerCasts());
    }
  }

  // Emit the function pointers in the target-specific order
  llvm::stable_sort(Structors, [](const Structor &L, const Structor &R) {
    return L.Priority < R.Priority;
  });
}

/// EmitXXStructorList - Emit the ctor or dtor list taking into account the init
/// priority.
void AsmPrinter::emitXXStructorList(const DataLayout &DL, const Constant *List,
                                    bool IsCtor) {
  SmallVector<Structor, 8> Structors;
  preprocessXXStructorList(DL, List, Structors);
  if (Structors.empty())
    return;

  // Emit the structors in reverse order if we are using the .ctor/.dtor
  // initialization scheme.
  if (!TM.Options.UseInitArray)
    std::reverse(Structors.begin(), Structors.end());

  const Align Align = DL.getPointerPrefAlignment(DL.getProgramAddressSpace());
  for (Structor &S : Structors) {
    const TargetLoweringObjectFile &Obj = getObjFileLowering();
    const MCSymbol *KeySym = nullptr;
    if (GlobalValue *GV = S.ComdatKey) {
      if (GV->isDeclarationForLinker())
        // If the associated variable is not defined in this module
        // (it might be available_externally, or have been an
        // available_externally definition that was dropped by the
        // EliminateAvailableExternally pass), some other TU
        // will provide its dynamic initializer.
        continue;

      KeySym = getSymbol(GV);
    }

    MCSection *OutputSection =
        (IsCtor ? Obj.getStaticCtorSection(S.Priority, KeySym)
                : Obj.getStaticDtorSection(S.Priority, KeySym));
    OutStreamer->switchSection(OutputSection);
    if (OutStreamer->getCurrentSection() != OutStreamer->getPreviousSection())
      emitAlignment(Align);
    emitXXStructor(DL, S.Func);
  }
}

void AsmPrinter::emitModuleIdents(Module &M) {
  if (!MAI->hasIdentDirective())
    return;

  if (const NamedMDNode *NMD = M.getNamedMetadata("llvm.ident")) {
    for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
      const MDNode *N = NMD->getOperand(i);
      assert(N->getNumOperands() == 1 &&
             "llvm.ident metadata entry can have only one operand");
      const MDString *S = cast<MDString>(N->getOperand(0));
      OutStreamer->emitIdent(S->getString());
    }
  }
}

void AsmPrinter::emitModuleCommandLines(Module &M) {
  MCSection *CommandLine = getObjFileLowering().getSectionForCommandLines();
  if (!CommandLine)
    return;

  const NamedMDNode *NMD = M.getNamedMetadata("llvm.commandline");
  if (!NMD || !NMD->getNumOperands())
    return;

  OutStreamer->pushSection();
  OutStreamer->switchSection(CommandLine);
  OutStreamer->emitZeros(1);
  for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
    const MDNode *N = NMD->getOperand(i);
    assert(N->getNumOperands() == 1 &&
           "llvm.commandline metadata entry can have only one operand");
    const MDString *S = cast<MDString>(N->getOperand(0));
    OutStreamer->emitBytes(S->getString());
    OutStreamer->emitZeros(1);
  }
  OutStreamer->popSection();
}

//===--------------------------------------------------------------------===//
// Emission and print routines
//

/// Emit a byte directive and value.
///
void AsmPrinter::emitInt8(int Value) const { OutStreamer->emitInt8(Value); }

/// Emit a short directive and value.
void AsmPrinter::emitInt16(int Value) const { OutStreamer->emitInt16(Value); }

/// Emit a long directive and value.
void AsmPrinter::emitInt32(int Value) const { OutStreamer->emitInt32(Value); }

/// EmitSLEB128 - emit the specified signed leb128 value.
void AsmPrinter::emitSLEB128(int64_t Value, const char *Desc) const {
  if (isVerbose() && Desc)
    OutStreamer->AddComment(Desc);

  OutStreamer->emitSLEB128IntValue(Value);
}

void AsmPrinter::emitULEB128(uint64_t Value, const char *Desc,
                             unsigned PadTo) const {
  if (isVerbose() && Desc)
    OutStreamer->AddComment(Desc);

  OutStreamer->emitULEB128IntValue(Value, PadTo);
}

/// Emit a long long directive and value.
void AsmPrinter::emitInt64(uint64_t Value) const {
  OutStreamer->emitInt64(Value);
}

/// Emit something like ".long Hi-Lo" where the size in bytes of the directive
/// is specified by Size and Hi/Lo specify the labels. This implicitly uses
/// .set if it avoids relocations.
void AsmPrinter::emitLabelDifference(const MCSymbol *Hi, const MCSymbol *Lo,
                                     unsigned Size) const {
  OutStreamer->emitAbsoluteSymbolDiff(Hi, Lo, Size);
}

/// Emit something like ".uleb128 Hi-Lo".
void AsmPrinter::emitLabelDifferenceAsULEB128(const MCSymbol *Hi,
                                              const MCSymbol *Lo) const {
  OutStreamer->emitAbsoluteSymbolDiffAsULEB128(Hi, Lo);
}

/// EmitLabelPlusOffset - Emit something like ".long Label+Offset"
/// where the size in bytes of the directive is specified by Size and Label
/// specifies the label.  This implicitly uses .set if it is available.
void AsmPrinter::emitLabelPlusOffset(const MCSymbol *Label, uint64_t Offset,
                                     unsigned Size,
                                     bool IsSectionRelative) const {
  if (MAI->needsDwarfSectionOffsetDirective() && IsSectionRelative) {
    OutStreamer->emitCOFFSecRel32(Label, Offset);
    if (Size > 4)
      OutStreamer->emitZeros(Size - 4);
    return;
  }

  // Emit Label+Offset (or just Label if Offset is zero)
  const MCExpr *Expr = MCSymbolRefExpr::create(Label, OutContext);
  if (Offset)
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(Offset, OutContext), OutContext);

  OutStreamer->emitValue(Expr, Size);
}

//===----------------------------------------------------------------------===//

// EmitAlignment - Emit an alignment directive to the specified power of
// two boundary.  If a global value is specified, and if that global has
// an explicit alignment requested, it will override the alignment request
// if required for correctness.
void AsmPrinter::emitAlignment(Align Alignment, const GlobalObject *GV,
                               unsigned MaxBytesToEmit) const {
  if (GV)
    Alignment = getGVAlignment(GV, GV->getParent()->getDataLayout(), Alignment);

  if (Alignment == Align(1))
    return; // 1-byte aligned: no need to emit alignment.

  if (getCurrentSection()->getKind().isText()) {
    const MCSubtargetInfo *STI = nullptr;
    if (this->MF)
      STI = &getSubtargetInfo();
    else
      STI = TM.getMCSubtargetInfo();
    OutStreamer->emitCodeAlignment(Alignment, STI, MaxBytesToEmit);
  } else
    OutStreamer->emitValueToAlignment(Alignment, 0, 1, MaxBytesToEmit);
}

//===----------------------------------------------------------------------===//
// Constant emission.
//===----------------------------------------------------------------------===//

const MCExpr *AsmPrinter::lowerConstant(const Constant *CV) {
  MCContext &Ctx = OutContext;

  if (CV->isNullValue() || isa<UndefValue>(CV))
    return MCConstantExpr::create(0, Ctx);

  if (const ConstantInt *CI = dyn_cast<ConstantInt>(CV))
    return MCConstantExpr::create(CI->getZExtValue(), Ctx);

  if (const GlobalValue *GV = dyn_cast<GlobalValue>(CV))
    return MCSymbolRefExpr::create(getSymbol(GV), Ctx);

  if (const BlockAddress *BA = dyn_cast<BlockAddress>(CV))
    return MCSymbolRefExpr::create(GetBlockAddressSymbol(BA), Ctx);

  if (const auto *Equiv = dyn_cast<DSOLocalEquivalent>(CV))
    return getObjFileLowering().lowerDSOLocalEquivalent(Equiv, TM);

  if (const NoCFIValue *NC = dyn_cast<NoCFIValue>(CV))
    return MCSymbolRefExpr::create(getSymbol(NC->getGlobalValue()), Ctx);

  const ConstantExpr *CE = dyn_cast<ConstantExpr>(CV);
  if (!CE) {
    llvm_unreachable("Unknown constant value to lower!");
  }

  // The constant expression opcodes are limited to those that are necessary
  // to represent relocations on supported targets. Expressions involving only
  // constant addresses are constant folded instead.
  switch (CE->getOpcode()) {
  default:
    break; // Error
  case Instruction::AddrSpaceCast: {
    const Constant *Op = CE->getOperand(0);
    unsigned DstAS = CE->getType()->getPointerAddressSpace();
    unsigned SrcAS = Op->getType()->getPointerAddressSpace();
    if (TM.isNoopAddrSpaceCast(SrcAS, DstAS))
      return lowerConstant(Op);
    // We can also lower a global addrspacecast from CHERI cap -> non-cap:
    if (isCheriPointer(DstAS, &getDataLayout()) ||
        isCheriPointer(SrcAS, &getDataLayout()))
      return lowerConstant(Op);

    break; // Error
  }
  case Instruction::GetElementPtr: {
    // Generate a symbolic expression for the byte address
    APInt OffsetAI(getDataLayout().getPointerAddrSizeInBits(CE->getType()), 0);
    cast<GEPOperator>(CE)->accumulateConstantOffset(getDataLayout(), OffsetAI);

    const MCExpr *Base = lowerConstant(CE->getOperand(0));
    if (!OffsetAI)
      return Base;

    int64_t Offset = OffsetAI.getSExtValue();
    return MCBinaryExpr::createAdd(Base, MCConstantExpr::create(Offset, Ctx),
                                   Ctx);
  }

  case Instruction::Trunc:
    // We emit the value and depend on the assembler to truncate the generated
    // expression properly.  This is important for differences between
    // blockaddress labels.  Since the two labels are in the same function, it
    // is reasonable to treat their delta as a 32-bit value.
    [[fallthrough]];
  case Instruction::BitCast:
    return lowerConstant(CE->getOperand(0));

  case Instruction::IntToPtr: {
    const DataLayout &DL = getDataLayout();

    // Handle casts to pointers by changing them into casts to the appropriate
    // integer type.  This promotes constant folding and simplifies this code.
    Constant *Op = CE->getOperand(0);
    Op = ConstantExpr::getIntegerCast(Op, DL.getIntPtrType(CV->getType()),
                                      false/*ZExt*/);
    return lowerConstant(Op);
  }

  case Instruction::PtrToInt: {
    const DataLayout &DL = getDataLayout();

    // Support only foldable casts to/from pointers that can be eliminated by
    // changing the pointer to the appropriately sized integer type.
    Constant *Op = CE->getOperand(0);
    Type *Ty = CE->getType();

    const MCExpr *OpExpr = lowerConstant(Op);

    // We can emit the pointer value into this slot if the slot is an
    // integer slot equal to the size of the pointer.
    //
    // If the pointer is larger than the resultant integer, then
    // as with Trunc just depend on the assembler to truncate it.
    if (DL.getTypeAllocSize(Ty).getFixedValue() <=
        DL.getTypeAllocSize(Op->getType()).getFixedValue())
      return OpExpr;
    if (Op->getType()->isPtrOrPtrVectorTy() &&
        DL.getTypeAllocSize(Ty) == DL.getPointerBaseSize(Op->getType()->getPointerAddressSpace()))
      return OpExpr;
    assert(!DL.isFatPointer(Op->getType()) && "Fat pointer should not be masked when lowering constant.");

    break; // Error
  }

  case Instruction::Sub: {
    GlobalValue *LHSGV;
    APInt LHSOffset;
    DSOLocalEquivalent *DSOEquiv;
    if (IsConstantOffsetFromGlobal(CE->getOperand(0), LHSGV, LHSOffset,
                                   getDataLayout(), false, &DSOEquiv)) {
      GlobalValue *RHSGV;
      APInt RHSOffset;
      if (IsConstantOffsetFromGlobal(CE->getOperand(1), RHSGV, RHSOffset,
                                     getDataLayout(), false)) {
        const MCExpr *RelocExpr =
            getObjFileLowering().lowerRelativeReference(LHSGV, RHSGV, TM);
        if (!RelocExpr) {
          const MCExpr *LHSExpr =
              MCSymbolRefExpr::create(getSymbol(LHSGV), Ctx);
          if (DSOEquiv &&
              getObjFileLowering().supportDSOLocalEquivalentLowering())
            LHSExpr =
                getObjFileLowering().lowerDSOLocalEquivalent(DSOEquiv, TM);
          RelocExpr = MCBinaryExpr::createSub(
              LHSExpr, MCSymbolRefExpr::create(getSymbol(RHSGV), Ctx), Ctx);
        }
        int64_t Addend = (LHSOffset - RHSOffset).getSExtValue();
        if (Addend != 0)
          RelocExpr = MCBinaryExpr::createAdd(
              RelocExpr, MCConstantExpr::create(Addend, Ctx), Ctx);
        return RelocExpr;
      }
    }

    const MCExpr *LHS = lowerConstant(CE->getOperand(0));
    const MCExpr *RHS = lowerConstant(CE->getOperand(1));
    return MCBinaryExpr::createSub(LHS, RHS, Ctx);
    break;
  }

  case Instruction::Add: {
    const MCExpr *LHS = lowerConstant(CE->getOperand(0));
    const MCExpr *RHS = lowerConstant(CE->getOperand(1));
    return MCBinaryExpr::createAdd(LHS, RHS, Ctx);
  }
  }

  // If the code isn't optimized, there may be outstanding folding
  // opportunities. Attempt to fold the expression using DataLayout as a
  // last resort before giving up.
  Constant *C = ConstantFoldConstant(CE, getDataLayout());
  if (C != CE)
    return lowerConstant(C);

  // Otherwise report the problem to the user.
  std::string S;
  raw_string_ostream OS(S);
  OS << "Unsupported expression in static initializer: ";
  CE->printAsOperand(OS, /*PrintType=*/false,
                     !MF ? nullptr : MF->getFunction().getParent());
  report_fatal_error(Twine(OS.str()));
}

static void emitGlobalConstantImpl(const DataLayout &DL, const Constant *C,
                                   AsmPrinter &AP,
                                   const Constant *BaseCV = nullptr,
                                   uint64_t Offset = 0,
                                   AsmPrinter::AliasMapTy *AliasList = nullptr);

static void emitGlobalConstantFP(const ConstantFP *CFP, AsmPrinter &AP);
static void emitGlobalConstantFP(APFloat APF, Type *ET, AsmPrinter &AP);

/// isRepeatedByteSequence - Determine whether the given value is
/// composed of a repeated sequence of identical bytes and return the
/// byte value.  If it is not a repeated sequence, return -1.
static int isRepeatedByteSequence(const ConstantDataSequential *V) {
  StringRef Data = V->getRawDataValues();
  assert(!Data.empty() && "Empty aggregates should be CAZ node");
  char C = Data[0];
  for (unsigned i = 1, e = Data.size(); i != e; ++i)
    if (Data[i] != C) return -1;
  return static_cast<uint8_t>(C); // Ensure 255 is not returned as -1.
}

/// isRepeatedByteSequence - Determine whether the given value is
/// composed of a repeated sequence of identical bytes and return the
/// byte value.  If it is not a repeated sequence, return -1.
static int isRepeatedByteSequence(const Value *V, const DataLayout &DL) {
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(V)) {
    uint64_t Size = DL.getTypeAllocSizeInBits(V->getType());
    assert(Size % 8 == 0);

    // Extend the element to take zero padding into account.
    APInt Value = CI->getValue().zext(Size);
    if (!Value.isSplat(8))
      return -1;

    return Value.zextOrTrunc(8).getZExtValue();
  }
  if (const ConstantArray *CA = dyn_cast<ConstantArray>(V)) {
    // Make sure all array elements are sequences of the same repeated
    // byte.
    assert(CA->getNumOperands() != 0 && "Should be a CAZ");
    Constant *Op0 = CA->getOperand(0);
    int Byte = isRepeatedByteSequence(Op0, DL);
    if (Byte == -1)
      return -1;

    // All array elements must be equal.
    for (unsigned i = 1, e = CA->getNumOperands(); i != e; ++i)
      if (CA->getOperand(i) != Op0)
        return -1;
    return Byte;
  }

  if (const ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(V))
    return isRepeatedByteSequence(CDS);

  return -1;
}

static void emitGlobalAliasInline(AsmPrinter &AP, uint64_t Offset,
                                  AsmPrinter::AliasMapTy *AliasList) {
  if (AliasList) {
    auto AliasIt = AliasList->find(Offset);
    if (AliasIt != AliasList->end()) {
      for (const GlobalAlias *GA : AliasIt->second)
        AP.OutStreamer->emitLabel(AP.getSymbol(GA));
      AliasList->erase(Offset);
    }
  }
}

static void emitGlobalConstantDataSequential(
    const DataLayout &DL, const ConstantDataSequential *CDS, AsmPrinter &AP,
    AsmPrinter::AliasMapTy *AliasList) {
  // See if we can aggregate this into a .fill, if so, emit it as such.
  int Value = isRepeatedByteSequence(CDS, DL);
  if (Value != -1) {
    uint64_t Bytes = DL.getTypeAllocSize(CDS->getType());
    // Don't emit a 1-byte object as a .fill.
    if (Bytes > 1)
      return AP.OutStreamer->emitFill(Bytes, Value);
  }

  // If this can be emitted with .ascii/.asciz, emit it as such.
  if (CDS->isString())
    return AP.OutStreamer->emitBytes(CDS->getAsString());

  // Otherwise, emit the values in successive locations.
  unsigned ElementByteSize = CDS->getElementByteSize();
  if (isa<IntegerType>(CDS->getElementType())) {
    for (unsigned I = 0, E = CDS->getNumElements(); I != E; ++I) {
      emitGlobalAliasInline(AP, ElementByteSize * I, AliasList);
      if (AP.isVerbose())
        AP.OutStreamer->getCommentOS()
            << format("0x%" PRIx64 "\n", CDS->getElementAsInteger(I));
      AP.OutStreamer->emitIntValue(CDS->getElementAsInteger(I),
                                   ElementByteSize);
    }
  } else {
    Type *ET = CDS->getElementType();
    for (unsigned I = 0, E = CDS->getNumElements(); I != E; ++I) {
      emitGlobalAliasInline(AP, ElementByteSize * I, AliasList);
      emitGlobalConstantFP(CDS->getElementAsAPFloat(I), ET, AP);
    }
  }

  unsigned Size = DL.getTypeAllocSize(CDS->getType());
  unsigned EmittedSize =
      DL.getTypeAllocSize(CDS->getElementType()) * CDS->getNumElements();
  assert(EmittedSize <= Size && "Size cannot be less than EmittedSize!");
  if (unsigned Padding = Size - EmittedSize)
    AP.OutStreamer->emitZeros(Padding);
}

static void emitGlobalConstantArray(const DataLayout &DL,
                                    const ConstantArray *CA, AsmPrinter &AP,
                                    const Constant *BaseCV, uint64_t Offset,
                                    AsmPrinter::AliasMapTy *AliasList) {
  // See if we can aggregate some values.  Make sure it can be
  // represented as a series of bytes of the constant value.
  int Value = isRepeatedByteSequence(CA, DL);

  if (Value != -1) {
    uint64_t Bytes = DL.getTypeAllocSize(CA->getType());
    AP.OutStreamer->emitFill(Bytes, Value);
  } else {
    for (unsigned I = 0, E = CA->getNumOperands(); I != E; ++I) {
      emitGlobalConstantImpl(DL, CA->getOperand(I), AP, BaseCV, Offset,
                             AliasList);
      Offset += DL.getTypeAllocSize(CA->getOperand(I)->getType());
    }
  }
}

static void emitGlobalConstantLargeInt(const ConstantInt *CI, AsmPrinter &AP);

static void emitGlobalConstantVector(const DataLayout &DL,
                                     const ConstantVector *CV, AsmPrinter &AP,
                                     AsmPrinter::AliasMapTy *AliasList) {
  Type *ElementType = CV->getType()->getElementType();
  uint64_t ElementSizeInBits = DL.getTypeSizeInBits(ElementType);
  uint64_t ElementAllocSizeInBits = DL.getTypeAllocSizeInBits(ElementType);
  uint64_t EmittedSize;
  if (ElementSizeInBits != ElementAllocSizeInBits) {
    // If the allocation size of an element is different from the size in bits,
    // printing each element separately will insert incorrect padding.
    //
    // The general algorithm here is complicated; instead of writing it out
    // here, just use the existing code in ConstantFolding.
    Type *IntT =
        IntegerType::get(CV->getContext(), DL.getTypeSizeInBits(CV->getType()));
    ConstantInt *CI = dyn_cast_or_null<ConstantInt>(ConstantFoldConstant(
        ConstantExpr::getBitCast(const_cast<ConstantVector *>(CV), IntT), DL));
    if (!CI) {
      report_fatal_error(
          "Cannot lower vector global with unusual element type");
    }
    emitGlobalAliasInline(AP, 0, AliasList);
    emitGlobalConstantLargeInt(CI, AP);
    EmittedSize = DL.getTypeStoreSize(CV->getType());
  } else {
    for (unsigned I = 0, E = CV->getType()->getNumElements(); I != E; ++I) {
      emitGlobalAliasInline(AP, DL.getTypeAllocSize(CV->getType()) * I, AliasList);
      emitGlobalConstantImpl(DL, CV->getOperand(I), AP);
    }
    EmittedSize =
        DL.getTypeAllocSize(ElementType) * CV->getType()->getNumElements();
  }

  unsigned Size = DL.getTypeAllocSize(CV->getType());
  if (unsigned Padding = Size - EmittedSize)
    AP.OutStreamer->emitZeros(Padding);
}

static void emitGlobalConstantStruct(const DataLayout &DL,
                                     const ConstantStruct *CS, AsmPrinter &AP,
                                     const Constant *BaseCV, uint64_t Offset,
                                     AsmPrinter::AliasMapTy *AliasList) {
  // Print the fields in successive locations. Pad to align if needed!
  unsigned Size = DL.getTypeAllocSize(CS->getType());
  const StructLayout *Layout = DL.getStructLayout(CS->getType());
  uint64_t SizeSoFar = 0;
  for (unsigned I = 0, E = CS->getNumOperands(); I != E; ++I) {
    const Constant *Field = CS->getOperand(I);

    // Print the actual field value.
    emitGlobalConstantImpl(DL, Field, AP, BaseCV, Offset + SizeSoFar,
                           AliasList);

    // Check if padding is needed and insert one or more 0s.
    uint64_t FieldSize = DL.getTypeAllocSize(Field->getType());
    uint64_t PadSize = ((I == E - 1 ? Size : Layout->getElementOffset(I + 1)) -
                        Layout->getElementOffset(I)) -
                       FieldSize;
    SizeSoFar += FieldSize + PadSize;

    // Insert padding - this may include padding to increase the size of the
    // current field up to the ABI size (if the struct is not packed) as well
    // as padding to ensure that the next field starts at the right offset.
    AP.OutStreamer->emitZeros(PadSize);
  }
  assert(SizeSoFar == Layout->getSizeInBytes() &&
         "Layout of constant struct may be incorrect!");
}

static void emitGlobalConstantFP(APFloat APF, Type *ET, AsmPrinter &AP) {
  assert(ET && "Unknown float type");
  APInt API = APF.bitcastToAPInt();

  // First print a comment with what we think the original floating-point value
  // should have been.
  if (AP.isVerbose()) {
    SmallString<8> StrVal;
    APF.toString(StrVal);
    ET->print(AP.OutStreamer->getCommentOS());
    AP.OutStreamer->getCommentOS() << ' ' << StrVal << '\n';
  }

  // Now iterate through the APInt chunks, emitting them in endian-correct
  // order, possibly with a smaller chunk at beginning/end (e.g. for x87 80-bit
  // floats).
  unsigned NumBytes = API.getBitWidth() / 8;
  unsigned TrailingBytes = NumBytes % sizeof(uint64_t);
  const uint64_t *p = API.getRawData();

  // PPC's long double has odd notions of endianness compared to how LLVM
  // handles it: p[0] goes first for *big* endian on PPC.
  if (AP.getDataLayout().isBigEndian() && !ET->isPPC_FP128Ty()) {
    int Chunk = API.getNumWords() - 1;

    if (TrailingBytes)
      AP.OutStreamer->emitIntValueInHexWithPadding(p[Chunk--], TrailingBytes);

    for (; Chunk >= 0; --Chunk)
      AP.OutStreamer->emitIntValueInHexWithPadding(p[Chunk], sizeof(uint64_t));
  } else {
    unsigned Chunk;
    for (Chunk = 0; Chunk < NumBytes / sizeof(uint64_t); ++Chunk)
      AP.OutStreamer->emitIntValueInHexWithPadding(p[Chunk], sizeof(uint64_t));

    if (TrailingBytes)
      AP.OutStreamer->emitIntValueInHexWithPadding(p[Chunk], TrailingBytes);
  }

  // Emit the tail padding for the long double.
  const DataLayout &DL = AP.getDataLayout();
  AP.OutStreamer->emitZeros(DL.getTypeAllocSize(ET) - DL.getTypeStoreSize(ET));
}

static void emitGlobalConstantFP(const ConstantFP *CFP, AsmPrinter &AP) {
  emitGlobalConstantFP(CFP->getValueAPF(), CFP->getType(), AP);
}

static void emitGlobalConstantLargeInt(const ConstantInt *CI, AsmPrinter &AP) {
  const DataLayout &DL = AP.getDataLayout();
  unsigned BitWidth = CI->getBitWidth();

  // Copy the value as we may massage the layout for constants whose bit width
  // is not a multiple of 64-bits.
  APInt Realigned(CI->getValue());
  uint64_t ExtraBits = 0;
  unsigned ExtraBitsSize = BitWidth & 63;

  if (ExtraBitsSize) {
    // The bit width of the data is not a multiple of 64-bits.
    // The extra bits are expected to be at the end of the chunk of the memory.
    // Little endian:
    // * Nothing to be done, just record the extra bits to emit.
    // Big endian:
    // * Record the extra bits to emit.
    // * Realign the raw data to emit the chunks of 64-bits.
    if (DL.isBigEndian()) {
      // Basically the structure of the raw data is a chunk of 64-bits cells:
      //    0        1         BitWidth / 64
      // [chunk1][chunk2] ... [chunkN].
      // The most significant chunk is chunkN and it should be emitted first.
      // However, due to the alignment issue chunkN contains useless bits.
      // Realign the chunks so that they contain only useful information:
      // ExtraBits     0       1       (BitWidth / 64) - 1
      //       chu[nk1 chu][nk2 chu] ... [nkN-1 chunkN]
      ExtraBitsSize = alignTo(ExtraBitsSize, 8);
      ExtraBits = Realigned.getRawData()[0] &
        (((uint64_t)-1) >> (64 - ExtraBitsSize));
      if (BitWidth >= 64)
        Realigned.lshrInPlace(ExtraBitsSize);
    } else
      ExtraBits = Realigned.getRawData()[BitWidth / 64];
  }

  // We don't expect assemblers to support integer data directives
  // for more than 64 bits, so we emit the data in at most 64-bit
  // quantities at a time.
  const uint64_t *RawData = Realigned.getRawData();
  for (unsigned i = 0, e = BitWidth / 64; i != e; ++i) {
    uint64_t Val = DL.isBigEndian() ? RawData[e - i - 1] : RawData[i];
    AP.OutStreamer->emitIntValue(Val, 8);
  }

  if (ExtraBitsSize) {
    // Emit the extra bits after the 64-bits chunks.

    // Emit a directive that fills the expected size.
    uint64_t Size = AP.getDataLayout().getTypeStoreSize(CI->getType());
    Size -= (BitWidth / 64) * 8;
    assert(Size && Size * 8 >= ExtraBitsSize &&
           (ExtraBits & (((uint64_t)-1) >> (64 - ExtraBitsSize)))
           == ExtraBits && "Directive too small for extra bits.");
    AP.OutStreamer->emitIntValue(ExtraBits, Size);
  }
}

/// Transform a not absolute MCExpr containing a reference to a GOT
/// equivalent global, by a target specific GOT pc relative access to the
/// final symbol.
static void handleIndirectSymViaGOTPCRel(AsmPrinter &AP, const MCExpr **ME,
                                         const Constant *BaseCst,
                                         uint64_t Offset) {
  // The global @foo below illustrates a global that uses a got equivalent.
  //
  //  @bar = global i32 42
  //  @gotequiv = private unnamed_addr constant i32* @bar
  //  @foo = i32 trunc (i64 sub (i64 ptrtoint (i32** @gotequiv to i64),
  //                             i64 ptrtoint (i32* @foo to i64))
  //                        to i32)
  //
  // The cstexpr in @foo is converted into the MCExpr `ME`, where we actually
  // check whether @foo is suitable to use a GOTPCREL. `ME` is usually in the
  // form:
  //
  //  foo = cstexpr, where
  //    cstexpr := <gotequiv> - "." + <cst>
  //    cstexpr := <gotequiv> - (<foo> - <offset from @foo base>) + <cst>
  //
  // After canonicalization by evaluateAsRelocatable `ME` turns into:
  //
  //  cstexpr := <gotequiv> - <foo> + gotpcrelcst, where
  //    gotpcrelcst := <offset from @foo base> + <cst>
  MCValue MV;
  if (!(*ME)->evaluateAsRelocatable(MV, nullptr, nullptr) || MV.isAbsolute())
    return;
  const MCSymbolRefExpr *SymA = MV.getSymA();
  if (!SymA)
    return;

  // Check that GOT equivalent symbol is cached.
  const MCSymbol *GOTEquivSym = &SymA->getSymbol();
  if (!AP.GlobalGOTEquivs.count(GOTEquivSym))
    return;

  const GlobalValue *BaseGV = dyn_cast_or_null<GlobalValue>(BaseCst);
  if (!BaseGV)
    return;

  // Check for a valid base symbol
  const MCSymbol *BaseSym = AP.getSymbol(BaseGV);
  const MCSymbolRefExpr *SymB = MV.getSymB();

  if (!SymB || BaseSym != &SymB->getSymbol())
    return;

  // Make sure to match:
  //
  //    gotpcrelcst := <offset from @foo base> + <cst>
  //
  // If gotpcrelcst is positive it means that we can safely fold the pc rel
  // displacement into the GOTPCREL. We can also can have an extra offset <cst>
  // if the target knows how to encode it.
  int64_t GOTPCRelCst = Offset + MV.getConstant();
  if (GOTPCRelCst < 0)
    return;
  if (!AP.getObjFileLowering().supportGOTPCRelWithOffset() && GOTPCRelCst != 0)
    return;

  // Emit the GOT PC relative to replace the got equivalent global, i.e.:
  //
  //  bar:
  //    .long 42
  //  gotequiv:
  //    .quad bar
  //  foo:
  //    .long gotequiv - "." + <cst>
  //
  // is replaced by the target specific equivalent to:
  //
  //  bar:
  //    .long 42
  //  foo:
  //    .long bar@GOTPCREL+<gotpcrelcst>
  AsmPrinter::GOTEquivUsePair Result = AP.GlobalGOTEquivs[GOTEquivSym];
  const GlobalVariable *GV = Result.first;
  int NumUses = (int)Result.second;
  const GlobalValue *FinalGV = dyn_cast<GlobalValue>(GV->getOperand(0));
  const MCSymbol *FinalSym = AP.getSymbol(FinalGV);
  *ME = AP.getObjFileLowering().getIndirectSymViaGOTPCRel(
      FinalGV, FinalSym, MV, Offset, AP.MMI, *AP.OutStreamer);

  // Update GOT equivalent usage information
  --NumUses;
  if (NumUses >= 0)
    AP.GlobalGOTEquivs[GOTEquivSym] = std::make_pair(GV, NumUses);
}

static void emitGlobalConstantCHERICap(const DataLayout &DL, const Constant *CV,
                                       AsmPrinter &AP) {
  const uint64_t CapWidth = DL.getPointerTypeSize(CV->getType());
  // Handle (void *)5 etc as an untagged capability with base/length/perms 0,
  // and offset 5.

  if (auto *GEP = dyn_cast<GEPOperator>(CV)) {
    // NULL + constant -> emit intcap constant value
    if (isa<ConstantPointerNull>(GEP->getPointerOperand())) {
      // This IR construct is used to generate guaranteed untagged values
      APInt Value(DL.getIndexSizeInBits(GEP->getPointerAddressSpace()), 0);
      if (GEP->accumulateConstantOffset(DL, Value)) {
        AP.OutStreamer->emitCheriIntcap(Value.getSExtValue(), CapWidth);
      } else {
        // Create an untagged capability from an expression:
        // This can happen for e.g. __intcap_t x = (__cheri_addr long)&x; etc.
        assert(GEP->getNumOperands() == 2 && "Unexpected operand count");
        const MCExpr *Expr =
            AP.lowerConstant(cast<Constant>(GEP->getOperand(1)));
        assert(Expr && "Could not lower constant expr?");
        AP.OutStreamer->emitCheriIntcap(Expr, CapWidth);
      }
      return;
    }
  }

  const MCExpr *Expr = AP.lowerConstant(CV);
  if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr)) {
    AP.OutStreamer->emitCheriIntcap(CE->getValue(), CapWidth);
    return;
  }
  GlobalValue *GV;
  APInt Addend;
  if (IsConstantOffsetFromGlobal(const_cast<Constant *>(CV), GV, Addend, DL,
                                 true)) {
    AP.OutStreamer->EmitCheriCapability(AP.getSymbol(GV), Addend.getSExtValue(),
                                        CapWidth);
    return;
  } else if (const MCSymbolRefExpr *SRE = dyn_cast<MCSymbolRefExpr>(Expr)) {
    if (auto BA = dyn_cast<BlockAddress>(CV)) {
      // For block addresses we emit `.chericap FN+(.LtmpN - FN)`
      // NB: Must use a non-preemptible symbol
      auto FnStart = AP.getSymbolPreferLocal(*BA->getFunction(), true);
      const MCExpr *DiffToStart = MCBinaryExpr::createSub(SRE, MCSymbolRefExpr::create(FnStart, AP.OutContext), AP.OutContext);
      AP.OutStreamer->EmitCheriCapability(FnStart, DiffToStart, CapWidth);
      return;
    }
    // Emit capability for label whose address is stored in a global variable
    // FIXME: Any more cases to handle other than blockaddress?
    if (SRE->getSymbol().isTemporary()) {
      report_fatal_error(
          "Cannot emit a global .chericap referring to a temporary since this "
          "will result in the wrong value at runtime!");
      AP.OutStreamer->EmitCheriCapability(&SRE->getSymbol(), CapWidth);
      return;
    }
  }
  llvm_unreachable("Tried to emit a capability which is neither a constant nor "
                   "a global+offset");
}

static void emitGlobalConstantImpl(const DataLayout &DL, const Constant *CV,
                                   AsmPrinter &AP, const Constant *BaseCV,
                                   uint64_t Offset,
                                   AsmPrinter::AliasMapTy *AliasList) {
  emitGlobalAliasInline(AP, Offset, AliasList);
  uint64_t Size = DL.getTypeAllocSize(CV->getType());

  // Globals with sub-elements such as combinations of arrays and structs
  // are handled recursively by emitGlobalConstantImpl. Keep track of the
  // constant symbol base and the current position with BaseCV and Offset.
  if (!BaseCV && CV->hasOneUse())
    BaseCV = dyn_cast<Constant>(CV->user_back());

  if (isa<ConstantAggregateZero>(CV) || isa<UndefValue>(CV))
    return AP.OutStreamer->emitZeros(Size);

  if (const ConstantInt *CI = dyn_cast<ConstantInt>(CV)) {
    const uint64_t StoreSize = DL.getTypeStoreSize(CV->getType());

    if (StoreSize <= 8) {
      if (AP.isVerbose())
        AP.OutStreamer->getCommentOS()
            << format("0x%" PRIx64 "\n", CI->getZExtValue());
      AP.OutStreamer->emitIntValue(CI->getZExtValue(), StoreSize);
    } else {
      emitGlobalConstantLargeInt(CI, AP);
    }

    // Emit tail padding if needed
    if (Size != StoreSize)
      AP.OutStreamer->emitZeros(Size - StoreSize);

    return;
  }

  if (const ConstantFP *CFP = dyn_cast<ConstantFP>(CV))
    return emitGlobalConstantFP(CFP, AP);

  if (isa<ConstantPointerNull>(CV)) {
    if (CV->getType()->isPointerTy() && DL.isFatPointer(CV->getType()))
      AP.OutStreamer->emitCheriIntcap((int64_t)0, Size);
    else
      AP.OutStreamer->emitIntValue(0, Size);
    return;
  }

  if (const ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(CV))
    return emitGlobalConstantDataSequential(DL, CDS, AP, AliasList);

  if (const ConstantArray *CVA = dyn_cast<ConstantArray>(CV))
    return emitGlobalConstantArray(DL, CVA, AP, BaseCV, Offset, AliasList);

  if (const ConstantStruct *CVS = dyn_cast<ConstantStruct>(CV))
    return emitGlobalConstantStruct(DL, CVS, AP, BaseCV, Offset, AliasList);

  if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(CV)) {
    // Look through bitcasts, which might not be able to be MCExpr'ized (e.g. of
    // vectors).
    if (CE->getOpcode() == Instruction::BitCast)
      return emitGlobalConstantImpl(DL, CE->getOperand(0), AP);

    if (Size > 8) {
      // If the constant expression's size is greater than 64-bits, then we have
      // to emit the value in chunks. Try to constant fold the value and emit it
      // that way.
      Constant *New = ConstantFoldConstant(CE, DL);
      if (New != CE)
        return emitGlobalConstantImpl(DL, New, AP);
    }
  }

  if (const ConstantVector *V = dyn_cast<ConstantVector>(CV))
    return emitGlobalConstantVector(DL, V, AP, AliasList);

  if (CV->getType()->isPointerTy() && DL.isFatPointer(CV->getType()))
    return emitGlobalConstantCHERICap(DL, CV, AP);

  // Otherwise, it must be a ConstantExpr.  Lower it to an MCExpr, then emit it
  // thread the streamer with EmitValue.
  const MCExpr *ME = AP.lowerConstant(CV);

  // Since lowerConstant already folded and got rid of all IR pointer and
  // integer casts, detect GOT equivalent accesses by looking into the MCExpr
  // directly.
  if (AP.getObjFileLowering().supportIndirectSymViaGOTPCRel())
    handleIndirectSymViaGOTPCRel(AP, &ME, BaseCV, Offset);

  AP.OutStreamer->emitValue(ME, Size);
}

/// EmitGlobalConstant - Print a general LLVM constant to the .s file.
void AsmPrinter::emitGlobalConstant(const DataLayout &DL, const Constant *CV,
                                    uint64_t TailPadding,
                                    AliasMapTy *AliasList) {
  uint64_t Size = DL.getTypeAllocSize(CV->getType());
  if (Size)
    emitGlobalConstantImpl(DL, CV, *this, nullptr, 0, AliasList);
  else if (MAI->hasSubsectionsViaSymbols()) {
    // If the global has zero size, emit a single byte so that two labels don't
    // look like they are at the same location.
    OutStreamer->emitIntValue(0, 1);
  }
  if (TailPadding != 0) {
    OutStreamer->AddComment("Tail padding to ensure precise bounds");
    OutStreamer->emitZeros(TailPadding);
  }
  if (!AliasList)
    return;
  // TODO: These remaining aliases are not emitted in the correct location. Need
  // to handle the case where the alias offset doesn't refer to any sub-element.
  for (auto &AliasPair : *AliasList) {
    for (const GlobalAlias *GA : AliasPair.second)
      OutStreamer->emitLabel(getSymbol(GA));
  }
}

void AsmPrinter::emitMachineConstantPoolValue(MachineConstantPoolValue *MCPV) {
  // Target doesn't support this yet!
  llvm_unreachable("Target does not support EmitMachineConstantPoolValue");
}

void AsmPrinter::printOffset(int64_t Offset, raw_ostream &OS) const {
  if (Offset > 0)
    OS << '+' << Offset;
  else if (Offset < 0)
    OS << Offset;
}

void AsmPrinter::emitNops(unsigned N) {
  MCInst Nop = MF->getSubtarget().getInstrInfo()->getNop();
  for (; N; --N)
    EmitToStreamer(*OutStreamer, Nop);
}

//===----------------------------------------------------------------------===//
// Symbol Lowering Routines.
//===----------------------------------------------------------------------===//

MCSymbol *AsmPrinter::createTempSymbol(const Twine &Name) const {
  return OutContext.createTempSymbol(Name, true);
}

MCSymbol *AsmPrinter::GetBlockAddressSymbol(const BlockAddress *BA) const {
  return const_cast<AsmPrinter *>(this)->getAddrLabelSymbol(
      BA->getBasicBlock());
}

MCSymbol *AsmPrinter::GetBlockAddressSymbol(const BasicBlock *BB) const {
  return const_cast<AsmPrinter *>(this)->getAddrLabelSymbol(BB);
}

/// GetCPISymbol - Return the symbol for the specified constant pool entry.
MCSymbol *AsmPrinter::GetCPISymbol(unsigned CPID) const {
  if (getSubtargetInfo().getTargetTriple().isWindowsMSVCEnvironment()) {
    const MachineConstantPoolEntry &CPE =
        MF->getConstantPool()->getConstants()[CPID];
    if (!CPE.isMachineConstantPoolEntry()) {
      const DataLayout &DL = MF->getDataLayout();
      SectionKind Kind = CPE.getSectionKind(&DL);
      const Constant *C = CPE.Val.ConstVal;
      Align Alignment = CPE.Alignment;
      if (const MCSectionCOFF *S = dyn_cast<MCSectionCOFF>(
              getObjFileLowering().getSectionForConstant(DL, Kind, C,
                                                         Alignment))) {
        if (MCSymbol *Sym = S->getCOMDATSymbol()) {
          if (Sym->isUndefined())
            OutStreamer->emitSymbolAttribute(Sym, MCSA_Global);
          return Sym;
        }
      }
    }
  }

  const DataLayout &DL = getDataLayout();
  return OutContext.getOrCreateSymbol(Twine(DL.getPrivateGlobalPrefix()) +
                                      "CPI" + Twine(getFunctionNumber()) + "_" +
                                      Twine(CPID));
}

/// GetJTISymbol - Return the symbol for the specified jump table entry.
MCSymbol *AsmPrinter::GetJTISymbol(unsigned JTID, bool isLinkerPrivate) const {
  return MF->getJTISymbol(JTID, OutContext, isLinkerPrivate);
}

/// GetJTSetSymbol - Return the symbol for the specified jump table .set
/// FIXME: privatize to AsmPrinter.
MCSymbol *AsmPrinter::GetJTSetSymbol(unsigned UID, unsigned MBBID) const {
  const DataLayout &DL = getDataLayout();
  return OutContext.getOrCreateSymbol(Twine(DL.getPrivateGlobalPrefix()) +
                                      Twine(getFunctionNumber()) + "_" +
                                      Twine(UID) + "_set_" + Twine(MBBID));
}

MCSymbol *AsmPrinter::getSymbolWithGlobalValueBase(const GlobalValue *GV,
                                                   StringRef Suffix) const {
  return getObjFileLowering().getSymbolWithGlobalValueBase(GV, Suffix, TM);
}

/// Return the MCSymbol for the specified ExternalSymbol.
MCSymbol *AsmPrinter::GetExternalSymbolSymbol(StringRef Sym) const {
  SmallString<60> NameStr;
  Mangler::getNameWithPrefix(NameStr, Sym, getDataLayout());
  return OutContext.getOrCreateSymbol(NameStr);
}

/// PrintParentLoopComment - Print comments about parent loops of this one.
static void PrintParentLoopComment(raw_ostream &OS, const MachineLoop *Loop,
                                   unsigned FunctionNumber) {
  if (!Loop) return;
  PrintParentLoopComment(OS, Loop->getParentLoop(), FunctionNumber);
  OS.indent(Loop->getLoopDepth()*2)
    << "Parent Loop BB" << FunctionNumber << "_"
    << Loop->getHeader()->getNumber()
    << " Depth=" << Loop->getLoopDepth() << '\n';
}

/// PrintChildLoopComment - Print comments about child loops within
/// the loop for this basic block, with nesting.
static void PrintChildLoopComment(raw_ostream &OS, const MachineLoop *Loop,
                                  unsigned FunctionNumber) {
  // Add child loop information
  for (const MachineLoop *CL : *Loop) {
    OS.indent(CL->getLoopDepth()*2)
      << "Child Loop BB" << FunctionNumber << "_"
      << CL->getHeader()->getNumber() << " Depth " << CL->getLoopDepth()
      << '\n';
    PrintChildLoopComment(OS, CL, FunctionNumber);
  }
}

/// emitBasicBlockLoopComments - Pretty-print comments for basic blocks.
static void emitBasicBlockLoopComments(const MachineBasicBlock &MBB,
                                       const MachineLoopInfo *LI,
                                       const AsmPrinter &AP) {
  // Add loop depth information
  const MachineLoop *Loop = LI->getLoopFor(&MBB);
  if (!Loop) return;

  MachineBasicBlock *Header = Loop->getHeader();
  assert(Header && "No header for loop");

  // If this block is not a loop header, just print out what is the loop header
  // and return.
  if (Header != &MBB) {
    AP.OutStreamer->AddComment("  in Loop: Header=BB" +
                               Twine(AP.getFunctionNumber())+"_" +
                               Twine(Loop->getHeader()->getNumber())+
                               " Depth="+Twine(Loop->getLoopDepth()));
    return;
  }

  // Otherwise, it is a loop header.  Print out information about child and
  // parent loops.
  raw_ostream &OS = AP.OutStreamer->getCommentOS();

  PrintParentLoopComment(OS, Loop->getParentLoop(), AP.getFunctionNumber());

  OS << "=>";
  OS.indent(Loop->getLoopDepth()*2-2);

  OS << "This ";
  if (Loop->isInnermost())
    OS << "Inner ";
  OS << "Loop Header: Depth=" + Twine(Loop->getLoopDepth()) << '\n';

  PrintChildLoopComment(OS, Loop, AP.getFunctionNumber());
}

/// emitBasicBlockStart - This method prints the label for the specified
/// MachineBasicBlock, an alignment (if present) and a comment describing
/// it if appropriate.
void AsmPrinter::emitBasicBlockStart(const MachineBasicBlock &MBB) {
  // End the previous funclet and start a new one.
  if (MBB.isEHFuncletEntry()) {
    for (const HandlerInfo &HI : Handlers) {
      HI.Handler->endFunclet();
      HI.Handler->beginFunclet(MBB);
    }
  }

  // Switch to a new section if this basic block must begin a section. The
  // entry block is always placed in the function section and is handled
  // separately.
  if (MBB.isBeginSection() && !MBB.isEntryBlock()) {
    OutStreamer->switchSection(
        getObjFileLowering().getSectionForMachineBasicBlock(MF->getFunction(),
                                                            MBB, TM));
    CurrentSectionBeginSym = MBB.getSymbol();
  }

  // Emit an alignment directive for this block, if needed.
  const Align Alignment = MBB.getAlignment();
  if (Alignment != Align(1))
    emitAlignment(Alignment, nullptr, MBB.getMaxBytesForAlignment());

  // If the block has its address taken, emit any labels that were used to
  // reference the block.  It is possible that there is more than one label
  // here, because multiple LLVM BB's may have been RAUW'd to this block after
  // the references were generated.
  if (MBB.isIRBlockAddressTaken()) {
    if (isVerbose())
      OutStreamer->AddComment("Block address taken");

    BasicBlock *BB = MBB.getAddressTakenIRBlock();
    assert(BB && BB->hasAddressTaken() && "Missing BB");
    for (MCSymbol *Sym : getAddrLabelSymbolToEmit(BB))
      OutStreamer->emitLabel(Sym);
  } else if (isVerbose() && MBB.isMachineBlockAddressTaken()) {
    OutStreamer->AddComment("Block address taken");
  }

  // Print some verbose block comments.
  if (isVerbose()) {
    if (const BasicBlock *BB = MBB.getBasicBlock()) {
      if (BB->hasName()) {
        BB->printAsOperand(OutStreamer->getCommentOS(),
                           /*PrintType=*/false, BB->getModule());
        OutStreamer->getCommentOS() << '\n';
      }
    }

    assert(MLI != nullptr && "MachineLoopInfo should has been computed");
    emitBasicBlockLoopComments(MBB, MLI, *this);
  }

  // Print the main label for the block.
  if (shouldEmitLabelForBasicBlock(MBB)) {
    if (isVerbose() && MBB.hasLabelMustBeEmitted())
      OutStreamer->AddComment("Label of block must be emitted");
    OutStreamer->emitLabel(MBB.getSymbol());
  } else {
    if (isVerbose()) {
      // NOTE: Want this comment at start of line, don't emit with AddComment.
      OutStreamer->emitRawComment(" %bb." + Twine(MBB.getNumber()) + ":",
                                  false);
    }
  }

  if (MBB.isEHCatchretTarget() &&
      MAI->getExceptionHandlingType() == ExceptionHandling::WinEH) {
    OutStreamer->emitLabel(MBB.getEHCatchretSymbol());
  }

  // With BB sections, each basic block must handle CFI information on its own
  // if it begins a section (Entry block call is handled separately, next to
  // beginFunction).
  if (MBB.isBeginSection() && !MBB.isEntryBlock())
    for (const HandlerInfo &HI : Handlers)
      HI.Handler->beginBasicBlockSection(MBB);
}

void AsmPrinter::emitBasicBlockEnd(const MachineBasicBlock &MBB) {
  // Check if CFI information needs to be updated for this MBB with basic block
  // sections.
  if (MBB.isEndSection())
    for (const HandlerInfo &HI : Handlers)
      HI.Handler->endBasicBlockSection(MBB);
}

void AsmPrinter::emitVisibility(MCSymbol *Sym, unsigned Visibility,
                                bool IsDefinition) const {
  MCSymbolAttr Attr = MCSA_Invalid;

  switch (Visibility) {
  default: break;
  case GlobalValue::HiddenVisibility:
    if (IsDefinition)
      Attr = MAI->getHiddenVisibilityAttr();
    else
      Attr = MAI->getHiddenDeclarationVisibilityAttr();
    break;
  case GlobalValue::ProtectedVisibility:
    Attr = MAI->getProtectedVisibilityAttr();
    break;
  }

  if (Attr != MCSA_Invalid)
    OutStreamer->emitSymbolAttribute(Sym, Attr);
}

bool AsmPrinter::shouldEmitLabelForBasicBlock(
    const MachineBasicBlock &MBB) const {
  // With `-fbasic-block-sections=`, a label is needed for every non-entry block
  // in the labels mode (option `=labels`) and every section beginning in the
  // sections mode (`=all` and `=list=`).
  if ((MF->hasBBLabels() || MBB.isBeginSection()) && !MBB.isEntryBlock())
    return true;
  // A label is needed for any block with at least one predecessor (when that
  // predecessor is not the fallthrough predecessor, or if it is an EH funclet
  // entry, or if a label is forced).
  return !MBB.pred_empty() &&
         (!isBlockOnlyReachableByFallthrough(&MBB) || MBB.isEHFuncletEntry() ||
          MBB.hasLabelMustBeEmitted());
}

/// isBlockOnlyReachableByFallthough - Return true if the basic block has
/// exactly one predecessor and the control transfer mechanism between
/// the predecessor and this block is a fall-through.
bool AsmPrinter::
isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB) const {
  // If this is a landing pad, it isn't a fall through.  If it has no preds,
  // then nothing falls through to it.
  if (MBB->isEHPad() || MBB->pred_empty())
    return false;

  // If there isn't exactly one predecessor, it can't be a fall through.
  if (MBB->pred_size() > 1)
    return false;

  // The predecessor has to be immediately before this block.
  MachineBasicBlock *Pred = *MBB->pred_begin();
  if (!Pred->isLayoutSuccessor(MBB))
    return false;

  // If the block is completely empty, then it definitely does fall through.
  if (Pred->empty())
    return true;

  // Check the terminators in the previous blocks
  for (const auto &MI : Pred->terminators()) {
    // If it is not a simple branch, we are in a table somewhere.
    if (!MI.isBranch() || MI.isIndirectBranch())
      return false;

    // If we are the operands of one of the branches, this is not a fall
    // through. Note that targets with delay slots will usually bundle
    // terminators with the delay slot instruction.
    for (ConstMIBundleOperands OP(MI); OP.isValid(); ++OP) {
      if (OP->isJTI())
        return false;
      if (OP->isMBB() && OP->getMBB() == MBB)
        return false;
    }
  }

  return true;
}

GCMetadataPrinter *AsmPrinter::getOrCreateGCPrinter(GCStrategy &S) {
  if (!S.usesMetadata())
    return nullptr;

  auto [GCPI, Inserted] = GCMetadataPrinters.insert({&S, nullptr});
  if (!Inserted)
    return GCPI->second.get();

  auto Name = S.getName();

  for (const GCMetadataPrinterRegistry::entry &GCMetaPrinter :
       GCMetadataPrinterRegistry::entries())
    if (Name == GCMetaPrinter.getName()) {
      std::unique_ptr<GCMetadataPrinter> GMP = GCMetaPrinter.instantiate();
      GMP->S = &S;
      GCPI->second = std::move(GMP);
      return GCPI->second.get();
    }

  report_fatal_error("no GCMetadataPrinter registered for GC: " + Twine(Name));
}

void AsmPrinter::emitStackMaps() {
  GCModuleInfo *MI = getAnalysisIfAvailable<GCModuleInfo>();
  assert(MI && "AsmPrinter didn't require GCModuleInfo?");
  bool NeedsDefault = false;
  if (MI->begin() == MI->end())
    // No GC strategy, use the default format.
    NeedsDefault = true;
  else
    for (const auto &I : *MI) {
      if (GCMetadataPrinter *MP = getOrCreateGCPrinter(*I))
        if (MP->emitStackMaps(SM, *this))
          continue;
      // The strategy doesn't have printer or doesn't emit custom stack maps.
      // Use the default format.
      NeedsDefault = true;
    }

  if (NeedsDefault)
    SM.serializeToStackMapSection();
}

/// Pin vtable to this file.
AsmPrinterHandler::~AsmPrinterHandler() = default;

void AsmPrinterHandler::markFunctionEnd() {}

// In the binary's "xray_instr_map" section, an array of these function entries
// describes each instrumentation point.  When XRay patches your code, the index
// into this table will be given to your handler as a patch point identifier.
void AsmPrinter::XRayFunctionEntry::emit(int Bytes, MCStreamer *Out) const {
  auto Kind8 = static_cast<uint8_t>(Kind);
  Out->emitBinaryData(StringRef(reinterpret_cast<const char *>(&Kind8), 1));
  Out->emitBinaryData(
      StringRef(reinterpret_cast<const char *>(&AlwaysInstrument), 1));
  Out->emitBinaryData(StringRef(reinterpret_cast<const char *>(&Version), 1));
  auto Padding = (4 * Bytes) - ((2 * Bytes) + 3);
  assert(Padding >= 0 && "Instrumentation map entry > 4 * Word Size");
  Out->emitZeros(Padding);
}

void AsmPrinter::emitXRayTable() {
  if (Sleds.empty())
    return;

  auto PrevSection = OutStreamer->getCurrentSectionOnly();
  const Function &F = MF->getFunction();
  MCSection *InstMap = nullptr;
  MCSection *FnSledIndex = nullptr;
  const Triple &TT = TM.getTargetTriple();
  // Use PC-relative addresses on all targets.
  if (TT.isOSBinFormatELF()) {
    auto LinkedToSym = cast<MCSymbolELF>(CurrentFnSym);
    auto Flags = ELF::SHF_ALLOC | ELF::SHF_LINK_ORDER;
    StringRef GroupName;
    if (F.hasComdat()) {
      Flags |= ELF::SHF_GROUP;
      GroupName = F.getComdat()->getName();
    }
    InstMap = OutContext.getELFSection("xray_instr_map", ELF::SHT_PROGBITS,
                                       Flags, 0, GroupName, F.hasComdat(),
                                       MCSection::NonUniqueID, LinkedToSym);

    if (TM.Options.XRayFunctionIndex)
      FnSledIndex = OutContext.getELFSection(
          "xray_fn_idx", ELF::SHT_PROGBITS, Flags, 0, GroupName, F.hasComdat(),
          MCSection::NonUniqueID, LinkedToSym);
  } else if (MF->getSubtarget().getTargetTriple().isOSBinFormatMachO()) {
    InstMap = OutContext.getMachOSection("__DATA", "xray_instr_map",
                                         MachO::S_ATTR_LIVE_SUPPORT,
                                         SectionKind::getReadOnlyWithRel());
    if (TM.Options.XRayFunctionIndex)
      FnSledIndex = OutContext.getMachOSection("__DATA", "xray_fn_idx",
                                               MachO::S_ATTR_LIVE_SUPPORT,
                                               SectionKind::getReadOnly());
  } else {
    llvm_unreachable("Unsupported target");
  }

  auto WordSizeBytes = MAI->getCodePointerSize();

  // Now we switch to the instrumentation map section. Because this is done
  // per-function, we are able to create an index entry that will represent the
  // range of sleds associated with a function.
  auto &Ctx = OutContext;
  MCSymbol *SledsStart =
      OutContext.createLinkerPrivateSymbol("xray_sleds_start");
  OutStreamer->switchSection(InstMap);
  OutStreamer->emitLabel(SledsStart);
  for (const auto &Sled : Sleds) {
    MCSymbol *Dot = Ctx.createTempSymbol();
    OutStreamer->emitLabel(Dot);
    OutStreamer->emitValueImpl(
        MCBinaryExpr::createSub(MCSymbolRefExpr::create(Sled.Sled, Ctx),
                                MCSymbolRefExpr::create(Dot, Ctx), Ctx),
        WordSizeBytes);
    OutStreamer->emitValueImpl(
        MCBinaryExpr::createSub(
            MCSymbolRefExpr::create(CurrentFnBegin, Ctx),
            MCBinaryExpr::createAdd(MCSymbolRefExpr::create(Dot, Ctx),
                                    MCConstantExpr::create(WordSizeBytes, Ctx),
                                    Ctx),
            Ctx),
        WordSizeBytes);
    Sled.emit(WordSizeBytes, OutStreamer.get());
  }
  MCSymbol *SledsEnd = OutContext.createTempSymbol("xray_sleds_end", true);
  OutStreamer->emitLabel(SledsEnd);

  // We then emit a single entry in the index per function. We use the symbols
  // that bound the instrumentation map as the range for a specific function.
  // Each entry here will be 2 * word size aligned, as we're writing down two
  // pointers. This should work for both 32-bit and 64-bit platforms.
  if (FnSledIndex) {
    OutStreamer->switchSection(FnSledIndex);
    OutStreamer->emitCodeAlignment(Align(2 * WordSizeBytes),
                                   &getSubtargetInfo());
    // For Mach-O, use an "l" symbol as the atom of this subsection. The label
    // difference uses a SUBTRACTOR external relocation which references the
    // symbol.
    MCSymbol *Dot = Ctx.createLinkerPrivateSymbol("xray_fn_idx");
    OutStreamer->emitLabel(Dot);
    OutStreamer->emitValueImpl(
        MCBinaryExpr::createSub(MCSymbolRefExpr::create(SledsStart, Ctx),
                                MCSymbolRefExpr::create(Dot, Ctx), Ctx),
        WordSizeBytes);
    OutStreamer->emitValueImpl(MCConstantExpr::create(Sleds.size(), Ctx),
                               WordSizeBytes);
    OutStreamer->switchSection(PrevSection);
  }
  Sleds.clear();
}

void AsmPrinter::recordSled(MCSymbol *Sled, const MachineInstr &MI,
                            SledKind Kind, uint8_t Version) {
  const Function &F = MI.getMF()->getFunction();
  auto Attr = F.getFnAttribute("function-instrument");
  bool LogArgs = F.hasFnAttribute("xray-log-args");
  bool AlwaysInstrument =
    Attr.isStringAttribute() && Attr.getValueAsString() == "xray-always";
  if (Kind == SledKind::FUNCTION_ENTER && LogArgs)
    Kind = SledKind::LOG_ARGS_ENTER;
  Sleds.emplace_back(XRayFunctionEntry{Sled, CurrentFnSym, Kind,
                                       AlwaysInstrument, &F, Version});
}

void AsmPrinter::emitPatchableFunctionEntries() {
  const Function &F = MF->getFunction();
  unsigned PatchableFunctionPrefix = 0, PatchableFunctionEntry = 0;
  (void)F.getFnAttribute("patchable-function-prefix")
      .getValueAsString()
      .getAsInteger(10, PatchableFunctionPrefix);
  (void)F.getFnAttribute("patchable-function-entry")
      .getValueAsString()
      .getAsInteger(10, PatchableFunctionEntry);
  if (!PatchableFunctionPrefix && !PatchableFunctionEntry)
    return;
  const unsigned PointerSize = getPointerSize();
  if (TM.getTargetTriple().isOSBinFormatELF()) {
    auto Flags = ELF::SHF_WRITE | ELF::SHF_ALLOC;
    const MCSymbolELF *LinkedToSym = nullptr;
    StringRef GroupName;

    // GNU as < 2.35 did not support section flag 'o'. GNU ld < 2.36 did not
    // support mixed SHF_LINK_ORDER and non-SHF_LINK_ORDER sections.
    if (MAI->useIntegratedAssembler() || MAI->binutilsIsAtLeast(2, 36)) {
      Flags |= ELF::SHF_LINK_ORDER;
      if (F.hasComdat()) {
        Flags |= ELF::SHF_GROUP;
        GroupName = F.getComdat()->getName();
      }
      LinkedToSym = cast<MCSymbolELF>(CurrentFnSym);
    }
    OutStreamer->switchSection(OutContext.getELFSection(
        "__patchable_function_entries", ELF::SHT_PROGBITS, Flags, 0, GroupName,
        F.hasComdat(), MCSection::NonUniqueID, LinkedToSym));
    emitAlignment(Align(PointerSize));
    OutStreamer->emitSymbolValue(CurrentPatchableFunctionEntrySym, PointerSize);
  }
}

uint16_t AsmPrinter::getDwarfVersion() const {
  return OutStreamer->getContext().getDwarfVersion();
}

void AsmPrinter::setDwarfVersion(uint16_t Version) {
  OutStreamer->getContext().setDwarfVersion(Version);
}

bool AsmPrinter::isDwarf64() const {
  return OutStreamer->getContext().getDwarfFormat() == dwarf::DWARF64;
}

unsigned int AsmPrinter::getDwarfOffsetByteSize() const {
  return dwarf::getDwarfOffsetByteSize(
      OutStreamer->getContext().getDwarfFormat());
}

dwarf::FormParams AsmPrinter::getDwarfFormParams() const {
  return {getDwarfVersion(), uint8_t(MAI->getCodePointerSize()),
          OutStreamer->getContext().getDwarfFormat(),
          doesDwarfUseRelocationsAcrossSections()};
}

unsigned int AsmPrinter::getUnitLengthFieldByteSize() const {
  return dwarf::getUnitLengthFieldByteSize(
      OutStreamer->getContext().getDwarfFormat());
}
