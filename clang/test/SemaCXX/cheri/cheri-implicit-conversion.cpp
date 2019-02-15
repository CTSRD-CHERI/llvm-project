// check that we get the same (or more) errors when compiling as C++ code
// RUN: %cheri_cc1 -x c -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify
// RUN: %cheri_cc1 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify

// RUN: not %cheri_cc1 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -ast-dump 2>/dev/null | FileCheck %s -check-prefix CXXAST
// RUN: not %cheri_cc1 -x c -o - %s -fsyntax-only -Wall -Wno-unused-variable -ast-dump 2>/dev/null | FileCheck %s -check-prefix CAST


int global_int;
struct test_struct {
  int* ptr;
  int* __capability cap;
};

typedef void (*voidfn_ptr)(void);
typedef void (*__capability voidfn_cap)(void);

void addrof(void) {
    // capability from taking address of global in hybrid mode is allowed
    int* __capability intcap = &global_int;
    void* __capability vcap = &global_int;
    // and fine for pointers
    int* intptr = &global_int;
    void* vptr = &global_int;
    struct test_struct s;
    s.ptr = &global_int;
    s.cap = &global_int;
    // unrelated types give an error
    float* __capability fcap = &global_int; // expected-error {{cannot implicitly or explicitly convert non-capability  type 'int *' to unrelated capability type 'float * __capability'}}

    // but assigning function pointers always works
    voidfn_ptr fnptr = addrof;
    voidfn_ptr fnptr2 = &addrof;
    voidfn_cap fncap = addrof;
    voidfn_cap fncap2 = &addrof;
}



int foo(int* __capability cap_arg_int, void* __capability cap_arg_void, int* ptr_arg_int, void* ptr_arg_void) {
  // pointer -> cap
  int* __capability intcap = ptr_arg_int; // expected-warning {{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast}}
  unsigned int* __capability uintcap = ptr_arg_int;
#ifdef __cplusplus
  // expected-error@-2 {{cannot implicitly or explicitly convert non-capability  type 'int *' to unrelated capability type 'unsigned int * __capability'}}
#else
  // expected-warning@-4 {{implicit conversion from non-capability type 'int *' to capability type 'unsigned int * __capability' converts between integer types with different signs}}
#endif
  void* __capability vcap = ptr_arg_int; // expected-error {{converting non-capability type 'int *' to capability type 'void * __capability' without an explicit cast}}
  // cap -> pointer
  int* intptr = cap_arg_int; // expected-error {{converting capability type 'int * __capability' to non-capability type 'int *' without an explicit cast}}
  void* vptr = cap_arg_int; // expected-error {{converting capability type 'int * __capability' to non-capability type 'void *' without an explicit cast}}
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
  s.cap = ptr_arg_int; // expected-warning  {{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  s.ptr = cap_arg_int; // expected-error  {{converting capability type 'int * __capability' to non-capability type 'int *' without an explicit cast; if this is intended use __cheri_fromcap}}
  s.cap = cap_arg_int; // okay

  return 0;
}

void fn_taking_const_char_ptr(const char* s);
void fn_taking_const_char_cap(const char* __capability s);

void str_to_ptr(void) {
  // ISO C++11 does not allow conversion from string literal to const char* is fine:
  const char* ptr = "foo";
  const char* __capability cap = "foo";

  // check that we can also call functions with string literals:
  fn_taking_const_char_ptr("foo");
  fn_taking_const_char_cap("foo");

  // but conversion from a pointer isn't
  const char* __capability cap2 = ptr;  // expected-warning {{converting non-capability type 'const char *' to capability type 'const char * __capability' without an explicit cast}}

  // conversion to char* should be an error:
  char* nonconst_ptr = "foo";
  char* __capability nonconst_cap = "foo";
#ifdef __cplusplus
  // expected-warning@-3 {{ISO C++11 does not allow conversion from string literal to 'char *'}}
  // expected-warning@-3 {{ISO C++11 does not allow conversion from string literal to 'char * __capability'}}
#endif
  // CXXAST: FunctionDecl {{.+}} str_to_ptr 'void ()'
  // CXXAST: VarDecl {{.+}} cap 'const char * __capability' cinit
  // CXXAST-NEXT: ImplicitCastExpr {{.+}} 'const char * __capability' <PointerToCHERICapability>
  // CXXAST-NEXT: ImplicitCastExpr {{.+}} 'const char *' <ArrayToPointerDecay>
  // CXXAST-NEXT: StringLiteral {{.+}} 'const char [4]' lvalue "foo"

  // CAST: FunctionDecl {{.+}} str_to_ptr 'void (void)'
  // CAST: VarDecl {{.+}} cap 'const char * __capability' cinit
  // CAST-NEXT: ImplicitCastExpr {{.+}} 'const char * __capability' <PointerToCHERICapability>
  // CAST-NEXT: ImplicitCastExpr {{.+}} 'const char *' <BitCast>
  // CAST-NEXT: ImplicitCastExpr {{.+}} 'char *' <ArrayToPointerDecay>
  // CAST-NEXT: StringLiteral {{.+}} 'char [4]' lvalue "foo"
}

// make sure that we warn about const char[] -> char* instead of suggesting __cheri_{to,from}cap
char *flag2str[] = {
        "public ", "private ", "protected ", "static ",
#ifdef __cplusplus
// expected-warning@-2 4 {{ISO C++11 does not allow conversion from string literal to 'char *'}}
#endif
};
