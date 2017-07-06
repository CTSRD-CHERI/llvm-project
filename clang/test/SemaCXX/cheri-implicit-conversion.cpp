
// check that we get the same (or more) errors when compiling as C++ code
// RUN: %cheri_cc1 -x c -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify
// RUN: %cheri_cc1 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify


int global_int;

void addrof(void) {
    // capability from taking address of global in hybrid mode is an error:
    int* __capability intcap = &global_int; // expected-error-re {{{{initializing|(cannot initialize a variable of type)}} 'int * __capability' with an {{(expression of incompatible)|(rvalue of)}} type 'int *'}}
    void* __capability vcap = &global_int; // expected-error-re  {{{{initializing|(cannot initialize a variable of type)}} 'void * __capability' with an {{(expression of incompatible)|(rvalue of)}} type 'int *'}}
    // but fine for pointers
    int* intptr = &global_int; // okay
    void* vptr = &global_int; // okay
}

int foo(int* __capability cap_arg_int, void* __capability cap_arg_void, int* ptr_arg_int, void* ptr_arg_void) {
  // pointer -> cap
  int* __capability intcap = ptr_arg_int; // expected-error-re {{{{initializing|(cannot initialize a variable of type)}} 'int * __capability' with an {{(expression of incompatible)|(lvalue of)}} type 'int *'}}
  void* __capability vcap = ptr_arg_int; // expected-error-re {{{{initializing|(cannot initialize a variable of type)}} 'void * __capability' with an {{(expression of incompatible)|(lvalue of)}} type 'int *'}}
  // cap -> pointer
  int* intptr = cap_arg_int; // expected-error-re {{{{initializing|(cannot initialize a variable of type)}} 'int *' with an {{(expression of incompatible)|(lvalue of)}} type 'int * __capability'}}
  void* vptr = cap_arg_int; // expected-error-re {{{{initializing|(cannot initialize a variable of type)}} 'void *' with an {{(expression of incompatible)|(lvalue of)}} type 'int * __capability'}}
  // to void*
  void* __capability vcap2 = cap_arg_int; // casting to void* should work without a cast
  void* vptr2 = ptr_arg_int; // casting to void* should work without a cast

  // cap/pointer -> __uintcap_t needs a cast
  __uintcap_t uintcap1 = cap_arg_int;
  __uintcap_t uintcap2 = cap_arg_void;
  __uintcap_t uintcap3 = ptr_arg_int;
  __uintcap_t uintcap4 = ptr_arg_void;
// TODO: the C error message is better we should be able to produce something like that
#ifdef __cplusplus
  // expected-error@-6 {{cannot initialize a variable of type '__uintcap_t' with an lvalue of type 'int * __capability'}}
  // expected-error@-6 {{cannot initialize a variable of type '__uintcap_t' with an lvalue of type 'void * __capability'}}
  // expected-error@-6 {{cannot initialize a variable of type '__uintcap_t' with an lvalue of type 'int *'}}
  // expected-error@-6 {{cannot initialize a variable of type '__uintcap_t' with an lvalue of type 'void *'}}
#else
  // expected-warning@-11 {{incompatible pointer to integer conversion initializing '__uintcap_t' with an expression of type 'int * __capability'}}
  // expected-warning@-11 {{incompatible pointer to integer conversion initializing '__uintcap_t' with an expression of type 'void * __capability'}}
  // expected-warning@-11 {{incompatible pointer to integer conversion initializing '__uintcap_t' with an expression of type 'int *'}}
  // expected-warning@-11 {{incompatible pointer to integer conversion initializing '__uintcap_t' with an expression of type 'void *'}}
#endif
  return 0;
}
