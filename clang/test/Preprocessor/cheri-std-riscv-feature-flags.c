// RUN: %riscv64_bakewell_purecap_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s --check-prefixes=CHECK64
// RUN: %riscv32_bakewell_purecap_cc1 -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s --check-prefixes=CHECK32
// RUN: %riscv64_bakewell_purecap_cc1 -target-feature +zcherilevels -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s --check-prefixes=ZCLEVEL,CHECK
// RUN: %riscv32_bakewell_purecap_cc1 -target-feature +zcherilevels -E -dM -ffreestanding < /dev/null \
// RUN:   | FileCheck %s --check-prefixes=ZCLEVEL,CHECK 

// CHECK: #define __CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__ 65536
// ZCLEVEL: #define __CHERI_CAP_PERMISSION_CAPABILITY_LEVEL__ 16
// CHECK: #define __CHERI_CAP_PERMISSION_CAPABILITY__ 32
// ZCLEVEL: #define __CHERI_CAP_PERMISSION_ELEVATE_LEVEL__ 4
// CHECK: #define __CHERI_CAP_PERMISSION_EXECUTE__ 131072
// CHECK: #define __CHERI_CAP_PERMISSION_LOAD_MUTABLE__ 2
// CHECK: #define __CHERI_CAP_PERMISSION_READ__ 262144
// ZCLEVEL: #define __CHERI_CAP_PERMISSION_STORE_LEVEL__ 8
// CHECK: #define __CHERI_CAP_PERMISSION_USER_00__ 64
// CHECK: #define __CHERI_CAP_PERMISSION_USER_01__ 128 
// CHECK64: #define __CHERI_CAP_PERMISSION_USER_02__ 256
// CHECK64: #define __CHERI_CAP_PERMISSION_USER_03__ 512 
// CHECK32-NOT: #define __CHERI_CAP_PERMISSION_USER_02__ 256
// CHECK32-NOT: #define __CHERI_CAP_PERMISSION_USER_03__ 512 
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
