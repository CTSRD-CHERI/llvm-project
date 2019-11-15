// RUN: %cheri_cc1 -fsyntax-only -verify %s

// Subtraction between capability and non-capability pointers should be an error
// See https://github.com/CTSRD-CHERI/llvm-project/issues/351

long sub_check(int * i, int* __capability icap, float* f, float* __capability fcap) {
  long result;
  // postitive checks
  result = i - i; // fine
  result = i - f; // expected-error{{'int *' and 'float *' are not pointers to compatible types}}
  result = f - i; // expected-error{{'float *' and 'int *' are not pointers to compatible types}}
  result = f - f; // fine
  result = icap - icap; // fine
  result = icap - fcap; // expected-error{{'int * __capability' and 'float * __capability' are not pointers to compatible types}}
  result = fcap - icap; // expected-error{{'float * __capability' and 'int * __capability' are not pointers to compatible types}}
  result = fcap - fcap; // fine

  result = i - icap; // expected-error{{subtraction between integer pointer and capability ('int *' and 'int * __capability')}}
  result = f - icap; // expected-error{{'float *' and 'int * __capability' are not pointers to compatible types}}
  result = icap - i; // expected-error{{subtraction between integer pointer and capability ('int * __capability' and 'int *')}}
  result = icap - f; // expected-error{{'int * __capability' and 'float *' are not pointers to compatible types}}

  result = i - fcap; // expected-error{{'int *' and 'float * __capability' are not pointers to compatible types}}
  result = f - fcap; // expected-error{{subtraction between integer pointer and capability ('float *' and 'float * __capability')}}
  result = fcap - i; // expected-error{{'float * __capability' and 'int *' are not pointers to compatible types}}
  result = fcap - f; // expected-error{{subtraction between integer pointer and capability ('float * __capability' and 'float *')}}

  return result;
}
