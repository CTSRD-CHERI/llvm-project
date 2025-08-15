// RUN: %riscv64y_purecap_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s
// RUN: %riscv32y_purecap_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s
// RUN: %riscv64y_hybrid_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck --check-prefixes=CHECK,CHECK-HYBRID %s
// RUN: %riscv32y_hybrid_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck --check-prefixes=CHECK,CHECK-HYBRID %s

// CHECK: #define __CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__ 65536
// CHECK: #define __CHERI_CAP_PERMISSION_CAPABILITY_GLOBAL__ 16
// CHECK: #define __CHERI_CAP_PERMISSION_CAPABILITY__ 32
// CHECK: #define __CHERI_CAP_PERMISSION_EXECUTE__ 131072
// CHECK: #define __CHERI_CAP_PERMISSION_LOAD_GLOBAL__ 4
// CHECK: #define __CHERI_CAP_PERMISSION_LOAD_MUTABLE__ 2
// CHECK: #define __CHERI_CAP_PERMISSION_READ__ 262144
// CHECK: #define __CHERI_CAP_PERMISSION_STORE_LOCAL__ 8
// CHECK: #define __CHERI_CAP_PERMISSION_WRITE__ 1
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__ 1024
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_GLOBAL__ 1
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__ 2
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_INVOKE__ 256
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__ 16
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_LOAD__ 4
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_SEAL__ 128
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ 32
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__ 64
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_STORE__ 8
// CHECK-NOT: #define __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ 512
// CHECK: #define __riscv_y [[VERSION:[0-9]+]]
// CHECK-HYBRID: #define __riscv_zcherihybrid [[VERSION]]
// CHECK: #define __riscv_zcheripurecap [[VERSION]]
// CHECK-HYBRID: #define __riscv_zyhybrid [[VERSION]]
