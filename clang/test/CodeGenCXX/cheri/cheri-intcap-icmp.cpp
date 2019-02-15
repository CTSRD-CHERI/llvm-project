// RUN: %cheri_purecap_cc1 -x c++ -std=c++11 -emit-llvm -o - %s | %cheri_FileCheck %s
// Check that the boolean result of integer comparisions are zero extended and not sign extended, to be consistent with non-CHERI C++ behaviour

void foo() {
  __intcap_t x = 0x2;
  __intcap_t y = 0x3;
  __intcap_t z = x == y;
  // CHECK: zext i1 %cmp to i64
}

