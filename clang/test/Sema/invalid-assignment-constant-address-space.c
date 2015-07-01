// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only

#define OPENCL_CONSTANT 4194050
int __attribute__((address_space(OPENCL_CONSTANT))) c[3] = {0};

void foo() {
  c[0] = 1; //expected-error{{read-only variable is not assignable}}
}
