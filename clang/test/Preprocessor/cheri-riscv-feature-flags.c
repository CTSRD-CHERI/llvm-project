// RUN: %riscv64_cheri_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK64,CHECK-V9ISA --implicit-check-not=cheri --implicit-check-not=CHERI
// RUN: %riscv32_cheri_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK32,CHECK-V9ISA --implicit-check-not=cheri --implicit-check-not=CHERI

// CHECK32: #define __CHERI_ADDRESS_BITS__ 32
// CHECK64: #define __CHERI_ADDRESS_BITS__ 64
// CHECK32: #define __CHERI_CAPABILITY_WIDTH__ 64
// CHECK64: #define __CHERI_CAPABILITY_WIDTH__ 128
// CHECK: #define __CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__ 1024
// CHECK: #define __CHERI_CAP_PERMISSION_GLOBAL__ 1
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__ 2
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_INVOKE__ 256
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__ 16
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_LOAD__ 4
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_SEAL__ 128
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ 32
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__ 64
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_STORE__ 8
// CHECK: #define __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ 512
// CHECK: #define __CHERI__ 1
// CHECK: #define __CHERI_HYBRID__ 1
// CHECK32: #define __SIZEOF_CHERI_CAPABILITY__ 8
// CHECK64: #define __SIZEOF_CHERI_CAPABILITY__ 16
// __VERSION__ and __clang_version__ could contain CHERI due to the Git URL.
// CHECK: #define __VERSION__ "{{.+}}"
// CHECK: #define __clang_version__ "{{.+}}"
// CHECK: #define __riscv_xcheri 0
// CHECK: #define __riscv_xcheri_mode_dependent_jumps 1
// CHECK-V9ISA: #define __riscv_xcheri_no_relocation 1
// CHECK-V9ISA: #define __riscv_xcheri_tag_clear 1
