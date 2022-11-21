// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm  %s | FileCheck %s

// This started crashing after fixing emission of uintcap_t arrays:
enum { a };
void b() { new int[a]; }
// CHECK-LABEL: @_Z1bv()
// CHECK: call noalias noundef nonnull i8 addrspace(200)* @_Znam(i64 noundef zeroext 0) #2
