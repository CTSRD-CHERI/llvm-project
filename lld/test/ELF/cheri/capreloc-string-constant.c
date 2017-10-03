// REQUIRES: clang

// RUN: %clang_cheri_purecap %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix READOBJ %s

// RUN: ld.lld -process-cap-relocs %t.o -static -o %t-static.exe 2>&1 | FileCheck -check-prefixes UNKNOWN_LENGTH %s
// RUN: llvm-objdump -C %t-static.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC %s

// same again for statically dynamically linked exe:
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/integrated_dummy_shlib.o
// RUN: ld.lld -process-cap-relocs -pie -Bdynamic %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s
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
// READOBJ-NEXT:   Section (6) .rela.data {
// READOBJ-NEXT:     0x20 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str 0x0
// READOBJ-NEXT:     0x60 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.1 0x4
// READOBJ-NEXT:     0xA0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.2 0x0
// READOBJ-NEXT:     0xE0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.2 0x0
// READOBJ-NEXT:     0x120 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.2 0x1
// READOBJ-NEXT:   }
// READOBJ-NEXT: ]

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
// STATIC-NEXT: 0x00000001200100{{20|10}}      Base: <Unnamed symbol> (0x0000000120000190)     Offset: 0x0000000000000000      Length: 0x0000000000000006      Permissions: 0x00000000
// STATIC-NEXT: 0x00000001200100{{60|30}}      Base: <Unnamed symbol> (0x0000000120000196)     Offset: 0x0000000000000004      Length: 0x0000000000000007      Permissions: 0x00000000
// STATIC-NEXT: 0x00000001200100{{a0|50}}      Base: <Unnamed symbol> (0x000000012000019d)     Offset: 0x0000000000000000      Length: 0x000000000000000a      Permissions: 0x00000000
// STATIC-NEXT: 0x00000001200100{{e0|70}}      Base: <Unnamed symbol> (0x000000012000019d)     Offset: 0x0000000000000000      Length: 0x000000000000000a      Permissions: 0x00000000
// STATIC-NEXT: 0x0000000120010{{120|090}}      Base: <Unnamed symbol> (0x000000012000019d)     Offset: 0x0000000000000001      Length: 0x000000000000000a      Permissions: 0x00000000{{$}}

// PIE exe amd shlib should have dynamic relocations and only the offset values
// DYNAMIC-NEXT: 0x00000000000100{{20|10}}      Base: <Unnamed symbol> (0x00000000000001c8)     Offset: 0x0000000000000000      Length: 0x0000000000000006      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x00000000000100{{60|30}}      Base: <Unnamed symbol> (0x00000000000001ce)     Offset: 0x0000000000000004      Length: 0x0000000000000007      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x00000000000100{{a0|50}}      Base: <Unnamed symbol> (0x00000000000001d5)     Offset: 0x0000000000000000      Length: 0x000000000000000a      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x00000000000100{{e0|70}}      Base: <Unnamed symbol> (0x00000000000001d5)     Offset: 0x0000000000000000      Length: 0x000000000000000a      Permissions: 0x00000000
// DYNAMIC-NEXT: 0x0000000000010{{120|090}}      Base: <Unnamed symbol> (0x00000000000001d5)     Offset: 0x0000000000000001      Length: 0x000000000000000a      Permissions: 0x00000000{{$}}


// DUMP-CAPRELOCS-SAME:{{[[:space:]]$}}
