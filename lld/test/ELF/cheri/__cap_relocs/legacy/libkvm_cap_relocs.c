// REQUIRES: clang

// RUN: llvm-readobj -r %S/../Inputs/kvm_pcpu.pico | FileCheck -check-prefix READOBJ %s
// RUN: ld.lld -process-cap-relocs %S/../Inputs/kvm_pcpu.pico -shared --fatal-warnings -o %t.so 2>&1 | FileCheck %s -check-prefix WARN
// WARN-NOT: Could not find a real symbol for __cap_reloc against .data.rel.ro+0x0
// WARN: Could not find a real symbol for __cap_reloc against .data.rel.ro+0x20
// WARN: Could not find a real symbol for __cap_reloc against .data.rel.ro+0x40
// WARN: Could not find a real symbol for __cap_reloc against .data.rel.ro+0x60
// RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix DYNAMIC-RELOCS
// RUN: llvm-objdump -C %t.so | FileCheck %s

// FIXME: it would be good if we could set bounds here instead of having it as -1


// READOBJ:      Relocations [
// READOBJ:        Section (14) .rela__cap_relocs {
// READOBJ-NEXT:     0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x0
// READOBJ-NEXT:     0x8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xE6
// READOBJ-NEXT:     0x28 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x20
// READOBJ-NEXT:     0x30 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xF5
// READOBJ-NEXT:     0x50 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x40
// READOBJ-NEXT:     0x58 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x101
// READOBJ-NEXT:     0x78 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x0
// READOBJ-NEXT:     0x80 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x10B
// READOBJ-NEXT:     0xA0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x20
// READOBJ-NEXT:     0xA8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x11D
// READOBJ-NEXT:     0xC8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x40
// READOBJ-NEXT:     0xD0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x12E
// READOBJ-NEXT:     0xF0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x60
// READOBJ-NEXT:     0xF8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0xF5
// READOBJ-NEXT:   }
// READOBJ-NEXT: ]

// dynamic should have 14 relocations against the load address
// DYNAMIC-RELOCS-LABEL: Relocations [
// DYNAMIC-RELOCS-NEXT:   Section (8) .rel.dyn {
// DYNAMIC-RELOCS-NEXT:     0x300A0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x300A8 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x300C8 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x300D0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x300F0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x300F8 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30118 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30120 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30140 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30148 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30168 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30170 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30190 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30198 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:   }
// DYNAMIC-RELOCS-NEXT: ]


// CHECK: CAPABILITY RELOCATION RECORDS:
// CHECK-NEXT: 0x0000000000020000      Base: <Unnamed symbol> (0x00000000000004da)     Offset: 0x0000000000000000  Length: 0x00000000000000bd       Permissions: 0x00000000
// CHECK-NEXT: 0x0000000000020020      Base: <Unnamed symbol> (0x00000000000004a4)     Offset: 0x0000000000000000  Length: 0x00000000000000f3       Permissions: 0x00000000
// CHECK-NEXT: 0x0000000000020040      Base: <Unnamed symbol> (0x000000000000058d)     Offset: 0x0000000000000000  Length: 0x000000000000000a       Permissions: 0x00000000
// CHECK-NEXT: 0x0000000000030000      Base: <Unnamed symbol> (0x000000000000055b)     Offset: 0x0000000000000000  Length: 0x000000000000003c       Permissions: 0x00000000
// CHECK-NEXT: 0x0000000000030020      Base: <Unnamed symbol> (0x00000000000004c9)     Offset: 0x0000000000000000  Length: 0x00000000000000ce       Permissions: 0x00000000
// CHECK-NEXT: 0x0000000000030040      Base: <Unnamed symbol> (0x00000000000004b0)     Offset: 0x0000000000000000  Length: 0x00000000000000e7       Permissions: 0x00000000
// CHECK-NEXT: 0x0000000000030060      Base: <Unnamed symbol> (0x00000000000004a4)     Offset: 0x0000000000000000  Length: 0x00000000000000f3       Permissions: 0x00000000
