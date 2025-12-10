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
# RV32-REL-NEXT:   0x13800 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV32-REL-NEXT: }
# RV32-REL:      __tgot_cap_relocs {
# RV32-REL-NEXT:   0x13808 DATA - 0x4 [0x4-0x8]
# RV32-REL-NEXT: }

# RV32-SO-REL:      .rela.dyn {
# RV32-SO-REL-NEXT:   0x24C8 R_RISCV_CHERI_TLS_TGOTREL - 0x0
# RV32-SO-REL-NEXT:   0x24D0 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT:   0x24D8 R_RISCV_CHERI_TLS_TGOTREL - 0x8
# RV32-SO-REL-NEXT:   0x24E0 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT: }
# RV32-SO-REL:      .rela.tgot {
# RV32-SO-REL-NEXT:   0x3800 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV32-SO-REL-NEXT: }
# RV32-SO-REL:      __tgot_cap_relocs {
# RV32-SO-REL-NEXT:   0x3808 DATA - 0x4 [0x4-0x8]
# RV32-SO-REL-NEXT: }

# RV32-GOT: section '.got':
# RV32-GOT-NEXT: 0x000124b0 40240100 00000000 00000000 00000000
# RV32-GOT-NEXT: 0x000124c0 01000000 00000000 08000000 00000000
# RV32-GOT-NEXT: 0x000124d0 01000000 08000000
# RV32-GOT: section '.tgot':
# RV32-GOT-NEXT: 0x00013800 00000000 00000000 00000000 00000000

# RV32-SO-GOT: section '.got':
# RV32-SO-GOT-NEXT: 0x000024c0 34240000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x000024d0 00000000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x000024e0 00000000 08000000
# RV32-SO-GOT: section '.tgot':
# RV32-SO-GOT-NEXT: 0x00003800 00000000 00000000 00000000 00000000

# 0x124c0 - 0x11414 = 0x10ac (GD evar)
# RV32-DIS:      11414: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 172

# 0x124b8 - 0x1141c = 0x109c (IE evar)
# RV32-DIS:      1141c: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 156(ca0)

# 0x124d0 - 0x11424 = 0x10ac (GD lvar)
# RV32-DIS:      11424: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 172

# 0x124c8 - 0x1142c = 0x109c (IE lvar)
# RV32-DIS:      1142c: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 156(ca0)

# RV32-DIS:      11434: lui a0, 0
# RV32-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV32-DIS-NEXT:        lc ca0, 8(ca0)

# 0x24d0 - 0x1414 = 0x10bc (GD evar)
# RV32-SO-DIS:      1414: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 188

# 0x24c8 - 0x141c = 0x10ac (IE evar)
# RV32-SO-DIS:      141c: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 172(ca0)

# 0x24e0 - 0x1424 = 0x10bc (GD lvar)
# RV32-SO-DIS:      1424: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 188

# 0x24d8 - 0x142c = 0x10ac (IE lvar)
# RV32-SO-DIS:      142c: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 172(ca0)

# RV64-REL:      .rela.tgot {
# RV64-REL-NEXT:   0x134C0 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV64-REL-NEXT: }
# RV64-REL:      __tgot_cap_relocs {
# RV64-REL-NEXT:   0x134D0 DATA - 0x4 [0x4-0x8]
# RV64-REL-NEXT: }

# RV64-SO-REL:      .rela.dyn {
# RV64-SO-REL-NEXT:   0x2530 R_RISCV_CHERI_TLS_TGOTREL - 0x0
# RV64-SO-REL-NEXT:   0x2540 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT:   0x2550 R_RISCV_CHERI_TLS_TGOTREL - 0x10
# RV64-SO-REL-NEXT:   0x2560 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT: }
# RV64-SO-REL:      .rela.tgot {
# RV64-SO-REL-NEXT:   0x3570 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV64-SO-REL-NEXT: }
# RV64-SO-REL:      __tgot_cap_relocs {
# RV64-SO-REL-NEXT:   0x3580 DATA - 0x4 [0x4-0x8]
# RV64-SO-REL-NEXT: }

# RV64-GOT: section '.got':
# RV64-GOT-NEXT: 0x00012470 88230100 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012480 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012490 01000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000124a0 10000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000124b0 01000000 00000000 10000000 00000000
# RV64-GOT: section '.tgot':
# RV64-GOT-NEXT: 0x000134c0 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x000134d0 00000000 00000000 00000000 00000000

# RV64-SO-GOT: section '.got':
# RV64-SO-GOT-NEXT: 0x00002520 08240000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002530 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002540 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002550 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002560 00000000 00000000 10000000 00000000
# RV64-SO-GOT: section '.tgot':
# RV64-SO-GOT-NEXT: 0x00003570 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00003580 00000000 00000000 00000000 00000000

# 0x12490 - 0x11358 = 0x1138 (GD evar)
# RV64-DIS:      11358: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 312

# 0x12480 - 0x11360 = 0x1120 (IE evar)
# RV64-DIS:      11360: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 288(ca0)

# 0x124b0 - 0x11368 = 0x1148 (GD lvar)
# RV64-DIS:      11368: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 328

# 0x124a0 - 0x11370 = 0x1130 (IE lvar)
# RV64-DIS:      11370: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 304(ca0)

# RV64-DIS:      11378: lui a0, 0
# RV64-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV64-DIS-NEXT:        lc ca0, 16(ca0)

# 0x2540 - 0x13e8 = 0x1158 (GD evar)
# RV64-SO-DIS:      13e8: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 344

# 0x2530 - 0x13f0 = 0x1140 (IE evar)
# RV64-SO-DIS:      13f0: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 320(ca0)

# 0x2560 - 0x13f8 = 0x1168 (GD lvar)
# RV64-SO-DIS:      13f8: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 360

# 0x2550 - 0x1400 = 0x1150 (IE lvar)
# RV64-SO-DIS:      1400: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 336(ca0)

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
