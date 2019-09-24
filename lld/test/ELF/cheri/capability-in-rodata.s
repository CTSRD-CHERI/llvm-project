# Check that capability relocations in .rodata sections cause errors (since they
# will trap when being processed at runtime). This is true both for static as well
# as dynamic binaries. Currently the kernel just gives us a read-write mapping to
# work around this but we really should just make this an error

# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUNNOT: llvm-readobj -r %t.o
# RUN: not ld.lld -shared %t.o -o %t.so 2>&1 | FileCheck %s
# RUN: ld.lld -shared %t.o -o %t.so -z notext
# RUN: llvm-readobj --cap-relocs %t.so | FileCheck %s -check-prefix SHLIB
# RUN: not ld.lld -static %t.o -o %t.exe 2>&1 | FileCheck %s
# RUN: ld.lld -static %t.o -o %t.exe -z notext
# RUN: llvm-readobj --cap-relocs %t.exe | FileCheck %s -check-prefix EXE

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
.protected foo
.p2align 5
foo:
  .chericap __start + 0x4
.Lend_foo:
.size foo, .Lend_foo - foo

.type	bar,@object
.global bar
.p2align 5
bar:
  .chericap foo + 123
.Lend_bar:
.size bar, .Lend_bar - bar
# CHECK: error: attempting to add a capability relocation against symbol __start in a read-only section; pass -Wl,-z,notext if you really want to do this
# CHECK-NEXT: >>> referenced by object foo
# CHECK-NEXT: >>> defined in  ({{.+}}capability-in-rodata.s.tmp.o:(foo))
# CHECK-EMPTY:
# CHECK: error: attempting to add a capability relocation against symbol foo in a read-only section; pass -Wl,-z,notext if you really want to do this
# CHECK-NEXT: >>> referenced by object bar
# CHECK-NEXT: >>> defined in  ({{.+}}capability-in-rodata.s.tmp.o:(bar))
# CHECK-EMPTY:

# EXE: CHERI __cap_relocs [
# EXE-NEXT: 0x{{[0-9a-f]+}} (foo)           Base: 0x{{[0-9a-f]+}} (__start+4) Length: 16 Perms: Function
# EXE-NEXT: 0x{{[0-9a-f]+}} (bar)           Base: 0x{{[0-9a-f]+}} (foo+123) Length: {{16|32}} Perms: Constant
# EXE-NEXT: ]

# SHLIB: CHERI __cap_relocs [
# SHLIB-NEXT: 0x{{[0-9a-f]+}} (foo)           Base: 0x{{[0-9a-f]+}} (__start+4) Length: 16 Perms: Function
# SHLIB-NEXT: 0x{{[0-9a-f]+}} (bar)           Base: 0x{{[0-9a-f]+}} (foo+123) Length: {{16|32}} Perms: Constant
# SHLIB-NEXT: ]
