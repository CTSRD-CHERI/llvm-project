// RUN: %clang -x c++ -target cheri -mabi=sandbox -E -dM %s |  FileCheck -check-prefixes=CHECK-CHERI-SANDBOX %s
// RUN: %clang_cc1 -x c++ -triple cheri -target-abi sandbox -E -dM %s | FileCheck -check-prefix=CHECK-CHERI-SANDBOX %s
// CHECK-CHERI-SANDBOX-NOT: #define OBJC_ZEROCOST_EXCEPTIONS 1
// CHECK-CHERI-SANDBOX-NOT: #define __EXCEPTIONS 1
// CHECK-CHERI-SANDBOX-NOT: #define __GXX_RTTI 1
// CHECK-CHERI-SANDBOX-NOT: #define __cpp_rtti 1

// RUN: %clang -x c++ -target cheri -mabi=n64 -E -dM %s |  FileCheck -check-prefix=CHECK-CHERI-N64 %s
// RUN: %clang_cc1 -x c++ -triple cheri -target-abi n64 -E -dM %s | FileCheck -check-prefix=CHECK-CHERI-N64 %s
// RUN: %clang -x c++ -target cheri -mabi=n64 -fexceptions -E -dM %s |  FileCheck -check-prefixes=CHECK-CHERI-N64,CHECK-CHERI-N64-FEXCEPTIONS %s
// RUN: %clang_cc1 -x c++ -triple cheri -target-abi n64 -fexceptions -E -dM %s | FileCheck -check-prefixes=CHECK-CHERI-N64,CHECK-CHERI-N64-FEXCEPTIONS %s
// CHECK-CHERI-N64-FEXCEPTIONS: #define __EXCEPTIONS 1
// CHECK-CHERI-N64: #define __GXX_RTTI 1
// CHECK-CHERI-N64: #define __cpp_rtti 1

// RUN: %clang -x c++ -target cheri -mabi=sandbox -fexceptions -E -dM %s 2>&1 | FileCheck -check-prefixes=CHECK-CHERI-SANDBOX-FEXCEPTIONS,IMPLICIT_RTTI %s
// IMPLICIT_RTTI: warning: implicitly enabling rtti for exception handling [-Wrtti-for-exceptions]
// RUN: %clang_cc1 -x c++ -triple cheri -target-abi sandbox -fexceptions -E -dM %s | FileCheck -check-prefix=CHECK-CHERI-SANDBOX-FEXCEPTIONS %s
// CHECK-CHERI-SANDBOX-FEXCEPTIONS: #define __EXCEPTIONS 1
// CHECK-CHERI-SANDBOX-FEXCEPTIONS: #define __GXX_RTTI 1
// CHECK-CHERI-SANDBOX-FEXCEPTIONS: #define __cpp_rtti 1
