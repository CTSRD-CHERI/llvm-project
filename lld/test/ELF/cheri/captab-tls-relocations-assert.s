# After the cheri capabitliy table relocations were added to relocations.h, RelExpr now has
# more than 64 members and the fast path using a single 64-bit bitmask no longer works for those values
# Check that we no longer trigger this assertion with R_* values >= 64

# RUNNOT: %cheri_purecap_llvm-mc %s -filetype=asm -show-encoding -show-inst -show-inst-operands
# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s
# RUN: ld.lld -shared %t.o -o %t.so

.text
__start:
  lui $2, %captab_tlsgd_hi(foo)
  daddiu $3, $2, %captab_tlsgd_lo(foo)
  lui $2, %captab_tlsldm_hi(foo)
  daddiu $3, $2, %captab_tlsldm_lo(foo)
  lui $2, %captab_tprel_hi(foo)
  daddiu $3, $2, %captab_tprel_lo(foo)

# CHECK-LABEL: Relocations [
# CHECK-NEXT:   Section (3) .rela.text {
# CHECK-NEXT:     0x0 R_MIPS_CHERI_CAPTAB_TLS_GD_HI16/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     0x4 R_MIPS_CHERI_CAPTAB_TLS_GD_LO16/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     0x8 R_MIPS_CHERI_CAPTAB_TLS_LDM_HI16/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     0xC R_MIPS_CHERI_CAPTAB_TLS_LDM_LO16/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     0x10 R_MIPS_CHERI_CAPTAB_TLS_TPREL_HI16/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     0x14 R_MIPS_CHERI_CAPTAB_TLS_TPREL_LO16/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:   }
# CHECK-NEXT: ]
