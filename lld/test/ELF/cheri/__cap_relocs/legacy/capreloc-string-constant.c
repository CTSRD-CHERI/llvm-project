// REQUIRES: clang

// RUN: %cheri_purecap_clang %legacy_caprelocs_flag %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix READOBJ %s
// RUN: llvm-objdump --cap-relocs -r %t.o | FileCheck -check-prefix OBJ-CAPRELOCS %s

// RUN: ld.lld --no-relative-cap-relocs -process-cap-relocs %t.o -static -o %t-static.exe -verbose 2>&1 | FileCheck -check-prefixes UNKNOWN_LENGTH_VERBOSE %s
// RUN: ld.lld --no-relative-cap-relocs -process-cap-relocs %t.o -static -o %t-static.exe 2>&1 | FileCheck -check-prefixes UNKNOWN_LENGTH %s
// RUN: llvm-readobj -s --cap-relocs %t-static.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC %s '-D$CONSTANT_FLAG=Constant'

// same again for statically dynamically linked exe:
// RUN: %cheri_purecap_clang %legacy_caprelocs_flag %S/../Inputs/dummy_shlib.c -c -o %T/integrated_dummy_shlib.o
// RUN: ld.lld --no-relative-cap-relocs -process-cap-relocs -pie -Bdynamic %t.o -o %t-dynamic.exe
// RUN: llvm-readobj --cap-relocs -s %t-dynamic.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s '-D$CONSTANT_FLAG=Constant'
// RUN: llvm-readobj -r -s %t-dynamic.exe | FileCheck -check-prefixes DYNAMIC-RELOCS %s

// Look at shared libraries:
// RUN: ld.lld --no-relative-cap-relocs -process-cap-relocs %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes DYNAMIC-RELOCS %s
// RUN: llvm-readobj --cap-relocs -s %t.so | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s '-D$CONSTANT_FLAG=Constant'

// RUN: ld.lld --no-relative-cap-relocs -no-process-cap-relocs %t.o -static -o %t-static-external-capsizefix.exe
// RUN: %capsizefix %t-static-external-capsizefix.exe
// RUN: llvm-readobj --cap-relocs -s %t-static-external-capsizefix.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC-EXTERNAL-CAPSIZEFIX %s '-D$CONSTANT_FLAG=Object'


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
// READOBJ-NEXT:     0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data {{0x20|0x10}}
// READOBJ-NEXT:     0x8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x0
// READOBJ-NEXT:     0x28 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data {{0x60|0x30}}
// READOBJ-NEXT:     0x30 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x6
// READOBJ-NEXT:     0x50 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data {{0xA0|0x50}}
// READOBJ-NEXT:     0x58 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xD
// READOBJ-NEXT:     0x78 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data {{0xE0|0x70}}
// READOBJ-NEXT:     0x80 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xD
// READOBJ-NEXT:     0xA0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data {{0x120|0x90}}
// READOBJ-NEXT:     0xA8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xD
// READOBJ-NEXT:   }
// READOBJ-NEXT: ]

// OBJ-CAPRELOCS: CAPABILITY RELOCATION RECORDS:
// OBJ-CAPRELOCS: 0x0000000000000000      Base: .rodata.str1.1 (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base: .rodata.str1.1 (0x0000000000000000)     Offset: 0x0000000000000004      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base: .rodata.str1.1 (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base: .rodata.str1.1 (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// OBJ-CAPRELOCS: 0x0000000000000000      Base: .rodata.str1.1 (0x0000000000000000)     Offset: 0x0000000000000001      Length: 0x0000000000000000      Permissions: 0x00000000{{$}}
// OBJ-CAPRELOCS-EMPTY:


// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section capreloc-string-constant.c:(.rodata.str1.1+0x0)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section capreloc-string-constant.c:(.rodata.str1.1+0x6)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section capreloc-string-constant.c:(.rodata.str1.1+0xD)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section capreloc-string-constant.c:(.rodata.str1.1+0xD)
// UNKNOWN_LENGTH_VERBOSE: warning: could not determine size of cap reloc against local section capreloc-string-constant.c:(.rodata.str1.1+0xD)
// UNKNOWN_LENGTH: warning: cannot find entry symbol __start
// UNKNOWN_LENGTH-NOT: warning


// dynamic should have 10 relocations against the load address
// DYNAMIC-RELOCS-LABEL: Relocations [
// DYNAMIC-RELOCS-NEXT:   Section ({{.+}}) .rel.dyn {
// DYNAMIC-RELOCS-NEXT:     0x10000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10028 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10030 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10050 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10058 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10078 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x10080 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x100A0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x100A8 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:   }
// DYNAMIC-RELOCS-NEXT: ]


// STATIC: Section {
// STATIC: Name: .rodata (
// STATIC-NEXT: Type: SHT_PROGBITS (0x1)
// STATIC-NEXT: Flags [ (0x32)
// STATIC-NEXT:   SHF_ALLOC (0x2)
// STATIC-NEXT:   SHF_MERGE (0x10)
// STATIC-NEXT:   SHF_STRINGS (0x20)
// STATIC-NEXT: ]
// STATIC-NEXT: Address: 0x120000208

// STATIC-LABEL: CHERI __cap_relocs [
// STATIC-NEXT: 0x1200100{{20|10}}  Base: 0x120000208     (<unknown symbol>+0) Length: 23 Perms: [[$CONSTANT_FLAG]]
// STATIC-NEXT: 0x1200100{{60|30}}  Base: 0x120000218 (<unknown symbol>+4) Length: 7  Perms: [[$CONSTANT_FLAG]]
// STATIC-NEXT: 0x1200100{{a0|50}}  Base: 0x12000020e  (<unknown symbol>+0) Length: 17 Perms: [[$CONSTANT_FLAG]]
// STATIC-NEXT: 0x1200100{{e0|70}}  Base: 0x12000020e  (<unknown symbol>+0) Length: 17 Perms: [[$CONSTANT_FLAG]]
// STATIC-NEXT: 0x120010{{120|090}} Base: 0x12000020e  (<unknown symbol>+1) Length: 17 Perms: [[$CONSTANT_FLAG]]
// STATIC-NEXT: ]

// DYNAMIC: Section {
// DYNAMIC:      Name: .rodata (
// DYNAMIC-NEXT: Type: SHT_PROGBITS (0x1)
// DYNAMIC-NEXT: Flags [ (0x32)
// DYNAMIC-NEXT:   SHF_ALLOC (0x2)
// DYNAMIC-NEXT:   SHF_MERGE (0x10)
// DYNAMIC-NEXT:   SHF_STRINGS (0x20)
// DYNAMIC-NEXT: ]
// DYNAMIC-NEXT: Address: [[$RODATA:0x(4D0|4D8)]]

// PIE exe and shlib should have dynamic relocations and only the offset values
// DYNAMIC-LABEL: CHERI __cap_relocs [
// DYNAMIC-NEXT: 0x0200{{20|10}}       Base: 0x[[@EXPR tolower(hex($RODATA))]]      (<unknown symbol>+0) Length: 23 Perms: [[$CONSTANT_FLAG]]
// DYNAMIC-NEXT: 0x0200{{60|30}}       Base: 0x[[@EXPR tolower(hex($RODATA + 16))]] (<unknown symbol>+4) Length: 7  Perms: [[$CONSTANT_FLAG]]
// DYNAMIC-NEXT: 0x0200{{a0|50}}       Base: 0x[[@EXPR tolower(hex($RODATA + 6))]]  (<unknown symbol>+0) Length: 17 Perms: [[$CONSTANT_FLAG]]
// DYNAMIC-NEXT: 0x0200{{e0|70}}       Base: 0x[[@EXPR tolower(hex($RODATA + 6))]]  (<unknown symbol>+0) Length: 17 Perms: [[$CONSTANT_FLAG]]
// DYNAMIC-NEXT: 0x020{{120|090}}      Base: 0x[[@EXPR tolower(hex($RODATA + 6))]]  (<unknown symbol>+1) Length: 17 Perms: [[$CONSTANT_FLAG]]
// DYNAMIC-NEXT: ]


// STATIC-EXTERNAL-CAPSIZEFIX: Section {
// STATIC-EXTERNAL-CAPSIZEFIX:      Name: .rodata (
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: Type: SHT_PROGBITS (0x1)
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: Flags [ (0x32)
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT:   SHF_ALLOC (0x2)
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT:   SHF_MERGE (0x10)
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT:   SHF_STRINGS (0x20)
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: ]
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: Address: [[$RODATA:0x120000208]]

// The external capsizefix doesn;t set the length correctly:
// STATIC-EXTERNAL-CAPSIZEFIX-LABEL: CHERI __cap_relocs [
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x1200100{{20|10}}  Base: 0x[[@EXPR tolower(hex($RODATA))]]      (<unknown symbol>+0) Length: 0   Perms: Object
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x1200100{{60|30}}  Base: 0x[[@EXPR tolower(hex($RODATA + 16))]] (<unknown symbol>+4) Length: 23  Perms: Object
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x1200100{{a0|50}}  Base: 0x[[@EXPR tolower(hex($RODATA + 6))]]  (<unknown symbol>+0) Length: 23  Perms: Object
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x1200100{{e0|70}}  Base: 0x[[@EXPR tolower(hex($RODATA + 6))]]  (<unknown symbol>+0) Length: 23  Perms: Object
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x120010{{120|090}} Base: 0x[[@EXPR tolower(hex($RODATA + 6))]]  (<unknown symbol>+1) Length: 23  Perms: Object
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: ]
