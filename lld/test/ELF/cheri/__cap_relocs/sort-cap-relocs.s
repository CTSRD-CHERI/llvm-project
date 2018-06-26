// RUN: %cheri128_purecap_llvm-mc -filetype=obj -defsym=FIRST=1 %s -o %t1.o
// RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t2.o
// RUN: echo "SECTIONS { \
// RUN:   .sec1 0x10000 : { sec1_start = .; *(.sec1) sec1_end = .;} \
// RUN:   .sec2 0x20000 : { sec2_start = .; *(.sec2) sec2_end = .;} \
// RUN:   .foo_sec 0x30000 : { *(.foo_sec) } \
// RUN:   .bar_sec 0x40000 : { *(.foo_sec) } \
// RUN: }" > %t.script

// First in the right order:
// RUN: ld.lld -preemptible-caprelocs=legacy %t1.o %t2.o -static -o %t.exe --script=%t.script --no-sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.exe | FileCheck %s -check-prefix UNSORTED1
// UNSORTED1-LABEL: CAPABILITY RELOCATION RECORDS:
// UNSORTED1-NEXT: 0x0000000000010040	Base: foo (0x0000000000030000)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)
// UNSORTED1-NEXT: 0x0000000000020020	Base: bar (0x0000000000040000)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)

// Now check that we put the 0x20020 __cap_reloc first:
// RUN: ld.lld -preemptible-caprelocs=legacy %t2.o %t1.o -static -o %t.exe --script=%t.script --no-sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.exe | FileCheck %s -check-prefix UNSORTED2
// UNSORTED2-LABEL: CAPABILITY RELOCATION RECORDS:
// UNSORTED2-NEXT: 0x0000000000020020	Base: bar (0x0000000000040000)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)
// UNSORTED2-NEXT: 0x0000000000010040	Base: foo (0x0000000000030000)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)

// check that both t1.o first and t2.o first produce the same result with --sort-cap-relocs
// RUN: ld.lld -preemptible-caprelocs=legacy %t1.o %t2.o -static -o %t.exe --script=%t.script --sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.exe | FileCheck %s -check-prefix SORTED
// RUN: ld.lld -preemptible-caprelocs=legacy %t2.o %t1.o -static -o %t.exe --script=%t.script --sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.exe | FileCheck %s -check-prefix SORTED
// SORTED-LABEL: CAPABILITY RELOCATION RECORDS:
// SORTED-NEXT: 0x0000000000010040	Base: foo (0x0000000000030000)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)
// SORTED-NEXT: 0x0000000000020020	Base: bar (0x0000000000040000)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)


// Check a shared library without cap_relocs sorting;
// RUN: ld.lld -preemptible-caprelocs=legacy %t1.o %t2.o -shared -o %t.so --script=%t.script --no-sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.so | FileCheck %s -check-prefix SHLIB-UNSORTED1
// SHLIB-UNSORTED1-LABEL: CAPABILITY RELOCATION RECORDS:
// SHLIB-UNSORTED1-NEXT: 0x0000000000010040	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x8000000000000000 (Function)
// SHLIB-UNSORTED1-NEXT: 0x0000000000020020	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x8000000000000000 (Function)
// SHLIB-UNSORTED1-LABEL: Sections:
// SHLIB-UNSORTED1-NEXT:  Idx Name          Size      Address          Type
// SHLIB-UNSORTED1-NEXT:    0 __cap_relocs  00000050 0000000000020040 DATA
// SHLIB-UNSORTED1-LABEL: Contents of section __cap_relocs:
// SHLIB-UNSORTED1-NEXT:  20040 00000000 00010040 00000000 00000000
// SHLIB-UNSORTED1-NEXT:  20050 00000000 00000000 00000000 00000000
// SHLIB-UNSORTED1-NEXT:  20060 80000000 00000000 00000000 00020020
// SHLIB-UNSORTED1-NEXT:  20070 00000000 00000000 00000000 00000000
// SHLIB-UNSORTED1-NEXT:  20080 00000000 00000000 80000000 00000000
// RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix SHLIB-RELOCS-UNSORTED1
// SHLIB-RELOCS-UNSORTED1-LABEL: Relocations [
// SHLIB-RELOCS-UNSORTED1-NEXT:    Section (16) .rel.dyn {
// SHLIB-RELOCS-UNSORTED1-NEXT:      0x20040 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED1-NEXT:      0x20068 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED1-NEXT:      0x20048 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED1-NEXT:      0x20058 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED1-NEXT:      0x20070 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE bar 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED1-NEXT:      0x20080 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE bar 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED1-NEXT:    }
// SHLIB-RELOCS-UNSORTED1-NEXT:  ]

// RUN: ld.lld -preemptible-caprelocs=legacy %t2.o %t1.o -shared -o %t.so --script=%t.script --no-sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.so 
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.so | FileCheck %s -check-prefix SHLIB-UNSORTED2
// SHLIB-UNSORTED2-LABEL: Sections:
// SHLIB-UNSORTED2-NEXT:  Idx Name          Size      Address          Type
// SHLIB-UNSORTED2-NEXT:    0 __cap_relocs  00000050 0000000000020040 DATA
// SHLIB-UNSORTED2-LABEL: Contents of section __cap_relocs:
// SHLIB-UNSORTED2-NEXT:  20040 00000000 00020020 00000000 00000000
// SHLIB-UNSORTED2-NEXT:  20050 00000000 00000000 00000000 00000000
// SHLIB-UNSORTED2-NEXT:  20060 80000000 00000000 00000000 00010040
// SHLIB-UNSORTED2-NEXT:  20070 00000000 00000000 00000000 00000000
// SHLIB-UNSORTED2-NEXT:  20080 00000000 00000000 80000000 00000000
// RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix SHLIB-RELOCS-UNSORTED2
// SHLIB-RELOCS-UNSORTED2-LABEL: Relocations [
// SHLIB-RELOCS-UNSORTED2-NEXT:    Section (16) .rel.dyn {
// SHLIB-RELOCS-UNSORTED2-NEXT:      0x20040 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED2-NEXT:      0x20068 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED2-NEXT:      0x20048 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE bar 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED2-NEXT:      0x20058 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE bar 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED2-NEXT:      0x20070 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED2-NEXT:      0x20080 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
// SHLIB-RELOCS-UNSORTED2-NEXT:    }
// SHLIB-RELOCS-UNSORTED2-NEXT:  ]


// Now check that we don't sort __cap_relocs (yet) when producing dynamic relocations:
// TODO: it would be nice to also sort them when emitting dynamic relocs but that needs some refactoring
// RUN: ld.lld -preemptible-caprelocs=legacy %t1.o %t2.o -shared -o %t.so --script=%t.script --sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.so | FileCheck %s -check-prefix SHLIB-UNSORTED1
// RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix SHLIB-RELOCS-UNSORTED1
// RUN: ld.lld -preemptible-caprelocs=legacy %t2.o %t1.o -shared -o %t.so --script=%t.script --sort-cap-relocs
// RUN: llvm-objdump -s --section=__cap_relocs -C -h %t.so | FileCheck %s -check-prefix SHLIB-UNSORTED2
// RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix SHLIB-RELOCS-UNSORTED2
// RUN: llvm-readobj -cheri-caprelocs -dyn-relocations  %t.so



.ifdef FIRST

.section .sec1, "aw"
.space 64
.global foo
.chericap foo

.section .foo_sec, "a"
.p2align 5
.ent foo
foo:
  li $a0, 1
.end foo

.else

.section .sec2, "aw"
.space 32
.global bar
.chericap bar


.section .bar_sec, "a"
.p2align 5
.ent bar
bar:
  li $a0, 2
.end bar

.endif
