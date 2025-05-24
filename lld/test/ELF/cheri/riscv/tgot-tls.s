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
# RV32-REL-NEXT:   0x132B8 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV32-REL-NEXT: }
# RV32-REL:      CHERI __tgot_cap_relocs [
# RV32-REL-NEXT:   0x0132c0 Base: 0x4 (lvar+0) Length: 4 Perms: Object
# RV32-REL-NEXT: ]

# RV32-SO-REL:      .rela.dyn {
# RV32-SO-REL-NEXT:   0x22F8 R_RISCV_CHERI_TLS_TGOTREL - 0x0
# RV32-SO-REL-NEXT:   0x2300 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT:   0x2308 R_RISCV_CHERI_TLS_TGOTREL - 0x8
# RV32-SO-REL-NEXT:   0x2310 R_RISCV_TLS_DTPMOD32 - 0x0
# RV32-SO-REL-NEXT: }
# RV32-SO-REL:      .rela.tgot {
# RV32-SO-REL-NEXT:   0x3318 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV32-SO-REL-NEXT: }
# RV32-SO-REL:      CHERI __tgot_cap_relocs [
# RV32-SO-REL-NEXT:   0x003320 Base: 0x4 (lvar+0) Length: 4 Perms: Object
# RV32-SO-REL-NEXT: ]

# RV32-GOT: section '.got':
# RV32-GOT-NEXT: 0x00012290 1c220100 00000000 00000000 00000000
# RV32-GOT-NEXT: 0x000122a0 01000000 00000000 08000000 00000000
# RV32-GOT-NEXT: 0x000122b0 01000000 08000000
# RV32-GOT: section '.tgot':
# RV32-GOT-NEXT: 0x000132b8 00000000 00000000 00000000 00000000

# RV32-SO-GOT: section '.got':
# RV32-SO-GOT-NEXT: 0x000022f0 64220000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002300 00000000 00000000 00000000 00000000
# RV32-SO-GOT-NEXT: 0x00002310 00000000 08000000
# RV32-SO-GOT: section '.tgot':
# RV32-SO-GOT-NEXT: 0x00003318 00000000 00000000 00000000 00000000

# 0x122a0 - 0x111f0 = 0x10b0 (GD evar)
# RV32-DIS:      111f0: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 176

# 0x12298 - 0x111f8 = 0x10a0 (IE evar)
# RV32-DIS:      111f8: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 160(ca0)

# 0x122b0 - 0x11200 = 0x10b0 (GD lvar)
# RV32-DIS:      11200: auipcc ca0, 1
# RV32-DIS-NEXT:        cincoffset ca0, ca0, 176

# 0x122a8 - 0x11208 = 0x10a0 (IE lvar)
# RV32-DIS:      11208: auipcc ca0, 1
# RV32-DIS-NEXT:        lw a0, 160(ca0)

# RV32-DIS:      11210: lui a0, 0
# RV32-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV32-DIS-NEXT:        lc ca0, 8(ca0)

# 0x2300 - 0x1244 = 0x10bc (GD evar)
# RV32-SO-DIS:      1244: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 188

# 0x22f8 - 0x124c = 0x10ac (IE evar)
# RV32-SO-DIS:      124c: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 172(ca0)

# 0x2310 - 0x1254 = 0x10bc (GD lvar)
# RV32-SO-DIS:      1254: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       cincoffset ca0, ca0, 188

# 0x2308 - 0x125c = 0x10ac (IE lvar)
# RV32-SO-DIS:      125c: auipcc ca0, 1
# RV32-SO-DIS-NEXT:       lw a0, 172(ca0)

# RV64-REL:      .rela.tgot {
# RV64-REL-NEXT:   0x13480 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV64-REL-NEXT: }
# RV64-REL:      CHERI __tgot_cap_relocs [
# RV64-REL-NEXT:   0x013490 Base: 0x4 (lvar+0) Length: 4 Perms: Object
# RV64-REL-NEXT: ]

# RV64-SO-REL:      .rela.dyn {
# RV64-SO-REL-NEXT:   0x24F0 R_RISCV_CHERI_TLS_TGOTREL - 0x0
# RV64-SO-REL-NEXT:   0x2500 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT:   0x2510 R_RISCV_CHERI_TLS_TGOTREL - 0x10
# RV64-SO-REL-NEXT:   0x2520 R_RISCV_TLS_DTPMOD64 - 0x0
# RV64-SO-REL-NEXT: }
# RV64-SO-REL:      .rela.tgot {
# RV64-SO-REL-NEXT:   0x3530 R_RISCV_CHERI_TLS_TGOT_SLOT evar 0x0
# RV64-SO-REL-NEXT: }
# RV64-SO-REL:      CHERI __tgot_cap_relocs [
# RV64-SO-REL-NEXT:   0x003540 Base: 0x4 (lvar+0) Length: 4 Perms: Object
# RV64-SO-REL-NEXT: ]

# RV64-GOT: section '.got':
# RV64-GOT-NEXT: 0x00012430 50230100 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012440 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012450 01000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012460 10000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00012470 01000000 00000000 10000000 00000000
# RV64-GOT: section '.tgot':
# RV64-GOT-NEXT: 0x00013480 00000000 00000000 00000000 00000000
# RV64-GOT-NEXT: 0x00013490 00000000 00000000 00000000 00000000

# RV64-SO-GOT: section '.got':
# RV64-SO-GOT-NEXT: 0x000024e0 d0230000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x000024f0 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002500 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002510 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00002520 00000000 00000000 10000000 00000000
# RV64-SO-GOT: section '.tgot':
# RV64-SO-GOT-NEXT: 0x00003530 00000000 00000000 00000000 00000000
# RV64-SO-GOT-NEXT: 0x00003540 00000000 00000000 00000000 00000000

# 0x12450 - 0x11320 = 0x1130 (GD evar)
# RV64-DIS:      11320: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 304

# 0x12440 - 0x11328 = 0x1118 (IE evar)
# RV64-DIS:      11328: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 280(ca0)

# 0x12470 - 0x11330 = 0x1140 (GD lvar)
# RV64-DIS:      11330: auipcc ca0, 1
# RV64-DIS-NEXT:        cincoffset ca0, ca0, 320

# 0x12460 - 0x11338 = 0x1128 (IE lvar)
# RV64-DIS:      11338: auipcc ca0, 1
# RV64-DIS-NEXT:        ld a0, 296(ca0)

# RV64-DIS:      11340: lui a0, 0
# RV64-DIS-NEXT:        cincoffset ca0, ctp, a0
# RV64-DIS-NEXT:        lc ca0, 16(ca0)

# 0x2500 - 0x13b0 = 0x1150 (GD evar)
# RV64-SO-DIS:      13b0: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 336

# 0x24f0 - 0x13b8 = 0x1138 (IE evar)
# RV64-SO-DIS:      13b8: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 312(ca0)

# 0x2520 - 0x13c0 = 0x1160 (GD lvar)
# RV64-SO-DIS:      13c0: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       cincoffset ca0, ca0, 352

# 0x2510 - 0x13c8 = 0x1148 (IE lvar)
# RV64-SO-DIS:      13c8: auipcc ca0, 1
# RV64-SO-DIS-NEXT:       ld a0, 328(ca0)

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
