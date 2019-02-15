// RUN: %clang_cc1 -triple mips64-qemu-elf -E -dM -x c /dev/null | FileCheck %s -check-prefix ELF
// RUN: %clang_cc1 -triple mips64-qemu-cheri-elf -E -dM -x c /dev/null | FileCheck %s -check-prefix ELF
// RUN: %clang_cc1 -triple mips64-qemu-coff -E -dM -x c /dev/null | FileCheck %s -check-prefix NOTELF
// unknown object format -> ELF
// RUN: %clang_cc1 -triple mips64-qemu-cooff -E -dM -x c /dev/null | FileCheck %s -check-prefix ELF

// ELF: __ELF__
// NOTELF-NOT: __ELF__

