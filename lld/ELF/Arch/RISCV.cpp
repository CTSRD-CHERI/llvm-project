//===- RISCV.cpp ----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "InputFiles.h"
#include "OutputSections.h"
#include "Symbols.h"
#include "SyntheticSections.h"
#include "Target.h"
#include "llvm/Support/ELFAttributes.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/RISCVAttributeParser.h"
#include "llvm/Support/RISCVAttributes.h"
#include "llvm/Support/RISCVISAInfo.h"
#include "llvm/Support/TimeProfiler.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {

class RISCV final : public TargetInfo {
public:
  RISCV();
  uint32_t calcEFlags() const override;
  int getCapabilitySize() const override;
  int64_t getImplicitAddend(const uint8_t *buf, RelType type) const override;
  void writeGotHeader(uint8_t *buf) const override;
  void writeGotPlt(uint8_t *buf, const Symbol &s) const override;
  void writeIgotPlt(uint8_t *buf, const Symbol &s) const override;
  void writePltHeader(uint8_t *buf) const override;
  void writePlt(uint8_t *buf, const Symbol &sym,
                uint64_t pltEntryAddr) const override;
  RelType getDynRel(RelType type) const override;
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
  bool relaxOnce(int pass) const override;
};

} // end anonymous namespace

// These are internal relocation numbers for GP relaxation. They aren't part
// of the psABI spec.
#define INTERNAL_R_RISCV_GPREL_I 256
#define INTERNAL_R_RISCV_GPREL_S 257

const uint64_t dtpOffset = 0x800;

enum Op {
  ADDI = 0x13,
  AUIPC = 0x17,
  JALR = 0x67,
  LD = 0x3003,
  LW = 0x2003,
  SRLI = 0x5013,
  SUB = 0x40000033,

  CIncOffsetImm = 0x105b,
  CLC_64 = 0x3003,
  CLC_128 = 0x200f,
};

enum Reg {
  X_RA = 1,
  X_GP = 3,
  X_TP = 4,
  X_T0 = 5,
  X_T1 = 6,
  X_T2 = 7,
  X_T3 = 28,
};

static uint32_t hi20(uint32_t val) { return (val + 0x800) >> 12; }
static uint32_t lo12(uint32_t val) { return val & 4095; }

static uint32_t itype(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t imm) {
  return op | (rd << 7) | (rs1 << 15) | (imm << 20);
}
static uint32_t rtype(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return op | (rd << 7) | (rs1 << 15) | (rs2 << 20);
}
static uint32_t utype(uint32_t op, uint32_t rd, uint32_t imm) {
  return op | (rd << 7) | (imm << 12);
}

// Extract bits v[begin:end], where range is inclusive, and begin must be < 63.
static uint32_t extractBits(uint64_t v, uint32_t begin, uint32_t end) {
  return (v & ((1ULL << (begin + 1)) - 1)) >> end;
}

static uint32_t setLO12_I(uint32_t insn, uint32_t imm) {
  return (insn & 0xfffff) | (imm << 20);
}
static uint32_t setLO12_S(uint32_t insn, uint32_t imm) {
  return (insn & 0x1fff07f) | (extractBits(imm, 11, 5) << 25) |
         (extractBits(imm, 4, 0) << 7);
}

RISCV::RISCV() {
  copyRel = R_RISCV_COPY;
  pltRel = R_RISCV_JUMP_SLOT;
  relativeRel = R_RISCV_RELATIVE;
  iRelativeRel = R_RISCV_IRELATIVE;
  cheriCapRel = R_RISCV_CHERI_CAPABILITY;
  if (config->is64) {
    symbolicRel = R_RISCV_64;
    tlsModuleIndexRel = R_RISCV_TLS_DTPMOD64;
    tlsOffsetRel = R_RISCV_TLS_DTPREL64;
    tlsGotRel = R_RISCV_TLS_TPREL64;
  } else {
    symbolicRel = R_RISCV_32;
    tlsModuleIndexRel = R_RISCV_TLS_DTPMOD32;
    tlsOffsetRel = R_RISCV_TLS_DTPREL32;
    tlsGotRel = R_RISCV_TLS_TPREL32;
  }
  if (config->isCheriAbi)
    gotRel = *cheriCapRel;
  else
    gotRel = symbolicRel;
  absPointerRel = symbolicRel;

  // .got[0] = _DYNAMIC
  gotHeaderEntriesNum = 1;
  if (config->isCheriAbi)
    gotEntrySize = getCapabilitySize();

  // .got.plt[0] = _dl_runtime_resolve, .got.plt[1] = link_map
  gotPltHeaderEntriesNum = 2;

  pltHeaderSize = 32;
  pltEntrySize = 16;
  ipltEntrySize = 16;
}

static uint32_t getEFlags(InputFile *f) {
  if (config->is64)
    return cast<ObjFile<ELF64LE>>(f)->getObj().getHeader().e_flags;
  return cast<ObjFile<ELF32LE>>(f)->getObj().getHeader().e_flags;
}

int RISCV::getCapabilitySize() const {
  return config->is64 ? 16 : 8;
}

uint32_t RISCV::calcEFlags() const {
  // If there are only binary input files (from -b binary), use a
  // value of 0 for the ELF header flags.
  if (ctx.objectFiles.empty())
    return 0;

  uint32_t target = getEFlags(ctx.objectFiles.front());

  for (InputFile *f : ctx.objectFiles) {
    uint32_t eflags = getEFlags(f);
    if (eflags & EF_RISCV_RVC)
      target |= EF_RISCV_RVC;

    if ((eflags & EF_RISCV_FLOAT_ABI) != (target & EF_RISCV_FLOAT_ABI))
      error(
          toString(f) +
          ": cannot link object files with different floating-point ABI from " +
          toString(ctx.objectFiles[0]));

    if ((eflags & EF_RISCV_RVE) != (target & EF_RISCV_RVE))
      error(toString(f) +
            ": cannot link object files with different EF_RISCV_RVE");

    if ((eflags & EF_RISCV_CHERIABI) != (target & EF_RISCV_CHERIABI))
      error(toString(f) +
            ": cannot link object files with different EF_RISCV_CHERIABI");

    if ((eflags & EF_RISCV_CAP_MODE) != (target & EF_RISCV_CAP_MODE))
      error(toString(f) +
            ": cannot link object files with different EF_RISCV_CAP_MODE");
  }

  return target;
}

int64_t RISCV::getImplicitAddend(const uint8_t *buf, RelType type) const {
  switch (type) {
  default:
    internalLinkerError(getErrorLocation(buf),
                        "cannot read addend for relocation " + toString(type));
    return 0;
  case R_RISCV_32:
  case R_RISCV_TLS_DTPMOD32:
  case R_RISCV_TLS_DTPREL32:
  case R_RISCV_TLS_TPREL32:
    return SignExtend64<32>(read32le(buf));
  case R_RISCV_64:
  case R_RISCV_TLS_DTPMOD64:
  case R_RISCV_TLS_DTPREL64:
  case R_RISCV_TLS_TPREL64:
    return read64le(buf);
  case R_RISCV_RELATIVE:
  case R_RISCV_IRELATIVE:
    return config->is64 ? read64le(buf) : read32le(buf);
  case R_RISCV_NONE:
  case R_RISCV_JUMP_SLOT:
    // These relocations are defined as not having an implicit addend.
    return 0;
  }
}

void RISCV::writeGotHeader(uint8_t *buf) const {
  if (config->is64)
    write64le(buf, mainPart->dynamic->getVA());
  else
    write32le(buf, mainPart->dynamic->getVA());
}

void RISCV::writeGotPlt(uint8_t *buf, const Symbol &s) const {
  // Initialised by __cap_relocs for CHERI
  if (config->isCheriAbi)
    return;

  if (config->is64)
    write64le(buf, in.plt->getVA());
  else
    write32le(buf, in.plt->getVA());
}

void RISCV::writeIgotPlt(uint8_t *buf, const Symbol &s) const {
  if (config->writeAddends) {
    if (config->is64)
      write64le(buf, s.getVA());
    else
      write32le(buf, s.getVA());
  }
}

void RISCV::writePltHeader(uint8_t *buf) const {
  // 1: auipc(c) (c)t2, %pcrel_hi(.got.plt)
  // (c)sub t1, (c)t1, (c)t3
  // l[wdc] (c)t3, %pcrel_lo(1b)((c)t2); (c)t3 = _dl_runtime_resolve
  // addi t1, t1, -pltHeaderSize-12; t1 = &.plt[i] - &.plt[0]
  // addi/cincoffset (c)t0, (c)t2, %pcrel_lo(1b)
  // (if shift != 0): srli t1, t1, shift; t1 = &.got.plt[i] - &.got.plt[0]
  // l[wdc] (c)t0, Ptrsize((c)t0); (c)t0 = link_map
  // (c)jr (c)t3
  // (if shift == 0): nop
  uint32_t offset = in.gotPlt->getVA() - in.plt->getVA();
  uint32_t ptrload = config->isCheriAbi ? config->is64 ? CLC_128 : CLC_64
                                        : config->is64 ? LD : LW;
  uint32_t ptraddi = config->isCheriAbi ? CIncOffsetImm : ADDI;
  // Shift is log2(pltsize / ptrsize), which is 0 for CHERI-128 so skipped
  uint32_t shift = 2 - config->is64 - config->isCheriAbi;
  uint32_t ptrsize = config->isCheriAbi ? config->capabilitySize
                                        : config->wordsize;
  write32le(buf + 0, utype(AUIPC, X_T2, hi20(offset)));
  write32le(buf + 4, rtype(SUB, X_T1, X_T1, X_T3));
  write32le(buf + 8, itype(ptrload, X_T3, X_T2, lo12(offset)));
  write32le(buf + 12, itype(ADDI, X_T1, X_T1, -target->pltHeaderSize - 12));
  write32le(buf + 16, itype(ptraddi, X_T0, X_T2, lo12(offset)));
  if (shift != 0)
    write32le(buf + 20, itype(SRLI, X_T1, X_T1, shift));
  write32le(buf + 24 - 4 * (shift == 0), itype(ptrload, X_T0, X_T0, ptrsize));
  write32le(buf + 28 - 4 * (shift == 0), itype(JALR, 0, X_T3, 0));
  if (shift == 0)
    write32le(buf + 28, itype(ADDI, 0, 0, 0));
}

void RISCV::writePlt(uint8_t *buf, const Symbol &sym,
                     uint64_t pltEntryAddr) const {
  // 1: auipc(c) (c)t3, %pcrel_hi(f@[.got.plt|.got])
  // l[wdc] (c)t3, %pcrel_lo(1b)((c)t3)
  // (c)jalr (c)t1, (c)t3
  // nop
  uint32_t ptrload = config->isCheriAbi ? config->is64 ? CLC_128 : CLC_64
                                        : config->is64 ? LD : LW;
  uint32_t entryva = sym.getGotPltVA();
  uint32_t offset = entryva - pltEntryAddr;
  write32le(buf + 0, utype(AUIPC, X_T3, hi20(offset)));
  write32le(buf + 4, itype(ptrload, X_T3, X_T3, lo12(offset)));
  write32le(buf + 8, itype(JALR, X_T1, X_T3, 0));
  write32le(buf + 12, itype(ADDI, 0, 0, 0));
}

RelType RISCV::getDynRel(RelType type) const {
  return type == symbolicRel || type == cheriCapRel
             ? type
             : static_cast<RelType>(R_RISCV_NONE);
}

RelExpr RISCV::getRelExpr(const RelType type, const Symbol &s,
                          const uint8_t *loc) const {
  switch (type) {
  case R_RISCV_NONE:
    return R_NONE;
  case R_RISCV_32:
  case R_RISCV_64:
  case R_RISCV_HI20:
  case R_RISCV_LO12_I:
  case R_RISCV_LO12_S:
  case R_RISCV_RVC_LUI:
    return R_ABS;
  case R_RISCV_ADD8:
  case R_RISCV_ADD16:
  case R_RISCV_ADD32:
  case R_RISCV_ADD64:
  case R_RISCV_SET6:
  case R_RISCV_SET8:
  case R_RISCV_SET16:
  case R_RISCV_SET32:
  case R_RISCV_SUB6:
  case R_RISCV_SUB8:
  case R_RISCV_SUB16:
  case R_RISCV_SUB32:
  case R_RISCV_SUB64:
    return R_RISCV_ADD;
  case R_RISCV_JAL:
  case R_RISCV_CHERI_CJAL:
  case R_RISCV_BRANCH:
  case R_RISCV_PCREL_HI20:
  case R_RISCV_RVC_BRANCH:
  case R_RISCV_RVC_JUMP:
  case R_RISCV_CHERI_RVC_CJUMP:
  case R_RISCV_32_PCREL:
    return R_PC;
  case R_RISCV_CALL:
  case R_RISCV_CALL_PLT:
  case R_RISCV_CHERI_CCALL:
  case R_RISCV_PLT32:
    return R_PLT_PC;
  case R_RISCV_GOT_HI20:
    return R_GOT_PC;
  case R_RISCV_PCREL_LO12_I:
  case R_RISCV_PCREL_LO12_S:
    return R_RISCV_PC_INDIRECT;
  case R_RISCV_TLS_GD_HI20:
    return R_TLSGD_PC;
  case R_RISCV_TLS_GOT_HI20:
    return R_GOT_PC;
  case R_RISCV_TPREL_HI20:
  case R_RISCV_TPREL_LO12_I:
  case R_RISCV_TPREL_LO12_S:
    return R_TPREL;
  case R_RISCV_ALIGN:
    return R_RELAX_HINT;
  case R_RISCV_TPREL_ADD:
  case R_RISCV_CHERI_TPREL_CINCOFFSET:
  case R_RISCV_RELAX:
    return config->relax ? R_RELAX_HINT : R_NONE;
  case R_RISCV_CHERI_CAPABILITY:
    return R_ABS_CAP;
  // TODO: Deprecate and eventually remove these
  case R_RISCV_CHERI_CAPTAB_PCREL_HI20:
    return R_GOT_PC;
  case R_RISCV_CHERI_TLS_IE_CAPTAB_PCREL_HI20:
    return R_GOT_PC;
  case R_RISCV_CHERI_TLS_GD_CAPTAB_PCREL_HI20:
    return R_TLSGD_PC;
  default:
    error(getErrorLocation(loc) + "unknown relocation (" + Twine(type) +
          ") against symbol " + toString(s));
    return R_NONE;
  }
}

void RISCV::relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const {
  const unsigned bits = config->wordsize * 8;

  switch (rel.type) {
  case R_RISCV_32:
    write32le(loc, val);
    return;
  case R_RISCV_64:
    write64le(loc, val);
    return;

  case R_RISCV_RVC_BRANCH: {
    checkInt(loc, val, 9, rel);
    checkAlignment(loc, val, 2, rel);
    uint16_t insn = read16le(loc) & 0xE383;
    uint16_t imm8 = extractBits(val, 8, 8) << 12;
    uint16_t imm4_3 = extractBits(val, 4, 3) << 10;
    uint16_t imm7_6 = extractBits(val, 7, 6) << 5;
    uint16_t imm2_1 = extractBits(val, 2, 1) << 3;
    uint16_t imm5 = extractBits(val, 5, 5) << 2;
    insn |= imm8 | imm4_3 | imm7_6 | imm2_1 | imm5;

    write16le(loc, insn);
    return;
  }

  case R_RISCV_RVC_JUMP:
  case R_RISCV_CHERI_RVC_CJUMP: {
    checkInt(loc, val, 12, rel);
    checkAlignment(loc, val, 2, rel);
    uint16_t insn = read16le(loc) & 0xE003;
    uint16_t imm11 = extractBits(val, 11, 11) << 12;
    uint16_t imm4 = extractBits(val, 4, 4) << 11;
    uint16_t imm9_8 = extractBits(val, 9, 8) << 9;
    uint16_t imm10 = extractBits(val, 10, 10) << 8;
    uint16_t imm6 = extractBits(val, 6, 6) << 7;
    uint16_t imm7 = extractBits(val, 7, 7) << 6;
    uint16_t imm3_1 = extractBits(val, 3, 1) << 3;
    uint16_t imm5 = extractBits(val, 5, 5) << 2;
    insn |= imm11 | imm4 | imm9_8 | imm10 | imm6 | imm7 | imm3_1 | imm5;

    write16le(loc, insn);
    return;
  }

  case R_RISCV_RVC_LUI: {
    int64_t imm = SignExtend64(val + 0x800, bits) >> 12;
    checkInt(loc, imm, 6, rel);
    if (imm == 0) { // `c.lui rd, 0` is illegal, convert to `c.li rd, 0`
      write16le(loc, (read16le(loc) & 0x0F83) | 0x4000);
    } else {
      uint16_t imm17 = extractBits(val + 0x800, 17, 17) << 12;
      uint16_t imm16_12 = extractBits(val + 0x800, 16, 12) << 2;
      write16le(loc, (read16le(loc) & 0xEF83) | imm17 | imm16_12);
    }
    return;
  }

  case R_RISCV_JAL:
  case R_RISCV_CHERI_CJAL: {
    checkInt(loc, val, 21, rel);
    checkAlignment(loc, val, 2, rel);

    uint32_t insn = read32le(loc) & 0xFFF;
    uint32_t imm20 = extractBits(val, 20, 20) << 31;
    uint32_t imm10_1 = extractBits(val, 10, 1) << 21;
    uint32_t imm11 = extractBits(val, 11, 11) << 20;
    uint32_t imm19_12 = extractBits(val, 19, 12) << 12;
    insn |= imm20 | imm10_1 | imm11 | imm19_12;

    write32le(loc, insn);
    return;
  }

  case R_RISCV_BRANCH: {
    checkInt(loc, val, 13, rel);
    checkAlignment(loc, val, 2, rel);

    uint32_t insn = read32le(loc) & 0x1FFF07F;
    uint32_t imm12 = extractBits(val, 12, 12) << 31;
    uint32_t imm10_5 = extractBits(val, 10, 5) << 25;
    uint32_t imm4_1 = extractBits(val, 4, 1) << 8;
    uint32_t imm11 = extractBits(val, 11, 11) << 7;
    insn |= imm12 | imm10_5 | imm4_1 | imm11;

    write32le(loc, insn);
    return;
  }

  // auipc[c] + [c]jalr pair
  case R_RISCV_CALL:
  case R_RISCV_CALL_PLT:
  case R_RISCV_CHERI_CCALL: {
    int64_t hi = SignExtend64(val + 0x800, bits) >> 12;
    checkInt(loc, hi, 20, rel);
    if (isInt<20>(hi)) {
      relocateNoSym(loc, R_RISCV_PCREL_HI20, val);
      relocateNoSym(loc + 4, R_RISCV_PCREL_LO12_I, val);
    }
    return;
  }

  case R_RISCV_CHERI_CAPTAB_PCREL_HI20:
  case R_RISCV_CHERI_TLS_IE_CAPTAB_PCREL_HI20:
  case R_RISCV_CHERI_TLS_GD_CAPTAB_PCREL_HI20:
  case R_RISCV_GOT_HI20:
  case R_RISCV_PCREL_HI20:
  case R_RISCV_TLS_GD_HI20:
  case R_RISCV_TLS_GOT_HI20:
  case R_RISCV_TPREL_HI20:
  case R_RISCV_HI20: {
    uint64_t hi = val + 0x800;
    checkInt(loc, SignExtend64(hi, bits) >> 12, 20, rel);
    write32le(loc, (read32le(loc) & 0xFFF) | (hi & 0xFFFFF000));
    return;
  }

  case R_RISCV_PCREL_LO12_I:
  case R_RISCV_TPREL_LO12_I:
  case R_RISCV_LO12_I: {
    uint64_t hi = (val + 0x800) >> 12;
    uint64_t lo = val - (hi << 12);
    write32le(loc, setLO12_I(read32le(loc), lo & 0xfff));
    return;
  }

  case R_RISCV_PCREL_LO12_S:
  case R_RISCV_TPREL_LO12_S:
  case R_RISCV_LO12_S: {
    uint64_t hi = (val + 0x800) >> 12;
    uint64_t lo = val - (hi << 12);
    write32le(loc, setLO12_S(read32le(loc), lo));
    return;
  }

  case INTERNAL_R_RISCV_GPREL_I:
  case INTERNAL_R_RISCV_GPREL_S: {
    Defined *gp = ElfSym::riscvGlobalPointer;
    int64_t displace = SignExtend64(val - gp->getVA(), bits);
    checkInt(loc, displace, 12, rel);
    uint32_t insn = (read32le(loc) & ~(31 << 15)) | (X_GP << 15);
    if (rel.type == INTERNAL_R_RISCV_GPREL_I)
      insn = setLO12_I(insn, displace);
    else
      insn = setLO12_S(insn, displace);
    write32le(loc, insn);
    return;
  }

  case R_RISCV_ADD8:
    *loc += val;
    return;
  case R_RISCV_ADD16:
    write16le(loc, read16le(loc) + val);
    return;
  case R_RISCV_ADD32:
    write32le(loc, read32le(loc) + val);
    return;
  case R_RISCV_ADD64:
    write64le(loc, read64le(loc) + val);
    return;
  case R_RISCV_SUB6:
    *loc = (*loc & 0xc0) | (((*loc & 0x3f) - val) & 0x3f);
    return;
  case R_RISCV_SUB8:
    *loc -= val;
    return;
  case R_RISCV_SUB16:
    write16le(loc, read16le(loc) - val);
    return;
  case R_RISCV_SUB32:
    write32le(loc, read32le(loc) - val);
    return;
  case R_RISCV_SUB64:
    write64le(loc, read64le(loc) - val);
    return;
  case R_RISCV_SET6:
    *loc = (*loc & 0xc0) | (val & 0x3f);
    return;
  case R_RISCV_SET8:
    *loc = val;
    return;
  case R_RISCV_SET16:
    write16le(loc, val);
    return;
  case R_RISCV_SET32:
  case R_RISCV_32_PCREL:
  case R_RISCV_PLT32:
    write32le(loc, val);
    return;

  case R_RISCV_TLS_DTPREL32:
    if (config->isCheriAbi)
      write32le(loc, val);
    else
      write32le(loc, val - dtpOffset);
    break;
  case R_RISCV_TLS_DTPREL64:
    if (config->isCheriAbi)
      write64le(loc, val);
    else
      write64le(loc, val - dtpOffset);
    break;

  case R_RISCV_RELAX:
    return; // Ignored (for now)

  case R_RISCV_CHERI_CAPABILITY:
    // Write a word within the capability
    if (config->is64)
      write64le(loc, val);
    else
      write32le(loc, val);
    break;

  default:
    llvm_unreachable("unknown relocation");
  }
}

namespace {
struct SymbolAnchor {
  uint64_t offset;
  Defined *d;
  bool end; // true for the anchor of st_value+st_size
};
} // namespace

struct elf::RISCVRelaxAux {
  // This records symbol start and end offsets which will be adjusted according
  // to the nearest relocDeltas element.
  SmallVector<SymbolAnchor, 0> anchors;
  // For relocations[i], the actual offset is r_offset - (i ? relocDeltas[i-1] :
  // 0).
  std::unique_ptr<uint32_t[]> relocDeltas;
  // For relocations[i], the actual type is relocTypes[i].
  std::unique_ptr<RelType[]> relocTypes;
  SmallVector<uint32_t, 0> writes;
};

static void initSymbolAnchors() {
  SmallVector<InputSection *, 0> storage;
  for (OutputSection *osec : outputSections) {
    if (!(osec->flags & SHF_EXECINSTR))
      continue;
    for (InputSection *sec : getInputSections(*osec, storage)) {
      sec->relaxAux = make<RISCVRelaxAux>();
      if (sec->relocs().size()) {
        sec->relaxAux->relocDeltas =
            std::make_unique<uint32_t[]>(sec->relocs().size());
        sec->relaxAux->relocTypes =
            std::make_unique<RelType[]>(sec->relocs().size());
      }
    }
  }
  // Store anchors (st_value and st_value+st_size) for symbols relative to text
  // sections.
  //
  // For a defined symbol foo, we may have `d->file != file` with --wrap=foo.
  // We should process foo, as the defining object file's symbol table may not
  // contain foo after redirectSymbols changed the foo entry to __wrap_foo. To
  // avoid adding a Defined that is undefined in one object file, use
  // `!d->scriptDefined` to exclude symbols that are definitely not wrapped.
  //
  // `relaxAux->anchors` may contain duplicate symbols, but that is fine.
  for (InputFile *file : ctx.objectFiles)
    for (Symbol *sym : file->getSymbols()) {
      auto *d = dyn_cast<Defined>(sym);
      if (!d || (d->file != file && !d->scriptDefined))
        continue;
      if (auto *sec = dyn_cast_or_null<InputSection>(d->section))
        if (sec->flags & SHF_EXECINSTR && sec->relaxAux) {
          // If sec is discarded, relaxAux will be nullptr.
          sec->relaxAux->anchors.push_back({d->value, d, false});
          sec->relaxAux->anchors.push_back({d->value + d->getSize(), d, true});
        }
    }
  // Sort anchors by offset so that we can find the closest relocation
  // efficiently. For a zero size symbol, ensure that its start anchor precedes
  // its end anchor. For two symbols with anchors at the same offset, their
  // order does not matter.
  for (OutputSection *osec : outputSections) {
    if (!(osec->flags & SHF_EXECINSTR))
      continue;
    for (InputSection *sec : getInputSections(*osec, storage)) {
      llvm::sort(sec->relaxAux->anchors, [](auto &a, auto &b) {
        return std::make_pair(a.offset, a.end) <
               std::make_pair(b.offset, b.end);
      });
    }
  }
}

// Relax R_RISCV_CALL/R_RISCV_CALL_PLT auipc+jalr to c.j, c.jal, or jal.
static void relaxCall(const InputSection &sec, size_t i, uint64_t loc,
                      Relocation &r, uint32_t &remove) {
  const bool rvc = config->eflags & EF_RISCV_RVC;
  const Symbol &sym = *r.sym;
  const uint64_t insnPair = read64le(sec.content().data() + r.offset);
  const uint32_t rd = extractBits(insnPair, 32 + 11, 32 + 7);
  const uint64_t dest =
      (r.expr == R_PLT_PC ? sym.getPltVA() : sym.getVA()) + r.addend;
  const int64_t displace = dest - loc;

  if (rvc && isInt<12>(displace) && rd == 0) {
    sec.relaxAux->relocTypes[i] = R_RISCV_RVC_JUMP;
    sec.relaxAux->writes.push_back(0xa001); // c.j
    remove = 6;
  } else if (rvc && isInt<12>(displace) && rd == X_RA &&
             !config->is64) { // RV32C only
    sec.relaxAux->relocTypes[i] = R_RISCV_RVC_JUMP;
    sec.relaxAux->writes.push_back(0x2001); // c.jal
    remove = 6;
  } else if (isInt<21>(displace)) {
    sec.relaxAux->relocTypes[i] = R_RISCV_JAL;
    sec.relaxAux->writes.push_back(0x6f | rd << 7); // jal
    remove = 4;
  }
}

// Relax local-exec TLS when hi20 is zero.
static void relaxTlsLe(const InputSection &sec, size_t i, uint64_t loc,
                       Relocation &r, uint32_t &remove) {
  uint64_t val = r.sym->getVA(r.addend);
  if (hi20(val) != 0)
    return;
  uint32_t insn = read32le(sec.content().data() + r.offset);
  switch (r.type) {
  case R_RISCV_TPREL_HI20:
  case R_RISCV_TPREL_ADD:
    // Remove lui rd, %tprel_hi(x) and add rd, rd, tp, %tprel_add(x).
    sec.relaxAux->relocTypes[i] = R_RISCV_RELAX;
    remove = 4;
    break;
  case R_RISCV_TPREL_LO12_I:
    // addi rd, rd, %tprel_lo(x) => addi rd, tp, st_value(x)
    sec.relaxAux->relocTypes[i] = R_RISCV_32;
    insn = (insn & ~(31 << 15)) | (X_TP << 15);
    sec.relaxAux->writes.push_back(setLO12_I(insn, val));
    break;
  case R_RISCV_TPREL_LO12_S:
    // sw rs, %tprel_lo(x)(rd) => sw rs, st_value(x)(rd)
    sec.relaxAux->relocTypes[i] = R_RISCV_32;
    insn = (insn & ~(31 << 15)) | (X_TP << 15);
    sec.relaxAux->writes.push_back(setLO12_S(insn, val));
    break;
  }
}

static void relaxHi20Lo12(const InputSection &sec, size_t i, uint64_t loc,
                          Relocation &r, uint32_t &remove) {
  const Defined *gp = ElfSym::riscvGlobalPointer;
  if (!gp)
    return;

  if (!isInt<12>(r.sym->getVA(r.addend) - gp->getVA()))
    return;

  switch (r.type) {
  case R_RISCV_HI20:
    // Remove lui rd, %hi20(x).
    sec.relaxAux->relocTypes[i] = R_RISCV_RELAX;
    remove = 4;
    break;
  case R_RISCV_LO12_I:
    sec.relaxAux->relocTypes[i] = INTERNAL_R_RISCV_GPREL_I;
    break;
  case R_RISCV_LO12_S:
    sec.relaxAux->relocTypes[i] = INTERNAL_R_RISCV_GPREL_S;
    break;
  }
}

static bool relax(InputSection &sec) {
  const uint64_t secAddr = sec.getVA();
  auto &aux = *sec.relaxAux;
  bool changed = false;
  ArrayRef<SymbolAnchor> sa = ArrayRef(aux.anchors);
  uint64_t delta = 0;

  std::fill_n(aux.relocTypes.get(), sec.relocs().size(), R_RISCV_NONE);
  aux.writes.clear();
  for (auto [i, r] : llvm::enumerate(sec.relocs())) {
    const uint64_t loc = secAddr + r.offset - delta;
    uint32_t &cur = aux.relocDeltas[i], remove = 0;
    switch (r.type) {
    case R_RISCV_ALIGN: {
      const uint64_t nextLoc = loc + r.addend;
      const uint64_t align = PowerOf2Ceil(r.addend + 2);
      // All bytes beyond the alignment boundary should be removed.
      remove = nextLoc - ((loc + align - 1) & -align);
      assert(static_cast<int32_t>(remove) >= 0 &&
             "R_RISCV_ALIGN needs expanding the content");
      break;
    }
    case R_RISCV_CALL:
    case R_RISCV_CALL_PLT:
      if (i + 1 != sec.relocs().size() &&
          sec.relocs()[i + 1].type == R_RISCV_RELAX)
        relaxCall(sec, i, loc, r, remove);
      break;
    case R_RISCV_TPREL_HI20:
    case R_RISCV_TPREL_ADD:
    case R_RISCV_TPREL_LO12_I:
    case R_RISCV_TPREL_LO12_S:
      if (i + 1 != sec.relocs().size() &&
          sec.relocs()[i + 1].type == R_RISCV_RELAX)
        relaxTlsLe(sec, i, loc, r, remove);
      break;
    case R_RISCV_HI20:
    case R_RISCV_LO12_I:
    case R_RISCV_LO12_S:
      if (i + 1 != sec.relocs().size() &&
          sec.relocs()[i + 1].type == R_RISCV_RELAX)
        relaxHi20Lo12(sec, i, loc, r, remove);
      break;
    }

    // For all anchors whose offsets are <= r.offset, they are preceded by
    // the previous relocation whose `relocDeltas` value equals `delta`.
    // Decrease their st_value and update their st_size.
    for (; sa.size() && sa[0].offset <= r.offset; sa = sa.slice(1)) {
      if (sa[0].end)
        sa[0].d->setSize(sa[0].offset - delta - sa[0].d->value);
      else
        sa[0].d->value = sa[0].offset - delta;
    }
    delta += remove;
    if (delta != cur) {
      cur = delta;
      changed = true;
    }
  }

  for (const SymbolAnchor &a : sa) {
    if (a.end)
      a.d->setSize(a.offset - delta - a.d->value);
    else
      a.d->value = a.offset - delta;
  }
  // Inform assignAddresses that the size has changed.
  if (!isUInt<32>(delta))
    fatal("section size decrease is too large: " + Twine(delta));
  sec.bytesDropped = delta;
  return changed;
}

// When relaxing just R_RISCV_ALIGN, relocDeltas is usually changed only once in
// the absence of a linker script. For call and load/store R_RISCV_RELAX, code
// shrinkage may reduce displacement and make more relocations eligible for
// relaxation. Code shrinkage may increase displacement to a call/load/store
// target at a higher fixed address, invalidating an earlier relaxation. Any
// change in section sizes can have cascading effect and require another
// relaxation pass.
bool RISCV::relaxOnce(int pass) const {
  llvm::TimeTraceScope timeScope("RISC-V relaxOnce");
  if (config->relocatable)
    return false;

  if (pass == 0)
    initSymbolAnchors();

  SmallVector<InputSection *, 0> storage;
  bool changed = false;
  for (OutputSection *osec : outputSections) {
    if (!(osec->flags & SHF_EXECINSTR))
      continue;
    for (InputSection *sec : getInputSections(*osec, storage))
      changed |= relax(*sec);
  }
  return changed;
}

void elf::riscvFinalizeRelax(int passes) {
  llvm::TimeTraceScope timeScope("Finalize RISC-V relaxation");
  log("relaxation passes: " + Twine(passes));
  SmallVector<InputSection *, 0> storage;
  for (OutputSection *osec : outputSections) {
    if (!(osec->flags & SHF_EXECINSTR))
      continue;
    for (InputSection *sec : getInputSections(*osec, storage)) {
      RISCVRelaxAux &aux = *sec->relaxAux;
      if (!aux.relocDeltas)
        continue;

      MutableArrayRef<Relocation> rels = sec->relocs();
      ArrayRef<uint8_t> old = sec->content();
      size_t newSize = old.size() - aux.relocDeltas[rels.size() - 1];
      size_t writesIdx = 0;
      uint8_t *p = context().bAlloc.Allocate<uint8_t>(newSize);
      uint64_t offset = 0;
      int64_t delta = 0;
      sec->content_ = p;
      sec->size = newSize;
      sec->bytesDropped = 0;

      // Update section content: remove NOPs for R_RISCV_ALIGN and rewrite
      // instructions for relaxed relocations.
      for (size_t i = 0, e = rels.size(); i != e; ++i) {
        uint32_t remove = aux.relocDeltas[i] - delta;
        delta = aux.relocDeltas[i];
        if (remove == 0 && aux.relocTypes[i] == R_RISCV_NONE)
          continue;

        // Copy from last location to the current relocated location.
        const Relocation &r = rels[i];
        uint64_t size = r.offset - offset;
        memcpy(p, old.data() + offset, size);
        p += size;

        // For R_RISCV_ALIGN, we will place `offset` in a location (among NOPs)
        // to satisfy the alignment requirement. If both `remove` and r.addend
        // are multiples of 4, it is as if we have skipped some NOPs. Otherwise
        // we are in the middle of a 4-byte NOP, and we need to rewrite the NOP
        // sequence.
        int64_t skip = 0;
        if (r.type == R_RISCV_ALIGN) {
          if (remove % 4 || r.addend % 4) {
            skip = r.addend - remove;
            int64_t j = 0;
            for (; j + 4 <= skip; j += 4)
              write32le(p + j, 0x00000013); // nop
            if (j != skip) {
              assert(j + 2 == skip);
              write16le(p + j, 0x0001); // c.nop
            }
          }
        } else if (RelType newType = aux.relocTypes[i]) {
          switch (newType) {
          case INTERNAL_R_RISCV_GPREL_I:
          case INTERNAL_R_RISCV_GPREL_S:
            break;
          case R_RISCV_RELAX:
            // Used by relaxTlsLe to indicate the relocation is ignored.
            break;
          case R_RISCV_RVC_JUMP:
            skip = 2;
            write16le(p, aux.writes[writesIdx++]);
            break;
          case R_RISCV_JAL:
            skip = 4;
            write32le(p, aux.writes[writesIdx++]);
            break;
          case R_RISCV_32:
            // Used by relaxTlsLe to write a uint32_t then suppress the handling
            // in relocateAlloc.
            skip = 4;
            write32le(p, aux.writes[writesIdx++]);
            aux.relocTypes[i] = R_RISCV_NONE;
            break;
          default:
            llvm_unreachable("unsupported type");
          }
        }

        p += skip;
        offset = r.offset + skip + remove;
      }
      memcpy(p, old.data() + offset, old.size() - offset);

      // Subtract the previous relocDeltas value from the relocation offset.
      // For a pair of R_RISCV_CALL/R_RISCV_RELAX with the same offset, decrease
      // their r_offset by the same delta.
      delta = 0;
      for (size_t i = 0, e = rels.size(); i != e;) {
        uint64_t cur = rels[i].offset;
        do {
          rels[i].offset -= delta;
          if (aux.relocTypes[i] != R_RISCV_NONE)
            rels[i].type = aux.relocTypes[i];
        } while (++i != e && rels[i].offset == cur);
        delta = aux.relocDeltas[i - 1];
      }
    }
  }
}

namespace {
// Representation of the merged .riscv.attributes input sections. The psABI
// specifies merge policy for attributes. E.g. if we link an object without an
// extension with an object with the extension, the output Tag_RISCV_arch shall
// contain the extension. Some tools like objdump parse .riscv.attributes and
// disabling some instructions if the first Tag_RISCV_arch does not contain an
// extension.
class RISCVAttributesSection final : public SyntheticSection {
public:
  RISCVAttributesSection()
      : SyntheticSection(0, SHT_RISCV_ATTRIBUTES, 1, ".riscv.attributes") {}

  size_t getSize() const override { return size; }
  void writeTo(uint8_t *buf) override;

  static constexpr StringRef vendor = "riscv";
  DenseMap<unsigned, unsigned> intAttr;
  DenseMap<unsigned, StringRef> strAttr;
  size_t size = 0;
};
} // namespace

static void mergeArch(RISCVISAInfo::OrderedExtensionMap &mergedExts,
                      unsigned &mergedXlen, const InputSectionBase *sec,
                      StringRef s) {
  auto maybeInfo = RISCVISAInfo::parseNormalizedArchString(s);
  if (!maybeInfo) {
    errorOrWarn(toString(sec) + ": " + s + ": " +
                llvm::toString(maybeInfo.takeError()));
    return;
  }

  // Merge extensions.
  RISCVISAInfo &info = **maybeInfo;
  if (mergedExts.empty()) {
    mergedExts = info.getExtensions();
    mergedXlen = info.getXLen();
  } else {
    for (const auto &ext : info.getExtensions()) {
      if (auto it = mergedExts.find(ext.first); it != mergedExts.end()) {
        if (std::tie(it->second.MajorVersion, it->second.MinorVersion) >=
            std::tie(ext.second.MajorVersion, ext.second.MinorVersion))
          continue;
      }
      mergedExts[ext.first] = ext.second;
    }
  }
}

static RISCVAttributesSection *
mergeAttributesSection(const SmallVector<InputSectionBase *, 0> &sections) {
  RISCVISAInfo::OrderedExtensionMap exts;
  const InputSectionBase *firstStackAlign = nullptr;
  unsigned firstStackAlignValue = 0, xlen = 0;
  bool hasArch = false;

  in.riscvAttributes = std::make_unique<RISCVAttributesSection>();
  auto &merged = static_cast<RISCVAttributesSection &>(*in.riscvAttributes);

  // Collect all tags values from attributes section.
  const auto &attributesTags = RISCVAttrs::getRISCVAttributeTags();
  for (const InputSectionBase *sec : sections) {
    RISCVAttributeParser parser;
    if (Error e = parser.parse(sec->content(), support::little))
      warn(toString(sec) + ": " + llvm::toString(std::move(e)));
    for (const auto &tag : attributesTags) {
      switch (RISCVAttrs::AttrType(tag.attr)) {
        // Integer attributes.
      case RISCVAttrs::STACK_ALIGN:
        if (auto i = parser.getAttributeValue(tag.attr)) {
          auto r = merged.intAttr.try_emplace(tag.attr, *i);
          if (r.second) {
            firstStackAlign = sec;
            firstStackAlignValue = *i;
          } else if (r.first->second != *i) {
            errorOrWarn(toString(sec) + " has stack_align=" + Twine(*i) +
                        " but " + toString(firstStackAlign) +
                        " has stack_align=" + Twine(firstStackAlignValue));
          }
        }
        continue;
      case RISCVAttrs::UNALIGNED_ACCESS:
        if (auto i = parser.getAttributeValue(tag.attr))
          merged.intAttr[tag.attr] |= *i;
        continue;

        // String attributes.
      case RISCVAttrs::ARCH:
        if (auto s = parser.getAttributeString(tag.attr)) {
          hasArch = true;
          mergeArch(exts, xlen, sec, *s);
        }
        continue;

        // Attributes which use the default handling.
      case RISCVAttrs::PRIV_SPEC:
      case RISCVAttrs::PRIV_SPEC_MINOR:
      case RISCVAttrs::PRIV_SPEC_REVISION:
        break;
      }

      // Fallback for deprecated priv_spec* and other unknown attributes: retain
      // the attribute if all input sections agree on the value. GNU ld uses 0
      // and empty strings as default values which are not dumped to the output.
      // TODO Adjust after resolution to
      // https://github.com/riscv-non-isa/riscv-elf-psabi-doc/issues/352
      if (tag.attr % 2 == 0) {
        if (auto i = parser.getAttributeValue(tag.attr)) {
          auto r = merged.intAttr.try_emplace(tag.attr, *i);
          if (!r.second && r.first->second != *i)
            r.first->second = 0;
        }
      } else if (auto s = parser.getAttributeString(tag.attr)) {
        auto r = merged.strAttr.try_emplace(tag.attr, *s);
        if (!r.second && r.first->second != *s)
          r.first->second = {};
      }
    }
  }

  if (hasArch) {
    if (auto result = RISCVISAInfo::postProcessAndChecking(
            std::make_unique<RISCVISAInfo>(xlen, exts))) {
      merged.strAttr.try_emplace(RISCVAttrs::ARCH,
                                 saver().save((*result)->toString()));
    } else {
      errorOrWarn(llvm::toString(result.takeError()));
    }
  }

  // The total size of headers: format-version [ <section-length> "vendor-name"
  // [ <file-tag> <size>.
  size_t size = 5 + merged.vendor.size() + 1 + 5;
  for (auto &attr : merged.intAttr)
    if (attr.second != 0)
      size += getULEB128Size(attr.first) + getULEB128Size(attr.second);
  for (auto &attr : merged.strAttr)
    if (!attr.second.empty())
      size += getULEB128Size(attr.first) + attr.second.size() + 1;
  merged.size = size;
  return &merged;
}

void RISCVAttributesSection::writeTo(uint8_t *buf) {
  const size_t size = getSize();
  uint8_t *const end = buf + size;
  *buf = ELFAttrs::Format_Version;
  write32(buf + 1, size - 1);
  buf += 5;

  memcpy(buf, vendor.data(), vendor.size());
  buf += vendor.size() + 1;

  *buf = ELFAttrs::File;
  write32(buf + 1, end - buf);
  buf += 5;

  for (auto &attr : intAttr) {
    if (attr.second == 0)
      continue;
    buf += encodeULEB128(attr.first, buf);
    buf += encodeULEB128(attr.second, buf);
  }
  for (auto &attr : strAttr) {
    if (attr.second.empty())
      continue;
    buf += encodeULEB128(attr.first, buf);
    memcpy(buf, attr.second.data(), attr.second.size());
    buf += attr.second.size() + 1;
  }
}

void elf::mergeRISCVAttributesSections() {
  // Find the first input SHT_RISCV_ATTRIBUTES; return if not found.
  size_t place =
      llvm::find_if(ctx.inputSections,
                    [](auto *s) { return s->type == SHT_RISCV_ATTRIBUTES; }) -
      ctx.inputSections.begin();
  if (place == ctx.inputSections.size())
    return;

  // Extract all SHT_RISCV_ATTRIBUTES sections into `sections`.
  SmallVector<InputSectionBase *, 0> sections;
  llvm::erase_if(ctx.inputSections, [&](InputSectionBase *s) {
    if (s->type != SHT_RISCV_ATTRIBUTES)
      return false;
    sections.push_back(s);
    return true;
  });

  // Add the merged section.
  ctx.inputSections.insert(ctx.inputSections.begin() + place,
                           mergeAttributesSection(sections));
}

TargetInfo *elf::getRISCVTargetInfo() {
  static RISCV target;
  return &target;
}
