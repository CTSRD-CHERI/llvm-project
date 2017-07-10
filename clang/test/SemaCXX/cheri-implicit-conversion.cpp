// check that we get the same (or more) errors when compiling as C++ code
// RUN: %cheri_cc1 -x c -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify
// RUN: %cheri_cc1 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify


int global_int;
struct test_struct {
  int* ptr;
  int* __capability cap;
};

typedef void (*voidfn_ptr)(void);
typedef void (*__capability voidfn_cap)(void);

void addrof(void) {
    // capability from taking address of global in hybrid mode is an error:
    int* __capability intcap = &global_int; // expected-error {{converting pointer type 'int *' to capability type 'int * __capability' without an explicit cast}}
    void* __capability vcap = &global_int; // expected-error  {{converting pointer type 'int *' to capability type 'void * __capability' without an explicit cast}}
    // but fine for pointers
    int* intptr = &global_int; // okay
    void* vptr = &global_int; // okay
    struct test_struct s;
    s.ptr = &global_int; // okay
    s.cap = &global_int; // expected-error  {{converting pointer type 'int *' to capability type 'int * __capability' without an explicit cast; if this is intended use __cheri_cast}}

    // but assigning function pointers always works
    voidfn_ptr fnptr = addrof;
    voidfn_ptr fnptr2 = &addrof;
    voidfn_cap fncap = addrof;
    voidfn_cap fncap2 = &addrof;
#ifdef __cplusplus
    // XXXAR: currently C++ doesn't allow implicit conversions from function pointer to capability (and I'm not sure we should allow it without a cast)
    //expected-error@-4 {{converting pointer type 'void ()' to capability type 'voidfn_cap' (aka 'void (* __capability)()') without an explicit cast}}
    //expected-error@-4 {{converting pointer type 'void (*)()' to capability type 'voidfn_cap' (aka 'void (* __capability)()') without an explicit cast}}
#endif
}



int foo(int* __capability cap_arg_int, void* __capability cap_arg_void, int* ptr_arg_int, void* ptr_arg_void) {
  // pointer -> cap
  int* __capability intcap = ptr_arg_int; // expected-error {{converting pointer type 'int *' to capability type 'int * __capability' without an explicit cast}}
  void* __capability vcap = ptr_arg_int; // expected-error {{converting pointer type 'int *' to capability type 'void * __capability' without an explicit cast}}
  // cap -> pointer
  int* intptr = cap_arg_int; // expected-error {{converting capability type 'int * __capability' to pointer type 'int *' without an explicit cast}}
  void* vptr = cap_arg_int; // expected-error {{converting capability type 'int * __capability' to pointer type 'void *' without an explicit cast}}
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


  struct test_struct s;
  s.ptr = ptr_arg_int; // okay
  s.cap = ptr_arg_int; // expected-error  {{converting pointer type 'int *' to capability type 'int * __capability' without an explicit cast; if this is intended use __cheri_cast}}
  s.ptr = cap_arg_int; // expected-error  {{converting capability type 'int * __capability' to pointer type 'int *' without an explicit cast; if this is intended use __cheri_cast}}
  s.cap = cap_arg_int; // okay

  return 0;
}

// not yet implemented
#if 0
void test_references(int& ptrref, int& __capability capref) {
  // TODO: look at callers of Sema::CompareReferenceRelationship
  int& ptr1 = ptrref; // okay
  int& ptr2 = capref; // expected-error {{foooof}}

  int& __capability cap1 = capref; // okay
  int& __capability cap2 = ptrref; // expected-error {{foooof}}

  int i;

  int& ptrref2 = i;
  int& __capability capref2 = i; //expected-error{{dasdasds}}
}
#endif
