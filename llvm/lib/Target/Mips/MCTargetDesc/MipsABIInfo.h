//===---- MipsABIInfo.h - Information about MIPS ABI's --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSABIINFO_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSABIINFO_H

#include "llvm/ADT/Triple.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {

template <typename T> class ArrayRef;
class MCTargetOptions;
class StringRef;
class TargetRegisterClass;

class MipsABIInfo {
public:
  enum class ABI { Unknown, O32, N32, N64 };

  class TemporalABILayout {
    uint32_t CapSize;
  public:
    int64_t GetThreadLocalOffset_CSP() const {return CapSize * 4;}
    int64_t GetThreadLocalOffset_CUSP() const {return CapSize * 5;}
    int64_t GetThreadLocalOffset_CDS() const {return CapSize * 6;}
    int64_t GetThreadLocalOffset_CDL() const {return CapSize * 7;}
    int64_t GetThreadLocalOffset_CGP() const {return CapSize * 8;}

    int64_t GetStackPointerOffset_Next() const {return CapSize * -1;}
    int64_t GetStackPointerOffset_Prev() const {return CapSize * -2;}

    int64_t GetMinStack_Size() const {return 0x2000;}
    TemporalABILayout(uint32_t capSize) : CapSize(capSize) {}
  };

protected:
  ABI ThisABI;
  bool isCheriPureCap = false;
  // FIXME: This probably exists elsewhere, but I am currently too lazy to find it.
  uint32_t CapSize;
  TemporalABILayout temporalABILayout;
public:
  MipsABIInfo(ABI ThisABI, bool isAllCap=false, uint32_t capSize = 32) : ThisABI(ThisABI),
    isCheriPureCap(isAllCap), CapSize(capSize), temporalABILayout(capSize) {}

  static MipsABIInfo Unknown() { return MipsABIInfo(ABI::Unknown); }
  static MipsABIInfo O32() { return MipsABIInfo(ABI::O32); }
  static MipsABIInfo N32() { return MipsABIInfo(ABI::N32); }
  static MipsABIInfo N64() { return MipsABIInfo(ABI::N64); }
  static MipsABIInfo CheriPureCap(uint32_t capSize = 32) { return MipsABIInfo(ABI::N64, true, capSize); }
  static MipsABIInfo computeTargetABI(const Triple &TT, StringRef CPU,
                                      const MCTargetOptions &Options);

  bool IsKnown() const { return ThisABI != ABI::Unknown; }
  bool IsO32() const { return ThisABI == ABI::O32; }
  bool IsN32() const { return ThisABI == ABI::N32; }
  bool IsN64() const { return ThisABI == ABI::N64; }
  bool IsCheriPureCap() const { return isCheriPureCap; }
  bool UsesCapabilityTable() const;
  const TemporalABILayout* GetTABILayout() const;
  uint32_t GetCapSize() const {return CapSize;}
  unsigned StackAddrSpace() const { return isCheriPureCap ? 200 : 0; }
  ABI GetEnumValue() const { return ThisABI; }

  /// The registers to use for byval arguments.
  ArrayRef<MCPhysReg> GetByValArgRegs() const;

  /// The registers to use for the variable argument list.
  ArrayRef<MCPhysReg> GetVarArgRegs() const;

  /// Obtain the size of the area allocated by the callee for arguments.
  /// CallingConv::FastCall affects the value for O32.
  unsigned GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const;

  /// Ordering of ABI's
  /// MipsGenSubtargetInfo.inc will use this to resolve conflicts when given
  /// multiple ABI options.
  bool operator<(const MipsABIInfo Other) const {
    return ThisABI < Other.GetEnumValue();
  }


  unsigned GetDefaultDataCapability() const;
  unsigned GetReturnAddress() const;
  unsigned GetReturnData() const   ;
  unsigned GetReturnSelector() const;
  unsigned GetStackPtr() const;
  unsigned GetUnsafeStackPtr() const;
  unsigned GetFramePtr() const;
  unsigned GetBasePtr() const;
  unsigned GetGlobalPtr() const;
  /// This method will eventually be replaced by GetGlobalPtr in
  /// pure-capability mode, but until all of the new linker work is done we
  /// need a separate $gp and $cgp as a transition step.
  unsigned GetGlobalCapability() const;
  unsigned GetLocalCapability() const;
  unsigned GetNullPtr() const;
  unsigned GetZeroReg() const;
  unsigned GetPtrAdduOp() const;
  unsigned GetPtrAddiuOp() const;
  unsigned GetPtrSubuOp() const;
  unsigned GetPtrAndOp() const;
  unsigned GetGPRMoveOp() const;
  unsigned GetSPMoveOp() const;
  inline bool ArePtrs64bit() const { return IsN64(); }
  inline bool AreGprs64bit() const { return IsN32() || IsN64(); }

  unsigned GetEhDataReg(unsigned I) const;
};
}

#endif
