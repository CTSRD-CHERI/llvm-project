// REQUIRES: clang

// RUN: %clang_cheri_purecap %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix READOBJ %s
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix OBJ-CAPRELOCS %s

// RUN: ld.lld -process-cap-relocs %t.o -static -o %t-static.exe -verbose 2>&1 | FileCheck -check-prefixes UNKNOWN_LENGTH_VERBOSE %s
// RUN: ld.lld -process-cap-relocs %t.o -static -o %t-static.exe 2>&1 | FileCheck -check-prefixes UNKNOWN_LENGTH %s
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC %s

// same again for statically dynamically linked exe:
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/integrated_dummy_shlib.o
// RUN: ld.lld -process-cap-relocs -pie -Bdynamic %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s
// RUN: llvm-readobj -r -s %t-dynamic.exe | FileCheck -check-prefixes DYNAMIC-RELOCS %s

// Look at shared libraries:
// RUN: ld.lld -process-cap-relocs %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes DYNAMIC-RELOCS %s
// RUN: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s

// FIXME: it would be good if we could set bounds here instead of having it as -1


struct option {
    int id;
    const char* name;
};

struct option options_table[] = {
        { 1, "first" },
        { 2, "second" + 4 },
        { 3, "duplicate" },
        { 4, "duplicate" },
        { 5, "duplicate" + 1 },


};

// READOBJ:      Relocations [
// READOBJ-NEXT:   Section {{.+}} .rela__cap_relocs {
// READOBJ-NEXT:     0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x20
// READOBJ-NEXT:     0x8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x0
// READOBJ-NEXT:     0x28 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x60
// READOBJ-NEXT:     0x30 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x6
// READOBJ-NEXT:     0x50 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0xA0
// READOBJ-NEXT:     0x58 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xD
// READOBJ-NEXT:     0x78 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0xE0
// READOBJ-NEXT:     0x80 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xD
// READOBJ-NEXT:     0xA0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x120
// READOBJ-NEXT:     0xA8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xD
// READOBJ-NEXT:   }
// READOBJ-NEXT: ]

// OBJ-CAPRELOCS: CAPABILITY RELOCATION RECORDS:
// OBJ-CAPRELOCS: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0x0000000000000004      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0x0000000000000001      Length: 0x0000000000000000      Permissions: 0x00000000{{$}}
// OBJ-CAPRELOCS-SAME:{{[[:space:]]$}}


// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section (in current DSO) /Users/alex/cheri/llvm/tools/lld/test/ELF/cheri/capreloc-string-constant.c:(.rodata.str1.1+0x0)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section (in current DSO) /Users/alex/cheri/llvm/tools/lld/test/ELF/cheri/capreloc-string-constant.c:(.rodata.str1.1+0x6)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section (in current DSO) /Users/alex/cheri/llvm/tools/lld/test/ELF/cheri/capreloc-string-constant.c:(.rodata.str1.1+0xD)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section (in current DSO) /Users/alex/cheri/llvm/tools/lld/test/ELF/cheri/capreloc-string-constant.c:(.rodata.str1.1+0xD)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section (in current DSO) /Users/alex/cheri/llvm/tools/lld/test/ELF/cheri/capreloc-string-constant.c:(.rodata.str1.1+0xD)
// UNKNOWN_LENGTH: warning: cannot find entry symbol __start
// UNKNOWN_LENGTH-NOT: warning


// dynamic should have 10 relocations against the load address
// DYNAMIC-RELOCS-LABEL: Relocations [
// DYNAMIC-RELOCS-NEXT:   Section (8) .rel.dyn {
// DYNAMIC-RELOCS-NEXT:     0x20000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20028 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20030 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20050 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20058 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20078 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x20080 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x200A0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x200A8 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:   }
// DYNAMIC-RELOCS-NEXT: ]


// DUMP-CAPRELOCS-LABEL: CAPABILITY RELOCATION RECORDS:
// STATIC-NEXT: 0x0000000120010020      Base: <Unnamed symbol> (0x0000000120000190)     Offset: 0x0000000000000000      Length: 0x0000000000000017      Permissions: 0x00000000
// STATIC-NEXT: 0x0000000120010060      Base: <Unnamed symbol> (0x0000000120000196)     Offset: 0x0000000000000004      Length: 0x0000000000000017      Permissions: 0x00000000
// STATIC-NEXT: 0x00000001200100a0      Base: <Unnamed symbol> (0x000000012000019d)     Offset: 0x0000000000000000      Length: 0x0000000000000017      Permissions: 0x00000000
// STATIC-NEXT: 0x00000001200100e0      Base: <Unnamed symbol> (0x000000012000019d)     Offset: 0x0000000000000000      Length: 0x0000000000000017      Permissions: 0x00000000
// STATIC-NEXT: 0x0000000120010120      Base: <Unnamed symbol> (0x000000012000019d)     Offset: 0x0000000000000001      Length: 0x0000000000000017      Permissions: 0x00000000{{$}}

// PIE exe amd shlib should have dynamic relocations and only the offset values
// DYNAMIC-NEXT: 0x0000000000010020      Base: <Unnamed symbol> (0x00000000000001c8)     Offset: 0x0000000000000000      Length: 0x0000000000000017      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x0000000000010060      Base: <Unnamed symbol> (0x00000000000001ce)     Offset: 0x0000000000000004      Length: 0x0000000000000017      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x00000000000100a0      Base: <Unnamed symbol> (0x00000000000001d5)     Offset: 0x0000000000000000      Length: 0x0000000000000017      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x00000000000100e0      Base: <Unnamed symbol> (0x00000000000001d5)     Offset: 0x0000000000000000      Length: 0x0000000000000017      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x0000000000010120      Base: <Unnamed symbol> (0x00000000000001d5)     Offset: 0x0000000000000001      Length: 0x0000000000000017      Permissions: 0x00000000{{$}}
// DUMP-CAPRELOCS-SAME:{{[[:space:]]$}}