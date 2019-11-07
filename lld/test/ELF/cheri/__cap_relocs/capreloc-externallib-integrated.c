// REQUIRES: clang

// RUN: %cheri256_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-obj %S/Inputs/external_lib_user.c -o %t.o
// RUN: %cheri256_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-obj %S/Inputs/external_lib.c -o %t-externs.o

// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t.o %t-externs.o -static -o %t-static.exe -e entry
// RUN: llvm-objdump -h -r -t --cap-relocs %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %s

// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t-externs.o -shared -o %t-externs.so
// RUN: llvm-objdump --cap-relocs -t  %t-externs.so | FileCheck -check-prefixes DUMP-EXTERNALLIB %s

// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t-externs.so %t.o -o %t-dynamic.exe -e entry
// RUN: llvm-objdump -h -r -t --cap-relocs %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %s
// RUN: llvm-readobj -r %t-dynamic.exe | FileCheck -check-prefix DYNAMIC-EXE-RELOCS %s

// See what it looks like as a shared library
// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t-externs.so %t.o -shared -o %t.so
// RUN: llvm-readobj -r -dyn-relocations -s %t.so | FileCheck -check-prefixes SHLIB %s
// RUN: llvm-objdump --cap-relocs -t %t.so | FileCheck -check-prefixes DUMP-SHLIB %s

// check that we get an undefined symbol error:
// RUN: not ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t.o --dynamic-linker /libexec/ld-elf.so -o /dev/null -e entry 2>&1 | FileCheck %s -check-prefix UNDEFINED
// RUN: not ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t.o -static -o /dev/null -e entry 2>&1 | FileCheck %s -check-prefix UNDEFINED
// RUN: not ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t.o -shared -no-undefined -o /dev/null 2>&1 | FileCheck %s -check-prefix UNDEFINED

// STATIC: CAPABILITY RELOCATION RECORDS:
// STATIC: 0x0000000120020320	Base: external_cap (0x00000001200214a0)	Offset: 0x0000000000000000	Length: 0x00000000000000{{1|2}}0	Permissions: 0x00000000
// STATIC: 0x0000000120020340	Base: external_buffer (0x00000001200203a0)	Offset: 0x0000000000000019	Length: 0x00000000000010e1	Permissions: 0x00000000
// STATIC: 0x0000000120020360	Base: external_func (0x00000001200{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000028	Permissions: 0x8000000000000000 (Function)

// The dynamic case fills in the offset length and permission fields but doesn't have the base yet
// DYNAMIC: CAPABILITY RELOCATION RECORDS:
// DYNAMIC: 0x0000000120030{{.+}}	Base: <Unnamed symbol> (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// DYNAMIC: 0x0000000120030{{.+}}	Base: <Unnamed symbol> (0x0000000000000000)	Offset: 0x0000000000000019	Length: 0x0000000000000000	Permissions: 0x00000000
// DYNAMIC: 0x0000000120030{{.+}}	Base: <Unnamed symbol> (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x8000000000000000 (Function)

// DYNAMIC-EXE-RELOCS: Section ({{.+}}) .rel.dyn {
// DYNAMIC-EXE-RELOCS-NEXT:    0x1200{{.+}} R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE external_buffer 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x1200{{.+}} R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE external_buffer 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x1200{{.+}} R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE external_cap 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x1200{{.+}} R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE external_cap 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x1200{{.+}} R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE external_func 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x1200{{.+}} R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE external_func 0x0
// DYNAMIC-EXE-RELOCS-NEXT: }


// DUMP-EXTERNALLIB: SYMBOL TABLE:
// DUMP-EXTERNALLIB:  0000000000020420 g     O .bss		 000010e1 external_buffer
// DUMP-EXTERNALLIB:  0000000000021520 g     O .bss		 00000020 external_cap
// DUMP-EXTERNALLIB:  00000000000103e0 g     F .text		 00000028 external_func


// SHLIB:      Section (8) .rel.dyn {
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE external_buffer 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE external_buffer 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE external_cap 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE external_cap 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE external_func 0x0
// SHLIB-NEXT:   0x20{{.+}} R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE external_func 0x0
// SHLIB-NEXT: }

// DUMP-SHLIB: CAPABILITY RELOCATION RECORDS:
// DUMP-SHLIB: 0x0000000000030{{.+}}      Base: <Unnamed symbol> (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// DUMP-SHLIB: 0x0000000000030{{.+}}      Base: <Unnamed symbol> (0x0000000000000000)     Offset: 0x0000000000000019      Length: 0x0000000000000000      Permissions: 0x00000000
// DUMP-SHLIB: 0x0000000000030{{.+}}      Base: <Unnamed symbol> (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x8000000000000000 (Function)

// UNDEFINED:      error: undefined symbol: external_cap
// UNDEFINED-NEXT: >>> referenced by {{(local object cap_ptr)|(external_lib_user.c)}}
// UNDEFINED-NEXT: >>> {{.+}}/capreloc-externallib-integrated.c.tmp.o:(cap_ptr)
// UNDEFINED:      error: undefined symbol: external_buffer
// UNDEFINED-NEXT: >>> referenced by {{(local object buffer_ptr)|(external_lib_user.c)}}
// UNDEFINED-NEXT: >>> {{.+}}/capreloc-externallib-integrated.c.tmp.o:(buffer_ptr)
// UNDEFINED:      error: undefined symbol: external_func
// UNDEFINED-NEXT: >>> referenced by {{(local object func_ptr)|(external_lib_user.c)}}
// UNDEFINED-NEXT: >>> {{.+}}/capreloc-externallib-integrated.c.tmp.o:(func_ptr)

