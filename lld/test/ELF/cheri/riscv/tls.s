# REQUIRES: riscv
# RUN: echo '.tbss; .globl evar; evar: .zero 4' > %t.s

# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %t.s -o %t1.32.o
# RUN: ld.lld -shared -soname=t1.so %t1.32.o -o %t1.32.so

# RUN: %riscv32_cheri_purecap_llvm-mc --defsym PIC=0 -filetype=obj %s -o %t.32.o
# RUN: ld.lld %t.32.o %t1.32.so -o %t.32
# RUN: llvm-readobj -r %t.32 | FileCheck --check-prefix=RV32-REL %s
# RUN: llvm-readelf -x .got %t.32 | FileCheck --check-prefix=RV32-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32 | FileCheck --check-prefix=RV32-DIS %s

# RUN: %riscv32_cheri_purecap_llvm-mc --defsym PIC=1 -filetype=obj %s -o %t.32.pico
# RUN: ld.lld -shared %t.32.pico %t1.32.so -o %t.32.so
# RUN: llvm-readobj -r %t.32.so | FileCheck --check-prefix=RV32-SO-REL %s
# RUN: llvm-readelf -x .got %t.32.so | FileCheck --check-prefix=RV32-SO-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32.so | FileCheck --check-prefix=RV32-SO-DIS %s

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t.s -o %t1.64.o
# RUN: ld.lld -shared -soname=t1.so %t1.64.o -o %t1.64.so

# RUN: %riscv64_cheri_purecap_llvm-mc --defsym PIC=0 -filetype=obj %s -o %t.64.o
# RUN: ld.lld %t.64.o %t1.64.so -o %t.64
# RUN: llvm-readobj -r %t.64 | FileCheck --check-prefix=RV64-REL %s
# RUN: llvm-readelf -x .got %t.64 | FileCheck --check-prefix=RV64-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64 | FileCheck --check-prefix=RV64-DIS %s

# RUN: %riscv64_cheri_purecap_llvm-mc --defsym PIC=1 -filetype=obj %s -o %t.64.pico
# RUN: ld.lld -shared %t.64.pico %t1.64.so -o %t.64.so
# RUN: llvm-readobj -r %t.64.so | FileCheck --check-prefix=RV64-SO-REL %s
# RUN: llvm-readelf -x .got %t.64.so | FileCheck --check-prefix=RV64-SO-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64.so | FileCheck --check-prefix=RV64-SO-DIS %s

# RV32-REL:      .rela.dyn {
# RV32-REL-NEXT:   0x12248 R_RISCV_TLS_DTPMOD32 evar 0x0
# RV32-REL-NEXT:   0x1224C R_RISCV_TLS_DTPREL32 evar 0x0
# RV32-REL-NEXT:   0x12250 R_RISCV_TLS_TPREL32 evar 0x0
# RV32-REL-NEXT: }

# RV32-SO-REL:      .rela.dyn {
# RV32-SO-REL-NEXT:   0x2288 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT:   0x2290 R_RISCV_TLS_TPREL32 - 0x4
# RV32-SO-REL-NEXT:   0x2278 R_RISCV_TLS_DTPMOD32 evar 0x0
# RV32-SO-REL-NEXT:   0x227C R_RISCV_TLS_DTPREL32 evar 0x0
# RV32-SO-REL-NEXT:   0x2280 R_RISCV_TLS_TPREL32 evar 0x0
# RV32-SO-REL-NEXT: }

# RV32-GOT: section '.got':
# RV32-GOT-NEXT: 0x00012240 e0210100 00000000 00000000 00000000
# RV32-GOT-NEXT: 0x00012250 00000000 00000000 01000000 04000000
# RV32-GOT-NEXT: 0x00012260 04000000 00000000

# RV32-SO-GOT: section '.got':
# RV32-SO-GOT-NEXT: 0x00002270 10220000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002280 00000000 00000000 00000000 04000000
# RV32-SO-GOT-NEXT: 0x00002290 00000000 00000000

# 0x12248 - 0x111b4 = 0x1094 (GD evar)
# RV32-DIS:      111b4: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 148

# 0x12250 - 0x111bc = 0x1094 (IE evar)
# RV32-DIS:      111bc: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 148(ca0)

# 0x12258 - 0x111c4 = 0x1094 (GD lvar)
# RV32-DIS:      111c4: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 148

# 0x12260 - 0x111cc = 0x1094 (IE lvar)
# RV32-DIS:      111cc: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 148(ca0)

# RV32-DIS:      111d4: lui a0, 0
# RV32-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 4

# 0x2278 - 0x11f0 = 0x1088 (GD evar)
# RV32-SO-DIS:      11f0: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 136

# 0x2280 - 0x11f8 = 0x1088 (IE evar)
# RV32-SO-DIS:      11f8: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 136(ca0)

# 0x2288 - 0x1200 = 0x1088 (GD lvar)
# RV32-SO-DIS:      1200: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 136

# 0x2290 - 0x1208 = 0x1088 (IE lvar)
# RV32-SO-DIS:      1208: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 136(ca0)

# RV64-REL:      .rela.dyn {
# RV64-REL-NEXT:   0x123C0 R_RISCV_TLS_DTPMOD64 evar 0x0
# RV64-REL-NEXT:   0x123C8 R_RISCV_TLS_DTPREL64 evar 0x0
# RV64-REL-NEXT:   0x123D0 R_RISCV_TLS_TPREL64 evar 0x0
# RV64-REL-NEXT: }

# RV64-SO-REL:      .rela.dyn {
# RV64-SO-REL-NEXT:   0x2430 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT:   0x2440 R_RISCV_TLS_TPREL64 - 0x4
# RV64-SO-REL-NEXT:   0x2410 R_RISCV_TLS_DTPMOD64 evar 0x0
# RV64-SO-REL-NEXT:   0x2418 R_RISCV_TLS_DTPREL64 evar 0x0
# RV64-SO-REL-NEXT:   0x2420 R_RISCV_TLS_TPREL64 evar 0x0
# RV64-SO-REL-NEXT: }

# RV64-GOT: section '.got':
# RV64-GOT-NEXT: 0x000123b0 e8220100 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000123c0 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000123d0 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000123e0 01000000 00000000 04000000 00000000
# RV64-GOT-NEXT: 0x000123f0 04000000 00000000 00000000 00000000

# RV64-SO-GOT: section '.got':
# RV64-SO-GOT-NEXT: 0x00002400 38230000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002410 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002420 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002430 00000000 00000000 04000000 00000000
# RV64-SO-GOT-NEXT: 0x00002440 00000000 00000000 00000000 00000000

# 0x123c0 - 0x112b8 = 0x1108 (GD evar)
# RV64-DIS:      112b8: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 264

# 0x123d0 - 0x112c0 = 0x1110 (IE evar)
# RV64-DIS:      112c0: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 272(ca0)

# 0x123e0 - 0x112c8 = 0x1118 (GD lvar)
# RV64-DIS:      112c8: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 280

# 0x123f0 - 0x112d0 = 0x1120 (IE lvar)
# RV64-DIS:      112d0: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 288(ca0)

# RV64-DIS:      112d8: lui a0, 0
# RV64-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 4

# 0x2410 - 0x1318 = 0x10f8 (GD evar)
# RV64-SO-DIS:      1318: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 248

# 0x2420 - 0x1320 = 0x1100 (IE evar)
# RV64-SO-DIS:      1320: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 256(ca0)

# 0x2430 - 0x1328 = 0x1108 (GD lvar)
# RV64-SO-DIS:      1328: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 264

# 0x2440 - 0x1330 = 0x1110 (IE lvar)
# RV64-SO-DIS:      1330: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 272(ca0)

.global _start
_start:
	clc.tls.gd ca0, evar

	cla.tls.ie a0, evar

	clc.tls.gd ca0, lvar

	cla.tls.ie a0, lvar

.if PIC == 0
	lui a0, %tprel_hi(lvar)
	cincoffset ca0, ctp, a0, %tprel_cincoffset(lvar)
	cincoffset ca0, ca0, %tprel_lo(lvar)
.endif

.tbss
	.zero 4
lvar:
	.zero 4
