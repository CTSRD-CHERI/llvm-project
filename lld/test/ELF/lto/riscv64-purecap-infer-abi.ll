; REQUIRES: riscv

; RUN: llvm-as %s -o %t.o
; Note: -mattr is needed since there is no global metadata for the target features,
; but we should be able to infer target-abi from the first input file
; RUN: ld.lld %t.o -o %t -plugin-opt=-mattr=+xcheri -e purecap_fn
; RUN: llvm-readelf --syms --arch-specific %t | FileCheck %s
; RUN: llvm-objdump -d %t | FileCheck %s --check-prefix=DUMP
; CHECK: Symbol table '.symtab' contains 3 entries:
; CHECK: FUNC    GLOBAL DEFAULT     1 purecap_fn
; CHECK:      Attribute {
; CHECK:        Tag: 5
; CHECK-NEXT:   TagName: arch
; CHECK-NEXT:   Value: rv64i2p0_xcheri0p0
; CHECK-NEXT: }
; DUMP: 0000000000011120 <purecap_fn>:
; DUMP-NEXT: 11120: 5b 80 c0 fe  	cret

; RUN: llvm-as %S/riscv64.ll -o %t-nocheri.o
; RUN: not ld.lld -o %t -plugin-opt=-mattr=+xcheri %t.o %t-nocheri.o -e purecap_fn 2>&1 | FileCheck %s --check-prefix INCOMPATIBLE
; INCOMPATIBLE: warning: Linking two modules of different data layouts: '{{.+}}riscv64-purecap-infer-abi.ll.tmp-nocheri.o' is 'e-m:e-p:64:64-i64:64-i128:128-n64-S128' whereas 'ld-temp.o' is 'e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200'
; INCOMPATIBLE: warning: Linking two modules of different target triples: '{{.+}}riscv64-purecap-infer-abi.ll.tmp-nocheri.o' is 'riscv64-unknown-elf' whereas 'ld-temp.o' is 'riscv64-unknown-freebsd'
; INCOMPATIBLE: error: linking module flags 'target-abi': IDs have conflicting values in '{{.+}}riscv64-purecap-infer-abi.ll.tmp-nocheri.o' and 'ld-temp.o'

target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200"
target triple = "riscv64-unknown-freebsd"

define void @purecap_fn() addrspace(200) #0 {
entry:
  ret void
}

attributes #0 = { nounwind "target-features"="+cap-mode,+xcheri" }
!llvm.module.flags = !{!0}
!0 = !{i32 1, !"target-abi", !"l64pc128"}
