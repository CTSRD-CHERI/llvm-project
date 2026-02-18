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
# RV32-REL-NEXT:   0x12238 R_RISCV_TLS_DTPMOD32 evar 0x0
# RV32-REL-NEXT:   0x1223C R_RISCV_TLS_DTPREL32 evar 0x0
# RV32-REL-NEXT:   0x12240 R_RISCV_TLS_TPREL32 evar 0x0
# RV32-REL-NEXT: }

# RV32-SO-REL:      .rela.dyn {
# RV32-SO-REL-NEXT:   0x2438 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT:   0x2440 R_RISCV_TLS_TPREL32 - 0x4
# RV32-SO-REL-NEXT:   0x2428 R_RISCV_TLS_DTPMOD32 evar 0x0
# RV32-SO-REL-NEXT:   0x242C R_RISCV_TLS_DTPREL32 evar 0x0
# RV32-SO-REL-NEXT:   0x2430 R_RISCV_TLS_TPREL32 evar 0x0
# RV32-SO-REL-NEXT: }

# RV32-GOT: section '.got':
# RV32-GOT-NEXT: 0x00012230 00240100 00000000 00000000 00000000
# RV32-GOT-NEXT: 0x00012240 00000000 00000000 01000000 04000000
# RV32-GOT-NEXT: 0x00012250 04000000 00000000

# RV32-SO-GOT: section '.got':
# RV32-SO-GOT-NEXT: 0x00002420 00260000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002430 00000000 00000000 00000000 04000000
# RV32-SO-GOT-NEXT: 0x00002440 00000000 00000000

# 0x12238 - 0x11200 = 0x1038 (GD evar)
# RV32-DIS:      11200: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 56

# 0x12240 - 0x11208 = 0x1038 (IE evar)
# RV32-DIS:      11208: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 56(ca0)

# 0x12248 - 0x11210 = 0x1038 (GD lvar)
# RV32-DIS:      11210: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 56

# 0x12250 - 0x11218 = 0x1038 (IE lvar)
# RV32-DIS:      11218: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 56(ca0)

# RV32-DIS:      11220: lui a0, 0
# RV32-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 4

# 0x2428 - 0x1400 = 0x1028 (GD evar)
# RV32-SO-DIS:      1400: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 40

# 0x2430 - 0x1408 = 0x1028 (IE evar)
# RV32-SO-DIS:      1408: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 40(ca0)

# 0x2438 - 0x1410 = 0x1028 (GD lvar)
# RV32-SO-DIS:      1410: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 40

# 0x2440 - 0x1418 = 0x1028 (IE lvar)
# RV32-SO-DIS:      1418: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 40(ca0)

# RV64-REL:      .rela.dyn {
# RV64-REL-NEXT:   0x12330 R_RISCV_TLS_DTPMOD64 evar 0x0
# RV64-REL-NEXT:   0x12338 R_RISCV_TLS_DTPREL64 evar 0x0
# RV64-REL-NEXT:   0x12340 R_RISCV_TLS_TPREL64 evar 0x0
# RV64-REL-NEXT: }

# RV64-SO-REL:      .rela.dyn {
# RV64-SO-REL-NEXT:   0x23A0 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT:   0x23B0 R_RISCV_TLS_TPREL64 - 0x4
# RV64-SO-REL-NEXT:   0x2380 R_RISCV_TLS_DTPMOD64 evar 0x0
# RV64-SO-REL-NEXT:   0x2388 R_RISCV_TLS_DTPREL64 evar 0x0
# RV64-SO-REL-NEXT:   0x2390 R_RISCV_TLS_TPREL64 evar 0x0
# RV64-SO-REL-NEXT: }

# RV64-GOT: section '.got':
# RV64-GOT-NEXT: 0x00012320 70230100 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012330 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012340 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012350 01000000 00000000 04000000 00000000
# RV64-GOT-NEXT: 0x00012360 04000000 00000000 00000000 00000000

# RV64-SO-GOT: section '.got':
# RV64-SO-GOT-NEXT: 0x00002370 c0230000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002380 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002390 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x000023a0 00000000 00000000 04000000 00000000
# RV64-SO-GOT-NEXT: 0x000023b0 00000000 00000000 00000000 00000000

# 0x12330 - 0x112f0 = 0x1040 (GD evar)
# RV64-DIS:      112f0: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 64

# 0x12340 - 0x112f8 = 0x1048 (IE evar)
# RV64-DIS:      112f8: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 72(ca0)

# 0x12350 - 0x11300 = 0x1050 (GD lvar)
# RV64-DIS:      11300: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 80

# 0x12360 - 0x11308 = 0x1058 (IE lvar)
# RV64-DIS:      11308: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 88(ca0)

# RV64-DIS:      11310: lui a0, 0
# RV64-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 4

# 0x2380 - 0x1350 = 0x1030 (GD evar)
# RV64-SO-DIS:      1350: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 48

# 0x2390 - 0x1358 = 0x1038 (IE evar)
# RV64-SO-DIS:      1358: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 56(ca0)

# 0x23a0 - 0x1360 = 0x1040 (GD lvar)
# RV64-SO-DIS:      1360: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 64

# 0x23b0 - 0x1368 = 0x1048 (IE lvar)
# RV64-SO-DIS:      1368: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 72(ca0)

.global _start
_start:
	clc.tls.gd ca0, evar

	cla.tls.ie a0, evar

	clc.tls.gd ca0, lvar

	cla.tls.ie a0, lvar

.if PIC == 0
	lui a0, %tprel_hi(lvar)
	cincoffset ca0, ctp, a0, %tprel_add(lvar)
	cincoffset ca0, ca0, %tprel_lo(lvar)
.endif

.tbss
	.zero 4
lvar:
	.zero 4
