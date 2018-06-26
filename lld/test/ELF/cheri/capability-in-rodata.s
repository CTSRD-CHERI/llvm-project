# Check that capability relocations in .rodata sections cause errors (since they
# will trap when being processed at runtime). This is true both for static as well
# as dynamic binaries. Currently the kernel just gives us a read-write mapping to
# work around this but we really should just make this an error

# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUNNOT: llvm-readobj -r %t.o
# RUN: not ld.lld -shared %t.o -o %t.so 2>&1 | FileCheck %s
# RUN: ld.lld -shared %t.o -o %t.so -z notext
# RUN: llvm-objdump -C -s -t %t.so | FileCheck %s -check-prefix SHLIB
# RUN: not ld.lld -static %t.o -o %t.exe 2>&1 | FileCheck %s
# RUN: ld.lld -static %t.o -o %t.exe -z notext
# RUN: llvm-objdump -C -s -t %t.exe | FileCheck %s -check-prefix EXE

.text
.global __start
.protected __start
.ent __start
__start:
  nop
  nop
  nop
  nop
.end __start

.rodata
.space 16

.type	foo,@object
.global foo
.p2align 5
foo:
  .chericap __start + 0x4
# CHECK: error: attempting to add a capability relocation against symbol __start in a read-only section; pass -Wl,-z,notext if you really want to do this
# CHECK-NEXT: >>> referenced by object foo
# CHECK-NEXT: >>> defined in  ({{.+}}capability-in-rodata.s.tmp.o:(.rodata+0x20))

# EXE: CAPABILITY RELOCATION RECORDS:
# EXE: 0x00000001200001c0	Base: __start (0x0000000120010000)	Offset: 0x0000000000000004	Length: 0x0000000000000010	Permissions: 0x8000000000000000 (Function)

# SHLIB: CAPABILITY RELOCATION RECORDS:
# SHLIB: 0x00000000000002{{.+}}	Base: __start (0x0000000000010000)	Offset: 0x0000000000000004	Length: 0x0000000000000010	Permissions: 0x8000000000000000 (Function)
