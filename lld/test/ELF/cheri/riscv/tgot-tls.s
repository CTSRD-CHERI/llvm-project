# REQUIRES: riscv
# RUN: echo '.tbss; .globl evar; evar: .zero 4' > %t.s

# RUN: %riscv32_cheri_purecap_llvm-mc -cheri-tgot-tls -filetype=obj %t.s -o %t1.32.o
# RUN: ld.lld -shared -soname=t1.so %t1.32.o -o %t1.32.so

# RUN: %riscv32_cheri_purecap_llvm-mc --defsym PIC=0 -cheri-tgot-tls -filetype=obj %s -o %t.32.o
# RUN: ld.lld %t.32.o %t1.32.so -o %t.32
# RUN: llvm-readobj -r --cap-relocs %t.32 | FileCheck --check-prefix=RV32-REL %s
# RUN: llvm-readelf -x .got -x .tgot %t.32 | FileCheck --check-prefix=RV32-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32 | FileCheck --check-prefix=RV32-DIS %s

# RUN: %riscv32_cheri_purecap_llvm-mc --defsym PIC=1 -cheri-tgot-tls -filetype=obj %s -o %t.32.pico
# RUN: ld.lld -shared %t.32.pico %t1.32.so -o %t.32.so
# RUN: llvm-readobj -r --cap-relocs %t.32.so | FileCheck --check-prefix=RV32-SO-REL %s
# RUN: llvm-readelf -x .got -x .tgot %t.32.so | FileCheck --check-prefix=RV32-SO-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32.so | FileCheck --check-prefix=RV32-SO-DIS %s

# RUN: %riscv64_cheri_purecap_llvm-mc -cheri-tgot-tls -filetype=obj %t.s -o %t1.64.o
# RUN: ld.lld -shared -soname=t1.so %t1.64.o -o %t1.64.so

# RUN: %riscv64_cheri_purecap_llvm-mc --defsym PIC=0 -cheri-tgot-tls -filetype=obj %s -o %t.64.o
# RUN: ld.lld %t.64.o %t1.64.so -o %t.64
# RUN: llvm-readobj -r --cap-relocs %t.64 | FileCheck --check-prefix=RV64-REL %s
# RUN: llvm-readelf -x .got -x .tgot %t.64 | FileCheck --check-prefix=RV64-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64 | FileCheck --check-prefix=RV64-DIS %s

# RUN: %riscv64_cheri_purecap_llvm-mc --defsym PIC=1 -cheri-tgot-tls -filetype=obj %s -o %t.64.pico
# RUN: ld.lld -shared %t.64.pico %t1.64.so -o %t.64.so
# RUN: llvm-readobj -r --cap-relocs %t.64.so | FileCheck --check-prefix=RV64-SO-REL %s
# RUN: llvm-readelf -x .got -x .tgot %t.64.so | FileCheck --check-prefix=RV64-SO-GOT %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64.so | FileCheck --check-prefix=RV64-SO-DIS %s

# RV32-REL:      .rela.tgot {
# RV32-REL-NEXT:   0x101F0 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV32-REL-NEXT: }
# RV32-REL:      __tgot_cap_relocs {
# RV32-REL-NEXT:   0x101F8 DATA - 0x4 [0x4-0x8]
# RV32-REL-NEXT: }

# RV32-SO-REL:      .rela.dyn {
# RV32-SO-REL-NEXT:   0x2428 R_RISCV_CHERI_TLS_TGOTREL - 0x0
# RV32-SO-REL-NEXT:   0x2430 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT:   0x2438 R_RISCV_CHERI_TLS_TGOTREL - 0x8
# RV32-SO-REL-NEXT:   0x2440 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT: }
# RV32-SO-REL:      .rela.tgot {
# RV32-SO-REL-NEXT:   0x248 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV32-SO-REL-NEXT: }
# RV32-SO-REL:      __tgot_cap_relocs {
# RV32-SO-REL-NEXT:   0x250 DATA - 0x4 [0x4-0x8]
# RV32-SO-REL-NEXT: }

# RV32-GOT: section '.tgot':
# RV32-GOT-NEXT: 0x000101f0 00000000 00000000 00000000 00000000
# RV32-GOT: section '.got':
# RV32-GOT-NEXT: 0x00012230 00240100 00000000 00000000 00000000
# RV32-GOT-NEXT: 0x00012240 01000000 00000000 08000000 00000000
# RV32-GOT-NEXT: 0x00012250 01000000 08000000

# RV32-SO-GOT: section '.tgot':
# RV32-SO-GOT-NEXT: 0x00000248 00000000 00000000 00000000 00000000
# RV32-SO-GOT: section '.got':
# RV32-SO-GOT-NEXT: 0x00002420 00260000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002430 00000000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002440 00000000 08000000

# 0x12240 - 0x11200 = 0x1040 (GD evar)
# RV32-DIS:      11200: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 64

# 0x12238 - 0x11208 = 0x1030 (IE evar)
# RV32-DIS:      11208: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 48(ca0)

# 0x12250 - 0x11210 = 0x1040 (GD lvar)
# RV32-DIS:      11210: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 64

# 0x12448 - 0x11218 = 0x1030 (IE lvar)
# RV32-DIS:      11218: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 48(ca0)

# RV32-DIS:      11220: lui a0, 0
# RV32-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV32-DIS-NEXT:        lc ca0, 8(ca0)

# 0x2430 - 0x1400 = 0x1030 (GD evar)
# RV32-SO-DIS:      1400: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 48

# 0x2428 - 0x1408 = 0x1020 (IE evar)
# RV32-SO-DIS:      1408: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 32(ca0)

# 0x2440 - 0x1410 = 0x1030 (GD lvar)
# RV32-SO-DIS:      1410: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 48

# 0x2438 - 0x1418 = 0x1020 (IE lvar)
# RV32-SO-DIS:      1418: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 32(ca0)

# RV64-REL:      .rela.tgot {
# RV64-REL-NEXT:   0x10320 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV64-REL-NEXT: }
# RV64-REL:      __tgot_cap_relocs {
# RV64-REL-NEXT:   0x10330 DATA - 0x4 [0x4-0x8]
# RV64-REL-NEXT: }

# RV64-SO-REL:      .rela.dyn {
# RV64-SO-REL-NEXT:   0x2400 R_RISCV_CHERI_TLS_TGOTREL - 0x0
# RV64-SO-REL-NEXT:   0x2410 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT:   0x2420 R_RISCV_CHERI_TLS_TGOTREL - 0x10
# RV64-SO-REL-NEXT:   0x2430 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT: }
# RV64-SO-REL:      .rela.tgot {
# RV64-SO-REL-NEXT:   0x3B0 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV64-SO-REL-NEXT: }
# RV64-SO-REL:      __tgot_cap_relocs {
# RV64-SO-REL-NEXT:   0x3C0 DATA - 0x4 [0x4-0x8]
# RV64-SO-REL-NEXT: }

# RV64-GOT: section '.tgot':
# RV64-GOT-NEXT: 0x00010320 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00010330 00000000 00000000 00000000 00000000
# RV64-GOT: section '.got':
# RV64-GOT-NEXT: 0x00012370 c0230100 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012380 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012390 01000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000123a0 10000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000123b0 01000000 00000000 10000000 00000000

# RV64-SO-GOT: section '.tgot':
# RV64-SO-GOT-NEXT: 0x000003b0 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x000003c0 00000000 00000000 00000000 00000000
# RV64-SO-GOT: section '.got':
# RV64-SO-GOT-NEXT: 0x000023f0 40240000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002400 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002410 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002420 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002430 00000000 00000000 10000000 00000000

# 0x12390 - 0x11340 = 0x1050 (GD evar)
# RV64-DIS:      11340: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 80

# 0x12380 - 0x11348 = 0x1038 (IE evar)
# RV64-DIS:      11348: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 56(ca0)

# 0x123b0 - 0x11350 = 0x1060 (GD lvar)
# RV64-DIS:      11350: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 96

# 0x122a0 - 0x11358 = 0x1048 (IE lvar)
# RV64-DIS:      11358: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 72(ca0)

# RV64-DIS:      11360: lui a0, 0
# RV64-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV64-DIS-NEXT:        lc ca0, 16(ca0)

# 0x2410 - 0x13d0 = 0x1040 (GD evar)
# RV64-SO-DIS:      13d0: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 64

# 0x2400 - 0x13d8 = 0x1028 (IE evar)
# RV64-SO-DIS:      13d8: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 40(ca0)

# 0x2430 - 0x13e0 = 0x1050 (GD lvar)
# RV64-SO-DIS:      13e0: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 80

# 0x2420 - 0x13e8 = 0x1038 (IE lvar)
# RV64-SO-DIS:      13e8: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 56(ca0)

.global _start
_start:
	clc.tls.gd ca0, evar

	cla.tls.ie a0, evar

	clc.tls.gd ca0, lvar

	cla.tls.ie a0, lvar

.if PIC == 0
	lui a0, %tgot_tprel_hi(lvar)
	cincoffset ca0, ctp, a0, %tgot_tprel_add(lvar)
	clc ca0, %tgot_tprel_lo(lvar)(ca0)
.endif

.tbss
	.zero 4
lvar:
	.zero 4
	.size lvar, . - lvar
