/// Check that we report pointers as being always lock-free, otherwise <atomic>
/// ends up using locks with -ffreestanding.
// RUN: %riscv32_cheri_cc1 -fgnuc-version=4.2.1 -target-feature +a -E -dM %s \
// RUN:    | FileCheck %s --check-prefixes=CHECK,CHECK-32 --implicit-check-not=_LOCK_FREE
// RUN: %riscv32_cheri_purecap_cc1 -fgnuc-version=4.2.1 -target-feature +a -E -dM %s \
// RUN:    | FileCheck %s --check-prefixes=CHECK,CHECK-32 --implicit-check-not=_LOCK_FREE
// RUN: %riscv64_cheri_cc1 -fgnuc-version=4.2.1 -target-feature +a -E -dM %s \
// RUN:    | FileCheck %s --check-prefixes=CHECK,CHECK-64 --implicit-check-not=_LOCK_FREE
// RUN: %riscv64_cheri_purecap_cc1 -fgnuc-version=4.2.1 -target-feature +a -E -dM %s \
// RUN:    | FileCheck %s --check-prefixes=CHECK,CHECK-64 --implicit-check-not=_LOCK_FREE

// CHECK: #define __CLANG_ATOMIC_BOOL_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_CHAR16_T_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_CHAR32_T_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_CHAR_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_INT_LOCK_FREE 2
// CHECK-64: #define __CLANG_ATOMIC_LLONG_LOCK_FREE 2
// CHECK-32: #define __CLANG_ATOMIC_LLONG_LOCK_FREE 1
// CHECK: #define __CLANG_ATOMIC_LONG_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_POINTER_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_SHORT_LOCK_FREE 2
// CHECK: #define __CLANG_ATOMIC_WCHAR_T_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_BOOL_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_CHAR_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_INT_LOCK_FREE 2
// CHECK-64: #define __GCC_ATOMIC_LLONG_LOCK_FREE 2
// CHECK-32: #define __GCC_ATOMIC_LLONG_LOCK_FREE 1
// CHECK: #define __GCC_ATOMIC_LONG_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_POINTER_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_SHORT_LOCK_FREE 2
// CHECK: #define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 2
