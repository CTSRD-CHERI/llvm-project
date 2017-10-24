// REQUIRES: clang

// RUN: %clang_cheri_purecap -mcpu=cheri -mllvm -cheri-test-mode %S/Inputs/external_lib_user.c -c -o %t.o
// RUN: %clang_cheri_purecap -mcpu=cheri -mllvm -cheri-test-mode %S/Inputs/external_lib.c -c -o %t-externs.o

// RUN: ld.lld -process-cap-relocs %t.o %t-externs.o -static -o %t-static.exe -e entry
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %s

// RUN: ld.lld -process-cap-relocs %t-externs.o -shared -o %t-externs.so
// RUN: llvm-objdump -C -t  %t-externs.so | FileCheck -check-prefixes DUMP-EXTERNALLIB %s

// RUN: ld.lld -process-cap-relocs %t-externs.so %t.o -o %t-dynamic.exe -e entry
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %s
// RUN: llvm-readobj -r %t-dynamic.exe | FileCheck -check-prefix DYNAMIC-EXE-RELOCS %s

// See what it looks like as a shared library
// RUN: ld.lld -process-cap-relocs %t-externs.so %t.o -shared -o %t.so
// RUN: llvm-readobj -r -dyn-relocations -s %t.so | FileCheck -check-prefixes SHLIB %s
// RUN: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-SHLIB %s

// check that we get an undefined symbol error:
// RUN: not ld.lld -process-cap-relocs %t.o -Bdynamic -o /dev/null -e entry 2>&1 | FileCheck %s -check-prefix UNDEFINED
// RUN: not ld.lld -process-cap-relocs %t.o -static -o /dev/null -e entry 2>&1 | FileCheck %s -check-prefix UNDEFINED
// RUN: not ld.lld -process-cap-relocs %t.o -shared -o /dev/null 2>&1 | FileCheck %s -check-prefix UNDEFINED

// STATIC: CAPABILITY RELOCATION RECORDS:
// STATIC: 0x0000000120020000	Base: external_cap (0x0000000120041100)	Offset: 0x0000000000000000	Length: 0x00000000000000{{1|2}}0	Permissions: 0x00000000
// STATIC: 0x0000000120020020	Base: external_buffer (0x0000000120040000)	Offset: 0x0000000000000019	Length: 0x00000000000010e1	Permissions: 0x00000000
// STATIC: 0x0000000120020040	Base: external_func (0x00000001200100a0)	Offset: 0x0000000000000000	Length: 0x0000000000000038	Permissions: 0x8000000000000000 (Function)

// The dynamic case fills in the offset length and permission fields but doesn't have the base yet
// DYNAMIC: CAPABILITY RELOCATION RECORDS:
// DYNAMIC: 0x0000000120020000	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x00000000000000{{1|2}}0	Permissions: 0x00000000
// DYNAMIC: 0x0000000120020020	Base:  (0x0000000000000000)	Offset: 0x0000000000000019	Length: 0x00000000000010e1	Permissions: 0x00000000
// DYNAMIC: 0x0000000120020040	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000038	Permissions: 0x8000000000000000 (Function)

// DYNAMIC-EXE-RELOCS: Section ({{.+}}) .rel.dyn {
// DYNAMIC-EXE-RELOCS-NEXT:    0x120030030 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE external_buffer 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x120030008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE external_cap 0x0
// DYNAMIC-EXE-RELOCS-NEXT:    0x120030058 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE external_func 0x0
// DYNAMIC-EXE-RELOCS-NEXT: }


// DUMP-EXTERNALLIB: SYMBOL TABLE:
// DUMP-EXTERNALLIB:  000000000030000 g       .bss		 000010e1 external_buffer
// DUMP-EXTERNALLIB:  0000000000031100 g       .bss		 00000020 external_cap
// DUMP-EXTERNALLIB:  0000000000010000 g     F .text		 00000038 external_func


// SHLIB:      Section (8) .rel.dyn {
// SHLIB-NEXT:   0x30000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
// SHLIB-NEXT:   0x30028 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
// SHLIB-NEXT:   0x30050 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
// SHLIB-NEXT:   0x30030 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE external_buffer 0x0
// SHLIB-NEXT:   0x30008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE external_cap 0x0
// SHLIB-NEXT:   0x30058 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE external_func 0x0
// SHLIB-NEXT: }

// DUMP-SHLIB: CAPABILITY RELOCATION RECORDS:
// DUMP-SHLIB: 0x0000000000020000      Base:  (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x00000000000000{{1|2}}0      Permissions: 0x00000000
// DUMP-SHLIB: 0x0000000000020020      Base:  (0x0000000000000000)     Offset: 0x0000000000000019      Length: 0x00000000000010e1      Permissions: 0x00000000
// DUMP-SHLIB: 0x0000000000020040      Base:  (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000038      Permissions: 0x8000000000000000 (Function)

// UNDEFINED:      error: cap_reloc against undefined symbol: external_cap
// UNDEFINED-NEXT: >>> referenced by local object (in current DSO) cap_ptr
// UNDEFINED-NEXT: >>> defined in {{.+}}/capreloc-externallib-integrated.c.tmp.o
// UNDEFINED: error: cap_reloc against undefined symbol: external_buffer
// UNDEFINED-NEXT: >>> referenced by local object (in current DSO) buffer_ptr
// UNDEFINED-NEXT: >>> defined in {{.+}}/capreloc-externallib-integrated.c.tmp.o
// UNDEFINED: error: cap_reloc against undefined symbol: external_func
// UNDEFINED-NEXT: >>> referenced by local object (in current DSO) func_ptr
// UNDEFINED-NEXT: >>> defined in {{.+}}/capreloc-externallib-integrated.c.tmp.o

