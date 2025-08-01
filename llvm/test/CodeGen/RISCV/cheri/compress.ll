; This test is designed to run twice, once with function attributes and once
; with target attributes added on the command line.
; See compress.ll in the folder above
;
; RUN: cat %s > %t.tgtattr
; RUN: echo 'attributes #0 = { nounwind }' >> %t.tgtattr
; RUN: %riscv32_cheri_purecap_llc -mattr=+c,+xcheri,+cap-mode -filetype=obj < %t.tgtattr \
; RUN:   | llvm-objdump -d -M no-aliases - | FileCheck %s
; RUN: %riscv64_cheri_purecap_llc -mattr=+c,+xcheri,+cap-mode -filetype=obj < %t.tgtattr \
; RUN:   | llvm-objdump -d -M no-aliases - | FileCheck %s
; RUN: %riscv64_cheri_purecap_llc -mattr=+c,+xcheri,+cap-mode,+xcheri-norvc -filetype=obj < %t.tgtattr \
; RUN:   | llvm-objdump -d -M no-aliases - | FileCheck %s --check-prefix=CHECK-NORVC

; RUN: cat %s > %t.fnattr
; RUN: echo 'attributes #0 = { nounwind "target-features"="+c,+xcheri,+cap-mode" }' >> %t.fnattr
; RUN: %riscv32_cheri_purecap_llc -filetype=obj < %t.fnattr \
; RUN:   | llvm-objdump -d --mattr=+c -M no-aliases - | FileCheck %s
; RUN: %riscv64_cheri_purecap_llc -filetype=obj < %t.fnattr \
; RUN:   | llvm-objdump -d --mattr=+c -M no-aliases - | FileCheck %s
; RUN: cat %s > %t.fnattr
; RUN: echo 'attributes #0 = { nounwind "target-features"="+c,+xcheri,+cap-mode,+xcheri-norvc" }' >> %t.fnattr
; RUN: %riscv64_cheri_purecap_llc -filetype=obj < %t.fnattr \
; RUN:   | llvm-objdump -d --mattr=+c -M no-aliases - | FileCheck %s --check-prefix=CHECK-NORVC

; Basic check that we can use CHERI compressed instructions

define i32 @loadstore(i32 addrspace(200)* %intptrarg, i8 addrspace(200)* addrspace(200)* %ptrptrarg) addrspace(200) #0 {
; CHECK-LABEL: <loadstore>:
; CHECK-NEXT:    {{[[:<:]]}}c.cincoffset16csp csp, -32
; CHECK-NEXT:    {{[[:<:]]}}c.lw a2, 0(ca0)
; CHECK-NEXT:    {{[[:<:]]}}c.li a3, 1
; CHECK-NEXT:    {{[[:<:]]}}c.sw a3, 0(ca0)
; CHECK-NEXT:    {{[[:<:]]}}c.lc ca0, 0(ca1)
; CHECK-NEXT:    {{[[:<:]]}}c.sc ca0, 0(ca1)
; CHECK-NEXT:    {{[[:<:]]}}c.scsp ca0, 16(csp)
; CHECK-NEXT:    {{[[:<:]]}}c.lcsp ca0, 16(csp)
; CHECK-NEXT:    {{[[:<:]]}}c.swsp a2, 0(csp)
; CHECK-NEXT:    {{[[:<:]]}}c.lwsp a0, 0(csp)
; CHECK-NEXT:    {{[[:<:]]}}c.mv a0, a2
; CHECK-NEXT:    {{[[:<:]]}}c.cincoffset16csp csp, 32
; CHECK-NEXT:    {{[[:<:]]}}c.jr cra
; CHECK-NORVC-LABEL: <loadstore>:
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset csp, csp, -32
; CHECK-NORVC-NEXT:  {{[[:<:]]}}lw a2, 0(ca0)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}c.li a3, 1
; CHECK-NORVC-NEXT:  {{[[:<:]]}}sw a3, 0(ca0)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}lc ca0, 0(ca1)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}sc ca0, 0(ca1)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}sc ca0, 16(csp)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}lc ca0, 16(csp)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}sw a2, 0(csp)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}lw a0, 0(csp)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}c.mv a0, a2
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset csp, csp, 32
; CHECK-NORVC-NEXT:  {{[[:<:]]}}jalr cnull, 0(cra)
  %stackptr = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %stackint = alloca i32, align 16, addrspace(200)
  %val = load volatile i32, i32 addrspace(200)* %intptrarg
  store volatile i32 1, i32 addrspace(200)* %intptrarg
  %ptrval = load volatile i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %ptrptrarg
  store volatile i8 addrspace(200)* %ptrval, i8 addrspace(200)* addrspace(200)* %ptrptrarg
  store volatile i8 addrspace(200)* %ptrval, i8 addrspace(200)* addrspace(200)* %stackptr
  %stackptrval = load volatile i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %stackptr
  store volatile i32 %val, i32 addrspace(200)* %stackint
  %stackintval = load volatile i32, i32 addrspace(200)* %stackint
  ret i32 %val
}

;; NB: No c.cjalr here, linker relaxations expect the full sequence.
define i32 @call() addrspace(200) #0 {
; CHECK-LABEL: <call>:
; CHECK-NEXT:    {{[[:<:]]}}c.cincoffset16csp csp, -{{112|144}}
; CHECK-NEXT:    {{[[:<:]]}}c.scsp cra, {{104|128}}(csp)
; CHECK-NEXT:    {{[[:<:]]}}c.cincoffset4cspn ca0, csp, 64
; CHECK-NEXT:    {{[[:<:]]}}csetbounds ca0, ca0, {{32|64}}
; CHECK-NEXT:    {{[[:<:]]}}cincoffset ca1, ca0, {{24|48}}
; CHECK-NEXT:    {{[[:<:]]}}cincoffset ca0, csp, 0
; CHECK-NEXT:    {{[[:<:]]}}csetbounds ca0, ca0, 64
; CHECK-NEXT:    {{[[:<:]]}}cincoffset ca0, ca0, 12
; CHECK-NEXT:    {{[[:<:]]}}auipcc cra, 0
; CHECK-NEXT:    {{[[:<:]]}}jalr cra, 0(cra)
; CHECK-NEXT:    {{[[:<:]]}}c.lcsp cra, {{104|128}}(csp)
; CHECK-NEXT:    {{[[:<:]]}}c.cincoffset16csp csp, {{112|144}}
; CHECK-NEXT:    {{[[:<:]]}}c.jr cra
; CHECK-NORVC-LABEL: <call>:
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset csp, csp, -144
; CHECK-NORVC-NEXT:  {{[[:<:]]}}sc cra, 128(csp)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset ca0, csp, 64
; CHECK-NORVC-NEXT:  {{[[:<:]]}}csetbounds ca0, ca0, 64
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset ca1, ca0, 48
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset ca0, csp, 0
; CHECK-NORVC-NEXT:  {{[[:<:]]}}csetbounds ca0, ca0, 64
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset ca0, ca0, 12
; CHECK-NORVC-NEXT:  {{[[:<:]]}}auipcc cra, 0
; CHECK-NORVC-NEXT:  {{[[:<:]]}}jalr cra, 0(cra)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}lc cra, 128(csp)
; CHECK-NORVC-NEXT:  {{[[:<:]]}}cincoffset csp, csp, 144
; CHECK-NORVC-NEXT:  {{[[:<:]]}}jalr cnull, 0(cra)

  %ptrarray = alloca [4 x i8 addrspace(200)*], align 16, addrspace(200)
  %intarray = alloca [16 x i32], align 1, addrspace(200)
  %ptrgep = getelementptr inbounds [4 x i8 addrspace(200)*], [4 x i8 addrspace(200)*] addrspace(200)* %ptrarray, i64 0, i64 3
  %intgep = getelementptr inbounds [16 x i32], [16 x i32] addrspace(200)* %intarray, i64 0, i64 3
  %ret = call i32 @loadstore(i32 addrspace(200)* %intgep, i8 addrspace(200)* addrspace(200)* %ptrgep)
  ret i32 %ret
}
