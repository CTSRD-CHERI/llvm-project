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
# RV32-REL-NEXT:   0x12298 R_RISCV_TLS_DTPMOD32 evar 0x0
# RV32-REL-NEXT:   0x1229C R_RISCV_TLS_DTPREL32 evar 0x0
# RV32-REL-NEXT:   0x122A0 R_RISCV_TLS_TPREL32 evar 0x0
# RV32-REL-NEXT: }

# RV32-SO-REL:      .rela.dyn {
# RV32-SO-REL-NEXT:   0x2498 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT:   0x24A0 R_RISCV_TLS_TPREL32 - 0x4
# RV32-SO-REL-NEXT:   0x2488 R_RISCV_TLS_DTPMOD32 evar 0x0
# RV32-SO-REL-NEXT:   0x248C R_RISCV_TLS_DTPREL32 evar 0x0
# RV32-SO-REL-NEXT:   0x2490 R_RISCV_TLS_TPREL32 evar 0x0
# RV32-SO-REL-NEXT: }

# RV32-GOT: section '.got':
# RV32-GOT-NEXT: 0x00012290 2c220100 00000000 00000000 00000000
# RV32-GOT-NEXT: 0x000122a0 00000000 00000000 01000000 04000000
# RV32-GOT-NEXT: 0x000122b0 04000000 00000000

# RV32-SO-GOT: section '.got':
# RV32-SO-GOT-NEXT: 0x00002480 20240000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002490 00000000 00000000 00000000 04000000
# RV32-SO-GOT-NEXT: 0x000024a0 00000000 00000000

# 0x12298 - 0x11200 = 0x1098 (GD evar)
# RV32-DIS:      11200: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 152

# 0x122a0 - 0x11208 = 0x1098 (IE evar)
# RV32-DIS:      11208: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 152(ca0)

# 0x122a8 - 0x11210 = 0x1098 (GD lvar)
# RV32-DIS:      11210: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 152

# 0x122b0 - 0x11218 = 0x1098 (IE lvar)
# RV32-DIS:      11218: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 152(ca0)

# RV32-DIS:      11220: lui a0, 0
# RV32-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 4

# 0x2488 - 0x1400 = 0x1088 (GD evar)
# RV32-SO-DIS:      1400: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 136

# 0x2490 - 0x1408 = 0x1088 (IE evar)
# RV32-SO-DIS:      1408: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 136(ca0)

# 0x2498 - 0x1410 = 0x1088 (GD lvar)
# RV32-SO-DIS:      1410: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 136

# 0x24a0 - 0x1418 = 0x1088 (IE lvar)
# RV32-SO-DIS:      1418: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 136(ca0)

# RV64-REL:      .rela.dyn {
# RV64-REL-NEXT:   0x123F0 R_RISCV_TLS_DTPMOD64 evar 0x0
# RV64-REL-NEXT:   0x123F8 R_RISCV_TLS_DTPREL64 evar 0x0
# RV64-REL-NEXT:   0x12400 R_RISCV_TLS_TPREL64 evar 0x0
# RV64-REL-NEXT: }

# RV64-SO-REL:      .rela.dyn {
# RV64-SO-REL-NEXT:   0x2460 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT:   0x2470 R_RISCV_TLS_TPREL64 - 0x4
# RV64-SO-REL-NEXT:   0x2440 R_RISCV_TLS_DTPMOD64 evar 0x0
# RV64-SO-REL-NEXT:   0x2448 R_RISCV_TLS_DTPREL64 evar 0x0
# RV64-SO-REL-NEXT:   0x2450 R_RISCV_TLS_TPREL64 evar 0x0
# RV64-SO-REL-NEXT: }

# RV64-GOT: section '.got':
# RV64-GOT-NEXT: 0x000123e0 20230100 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000123f0 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012400 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012410 01000000 00000000 04000000 00000000
# RV64-GOT-NEXT: 0x00012420 04000000 00000000 00000000 00000000

# RV64-SO-GOT: section '.got':
# RV64-SO-GOT-NEXT: 0x00002430 70230000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002440 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002450 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002460 00000000 00000000 04000000 00000000
# RV64-SO-GOT-NEXT: 0x00002470 00000000 00000000 00000000 00000000

# 0x123f0 - 0x112f0 = 0x1100 (GD evar)
# RV64-DIS:      112f0: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 256

# 0x12400 - 0x112f8 = 0x1108 (IE evar)
# RV64-DIS:      112f8: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 264(ca0)

# 0x12410 - 0x11300 = 0x1110 (GD lvar)
# RV64-DIS:      11300: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 272

# 0x12420 - 0x11308 = 0x1118 (IE lvar)
# RV64-DIS:      11308: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 280(ca0)

# RV64-DIS:      11310: lui a0, 0
# RV64-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 4

# 0x2440 - 0x1350 = 0x10f0 (GD evar)
# RV64-SO-DIS:      1350: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 240

# 0x2450 - 0x1358 = 0x10f8 (IE evar)
# RV64-SO-DIS:      1358: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 248(ca0)

# 0x2460 - 0x1360 = 0x1100 (GD lvar)
# RV64-SO-DIS:      1360: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 256

# 0x2470 - 0x1368 = 0x1108 (IE lvar)
# RV64-SO-DIS:      1368: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 264(ca0)

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
