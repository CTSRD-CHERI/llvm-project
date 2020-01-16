## Check that LLD emits a warning when emitting a relocation against function+offset
## This may no longer work in the future since it's not ideal when using sentries
## and tightly bounded code capabilities.

# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: %cheri128_purecap_llvm-mc %s -defsym=SHLIB=1 -filetype=obj -o %t-lib.o
# RUN: ld.lld %t-lib.o -shared -o %t-lib.so
# RUN: llvm-readobj --cap-relocs -r %t.o | FileCheck %s --check-prefix=OBJ
# RUN: ld.lld %t.o %t-lib.so -shared -o %t.so 2>&1 | FileCheck %s --check-prefix=WARNING-MSG
# RUN: llvm-readobj --cap-relocs -r --sections --section-data %t.so | FileCheck %s

.type undef_func,@function
.global undef_func

.macro define_func name
.type \name,@function
.ent \name
\name :
  nop
.end \name
.endm

.ifdef SHLIB
.global external_func
define_func external_func

.else

.text
foo:
nop

.hidden hidden_local_func
define_func hidden_local_func

.protected protected_local_func
define_func protected_local_func

.global exported_local_func
define_func exported_local_func

.global weak_exported_local_func
.weak weak_exported_local_func
define_func weak_exported_local_func

.data
.type bad_relocs,@object
bad_relocs:
.8byte hidden_local_func -0x1
.chericap hidden_local_func + 0x1
.chericap protected_local_func + 0x2
.chericap exported_local_func + 0x3
# WARNING-MSG: warning: got capability relocation with non-zero addend (0x3) against function exported_local_func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp.o
# WARNING-MSG-NEXT: >>> referenced by {{.+}}function-with-offset-reloc.s.tmp.o:(.data+0x30)
# WARNING-MSG-EMPTY:
.chericap external_func + 0x4
# WARNING-MSG-NEXT: warning: got capability relocation with non-zero addend (0x4) against function external_func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp-lib.so
# WARNING-MSG-NEXT: >>> referenced by {{.+}}function-with-offset-reloc.s.tmp.o:(.data+0x40)
# WARNING-MSG-EMPTY:
.chericap undef_func + 0x5
# WARNING-MSG-NEXT: warning: got capability relocation with non-zero addend (0x5) against function undef_func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp.o
# WARNING-MSG-NEXT: >>> referenced by {{.+}}function-with-offset-reloc.s.tmp.o:(.data+0x50)
# WARNING-MSG-EMPTY:
.chericap weak_exported_local_func + 0x6
# WARNING-MSG-NEXT: warning: got capability relocation with non-zero addend (0x6) against function weak_exported_local_func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp.o
# WARNING-MSG-NEXT: >>> referenced by {{.+}}function-with-offset-reloc.s.tmp.o:(.data+0x60)
# WARNING-MSG-EMPTY:

.section .gcc_except_table,"awx",@progbits
except_table:
.8byte hidden_local_func -0x2
.chericap hidden_local_func + 0x10
.chericap protected_local_func + 0x11
.chericap exported_local_func + 0x12
## Note: no warning for exported_local_func
.chericap external_func + 0x13
# WARNING-MSG-NEXT: warning: got relocation against undefined symbol external_func in exception handling table
# WARNING-MSG-NEXT: warning: got capability relocation with non-zero addend (0x13) against function external_func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp-lib.so
# WARNING-MSG-NEXT: >>> referenced by {{.+}}function-with-offset-reloc.s.tmp.o:(.gcc_except_table+0x40)
# WARNING-MSG-EMPTY:
.chericap undef_func + 0x14
# WARNING-MSG-NEXT: warning: got relocation against undefined symbol undef_func in exception handling table
# WARNING-MSG-NEXT: warning: got capability relocation with non-zero addend (0x14) against function undef_func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp.o
# WARNING-MSG-NEXT: >>> referenced by {{.+}}function-with-offset-reloc.s.tmp.o:(.gcc_except_table+0x50)
# WARNING-MSG-EMPTY:
.chericap weak_exported_local_func + 0x15
# No warning here either since we want the table to refer to the local definition of the weak symbol

.endif

# OBJ-LABEL: Relocations [
# OBJ-LABEL:  Section ({{.+}}) .rela.data {
# OBJ-NEXT:    R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .text 0x3
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE hidden_local_func 0x1
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE protected_local_func 0x2
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE exported_local_func 0x3
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE external_func 0x4
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE undef_func 0x5
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE weak_exported_local_func 0x6
# OBJ-NEXT:  }
# OBJ-LABEL:  Section ({{.+}}) .rela.gcc_except_table {
# OBJ-NEXT:    R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .text 0x2
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE hidden_local_func 0x10
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE protected_local_func 0x11
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE exported_local_func 0x12
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE external_func 0x13
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE undef_func 0x14
# OBJ-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE weak_exported_local_func 0x15
# OBJ-NEXT:  }
# OBJ-NEXT: ]
# OBJ-NEXT: There is no __cap_relocs section in the file.

# CHECK:   Section {
# CHECK:      Index:
# CHECK:      Name: .gcc_except_table
# CHECK:      SectionData (
# CHECK-NEXT:   0000: 00000000 00010{{.+}} 00000000 00000000
# CHECK-NEXT:   0010: CACACACA CACACACA CACACACA CACACACA
##                            ^^ no addend since __cap_relocs is used (hidden_local_func)
# CHECK-NEXT:   0020: CACACACA CACACACA CACACACA CACACACA
##                            ^^ no addend since __cap_relocs is used (protected_local_func)
# CHECK-NEXT:   0030: CACACACA CACACACA CACACACA CACACACA
##                            ^^ no addend since __cap_relocs is used (exported_local_func)
##                               While this function is preemptible, we don't want it to be preemptible
##                               when used inside exception tables. Those should always refer to the local symbol.
# CHECK-NEXT:   0040: 00000000 00000013 CACACACA CACACACA
##                            ^^ addend 0x13 (external_func)
# CHECK-NEXT:   0050: 00000000 00000014 CACACACA CACACACA
##                            ^^ addend 0x14 (undef_func)
# CHECK-NEXT:   0060: CACACACA CACACACA CACACACA CACACACA
##                            ^^ no addend since __cap_relocs is used (weak_exported_local_func)
##                               While this function is preemptible, we don't want it to be preemptible
##                               when used inside exception tables. Those should always refer to the local symbol.
# CHECK-NEXT: )

# CHECK:   Section {
# CHECK:      Index:
# CHECK:      Name: .data
# CHECK:      SectionData (
# CHECK-NEXT:    0000: 00000000 00010{{.+}} 00000000 00000000  |
# CHECK-NEXT:    0010: CACACACA CACACACA CACACACA CACACACA  |
##                            ^^ no addend since __cap_relocs is used (hidden_local_func)
# CHECK-NEXT:    0020: CACACACA CACACACA CACACACA CACACACA  |
##                            ^^ no addend since __cap_relocs is used (protected_local_func)
# CHECK-NEXT:    0030: 00000000 00000003 CACACACA CACACACA  |
##                            ^^ addend 0x3 (exported_local_func) since it's preemptible
# CHECK-NEXT:    0040: 00000000 00000004 CACACACA CACACACA  |
##                            ^^ addend 0x4 (external_func)
# CHECK-NEXT:    0050: 00000000 00000005 CACACACA CACACACA  |
##                            ^^ addend 0x5 (undef_func)
# CHECK-NEXT:    0060: 00000000 00000006 CACACACA CACACACA  |
##                            ^^ addend 0x6 (weak_exported_local_func)
# CHECK-NEXT: )


# CHECK-LABEL:  Relocations [
# CHECK-NEXT:    Section ({{.+}}) .rel.dyn {
# CHECK-NEXT:      R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE exported_local_func 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE external_func 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE external_func 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE undef_func 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE undef_func 0x0 (real addend unknown)
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE weak_exported_local_func 0x0 (real addend unknown)
# CHECK-NEXT:    }
# CHECK-NEXT:  ]

# CHECK-LABEL: CHERI __cap_relocs [
## First the three .eh_frame relocations, including the fake symbol that was added to avoid relocations:
# CHECK-NEXT:    0x02{{.+}} Base: 0x{{.+}} (hidden_local_func+16) Length: 4 Perms: Function
# CHECK-NEXT:    0x02{{.+}} Base: 0x{{.+}} (protected_local_func+17) Length: 4 Perms: Function
# CHECK-NEXT:    0x02{{.+}} Base: 0x{{.+}} (__cheri_eh_exported_local_func+18) Length: 4 Perms: Function
# CHECK-NEXT:    0x02{{.+}} Base: 0x{{.+}} (__cheri_eh_weak_exported_local_func+21) Length: 4 Perms: Function
# CHECK-NEXT:    0x03{{.+}} Base: 0x{{.+}} (hidden_local_func+1) Length: 4 Perms: Function
# CHECK-NEXT:    0x03{{.+}} Base: 0x{{.+}} (protected_local_func+2) Length: 4 Perms: Function
# CHECK-NEXT: ]
