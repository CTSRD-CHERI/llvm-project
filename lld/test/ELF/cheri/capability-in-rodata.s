# Check that capability relocations in .rodata sections cause errors (since they
# will trap when being processed at runtime). This is true both for static as well
# as dynamic binaries. Currently the kernel just gives us a read-write mapping to
# work around this but we really should just make this an error

# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUNNOT: llvm-readobj -r %t.o
# RUN: not ld.lld -shared %t.o -o %t.so 2>&1 | FileCheck %s --check-prefix=ERR
# RUN: ld.lld -shared %t.o -o %t.so -z notext
# RUN: llvm-readobj -s --cap-relocs %t.so | FileCheck %s
# RUN: not ld.lld -static %t.o -o %t.exe 2>&1 | FileCheck %s --check-prefix=ERR
# RUN: ld.lld -static %t.o -o %t.exe -z notext
# RUN: llvm-readobj -s --cap-relocs %t.exe | FileCheck %s

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
# ERR: error: attempting to add a capability relocation against symbol __start in a read-only section; pass -Wl,-z,notext if you really want to do this
# ERR-NEXT: >>> referenced by object foo
# ERR-NEXT: >>> defined in  ({{.+}}capability-in-rodata.s.tmp.o:(foo))
# ERR-EMPTY:
# ERR: error: attempting to add a capability relocation against symbol foo in a read-only section; pass -Wl,-z,notext if you really want to do this
# ERR-NEXT: >>> referenced by object bar
# ERR-NEXT: >>> defined in  ({{.+}}capability-in-rodata.s.tmp.o:(bar))
# ERR-EMPTY:

# CHECK:      Name: __start
# CHECK-NEXT: Value: 0x[[#%X,START:]]
# CHECK:      Name: foo
# CHECK-NEXT: Value: 0x[[#%X,FOO:]]
# CHECK:      Name: bar
# CHECK-NEXT: Value: 0x[[#%X,BAR:]]

# CHECK:      CHERI Capability Relocations [
# CHECK-NEXT:   Section ({{.+}}) __cap_relocs {
# CHECK-NEXT:     0x[[#FOO]] FUNC - 0x[[#START+4]] [0x[[#START]]-0x[[#START+16]]]
# CHECK-NEXT:     0x[[#BAR]] RODATA - 0x[[#FOO+123]] [0x[[#FOO]]-0x[[#FOO+16]]]
# CHECK-NEXT:   }
# CHECK-NEXT: ]
