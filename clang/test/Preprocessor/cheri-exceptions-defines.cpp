// RUN: %clang -fgnuc-version=4.2.1 -x c++ -target cheri -mabi=purecap -fno-rtti -fno-exceptions -E -dM %s |  FileCheck -check-prefixes=CHECK-CHERI-SANDBOX-NOEXCEPTIONS %s
// RUN: %clang_cc1 -fgnuc-version=4.2.1 -x c++ -triple cheri -target-abi purecap -fno-rtti -E -dM %s | FileCheck -check-prefix=CHECK-CHERI-SANDBOX-NOEXCEPTIONS %s
// CHECK-CHERI-SANDBOX-NOEXCEPTIONS-NOT: #define OBJC_ZEROCOST_EXCEPTIONS 1
// CHECK-CHERI-SANDBOX-NOEXCEPTIONS-NOT: #define __EXCEPTIONS 1
// CHECK-CHERI-SANDBOX-NOEXCEPTIONS-NOT: #define __GXX_RTTI 1
// CHECK-CHERI-SANDBOX-NOEXCEPTIONS-NOT: #define __cpp_rtti 1

// RUN: %clang -fgnuc-version=4.2.1 -x c++ -target mips64c128-none-none -mabi=n64 -E -dM %s |  FileCheck -check-prefix=CHECK-CHERI-N64 %s
// RUN: %cheri_cc1 -fgnuc-version=4.2.1 -x c++ -target-abi n64 -E -dM %s | FileCheck -check-prefix=CHECK-CHERI-N64 %s
// RUN: %clang -fgnuc-version=4.2.1 -x c++ -target mips64c128-none-none -mabi=n64 -fexceptions -E -dM %s |  FileCheck -check-prefixes=CHECK-CHERI-N64,CHECK-CHERI-N64-FEXCEPTIONS %s
// RUN: %cheri_cc1 -fgnuc-version=4.2.1 -x c++ -target-abi n64 -fexceptions -E -dM %s | FileCheck -check-prefixes=CHECK-CHERI-N64,CHECK-CHERI-N64-FEXCEPTIONS %s
// CHECK-CHERI-N64-FEXCEPTIONS: #define __EXCEPTIONS 1
// CHECK-CHERI-N64: #define __GXX_RTTI 1
// CHECK-CHERI-N64: #define __cpp_rtti 1

// RUN: %clang -fgnuc-version=4.2.1 -x c++ -target cheri -mabi=purecap -fexceptions -E -dM %s 2>&1 | FileCheck -check-prefixes=CHECK-CHERI-SANDBOX-FEXCEPTIONS %s
// IMPLICIT_RTTI: warning: implicitly enabling rtti for exception handling [-Wrtti-for-exceptions]
// RUN: %clang_cc1 -fgnuc-version=4.2.1 -x c++ -triple cheri -target-abi purecap -fexceptions -E -dM %s | FileCheck -check-prefix=CHECK-CHERI-SANDBOX-FEXCEPTIONS %s
// CHECK-CHERI-SANDBOX-FEXCEPTIONS: #define __EXCEPTIONS 1
// CHECK-CHERI-SANDBOX-FEXCEPTIONS: #define __GXX_RTTI 1
// CHECK-CHERI-SANDBOX-FEXCEPTIONS: #define __cpp_rtti 1
