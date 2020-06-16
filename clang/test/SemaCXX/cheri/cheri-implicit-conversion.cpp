// check that we get the same (or more) errors when compiling as C++ code
// RUN: %cheri_cc1 -x c -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify=expected,expected-c,implicit,implicit-c
// RUN: %cheri_cc1 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify=expected,expected-cxx,implicit,implicit-cxx
// RUN: %cheri_cc1 -x c -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify=expected,expected-c,explicit,explicit-c -cheri-int-to-cap=explicit
// RUN: %cheri_cc1 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify=expected,expected-cxx,explicit,explicit-cxx -cheri-int-to-cap=explicit

int global_int;
struct test_struct {
  int *ptr;
  int *__capability cap;
};

typedef void (*voidfn_ptr)(void);
typedef void (*__capability voidfn_cap)(void);

void addrof(void) {
  // capability from taking address of global in hybrid mode is allowed
  int *__capability intcap = &global_int; // explicit-error{{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast}}
  void *__capability vcap = &global_int;
  // explicit-error@-1{{converting non-capability type 'int *' to capability type 'void * __capability' without an explicit cast}}
  // and fine for pointers
  int *intptr = &global_int;
  void *vptr = &global_int;
  struct test_struct s;
  s.ptr = &global_int;
  s.cap = &global_int; // explicit-error{{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast}}
  // unrelated types give an error
  float *__capability fcap = &global_int;
  // implicit-error@-1{{cannot implicitly or explicitly convert non-capability type 'int *' to unrelated capability type 'float * __capability'}}
  // explicit-error@-2{{cannot implicitly or explicitly convert non-capability type 'int *' to unrelated capability type 'float * __capability'}}

  // but assigning function pointers always works
  voidfn_ptr fnptr = addrof;
  voidfn_ptr fnptr2 = &addrof;
  voidfn_cap fncap = addrof;
  // explicit-cxx-error@-1{{converting non-capability type 'void (&)()' to capability type 'voidfn_cap' (aka 'void (* __capability)()') without an explicit cast}}
  // explicit-c-error@-2{{converting non-capability type 'void (*)(void)' to capability type 'void (* __capability)(void)' without an explicit cast}}
  voidfn_cap fncap2 = &addrof; // explicit-error-re{{converting non-capability type 'void (*)({{(void)?}})' to capability type}}
}

int foo(int *__capability cap_arg_int, void *__capability cap_arg_void, int *ptr_arg_int, void *ptr_arg_void) {
  // pointer -> cap
  int *__capability intcap = ptr_arg_int;
  // implicit-warning@-1 {{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast}}
  // explicit-error@-2 {{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast}}
  unsigned int *__capability uintcap = ptr_arg_int;
  // explicit-error@-1{{cannot implicitly or explicitly convert non-capability type 'int *' to unrelated capability type 'unsigned int * __capability'}}
  // implicit-c-warning@-2{{implicit conversion from non-capability type 'int *' to capability type 'unsigned int * __capability' converts between integer types with different signs}}
  // implicit-cxx-error@-3{{cannot implicitly or explicitly convert non-capability type 'int *' to unrelated capability type 'unsigned int * __capability'}}
  void *__capability vcap = ptr_arg_int; // expected-error {{converting non-capability type 'int *' to capability type 'void * __capability' without an explicit cast}}
  // cap -> pointer
  int *intptr = cap_arg_int; // expected-error {{converting capability type 'int * __capability' to non-capability type 'int *' without an explicit cast}}
  void *vptr = cap_arg_int;  // expected-error {{converting capability type 'int * __capability' to non-capability type 'void *' without an explicit cast}}
  // to void*
  void *__capability vcap2 = cap_arg_int; // casting to void* should work without a cast
  void *vptr2 = ptr_arg_int;              // casting to void* should work without a cast

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
  s.cap = ptr_arg_int;
  // implicit-warning@-1{{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  // explicit-error@-2{{converting non-capability type 'int *' to capability type 'int * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  s.ptr = cap_arg_int;
  // expected-error@-1{{converting capability type 'int * __capability' to non-capability type 'int *' without an explicit cast; if this is intended use __cheri_fromcap}}
  s.cap = cap_arg_int; // okay

  return 0;
}

void fn_taking_const_char_ptr(const char *s);
void fn_taking_const_char_cap(const char *__capability s); // explicit-cxx-note{{passing argument to parameter 's' here}}

void str_to_ptr(void) {
  // ISO C++11 does not allow conversion from string literal to const char* is fine:
  const char *ptr = "foo";
  const char *__capability cap = "foo";
  // explicit-cxx-error@-1{{converting non-capability type 'const char (&)[4]' to capability type 'const char * __capability' without an explicit cast}}
  // explicit-c-error@-2{{converting non-capability type 'char *' to capability type 'const char * __capability' without an explicit cast}}

  // check that we can also call functions with string literals:
  fn_taking_const_char_ptr("foo");
  fn_taking_const_char_cap("foo");
  // explicit-cxx-error@-1{{converting non-capability type 'const char (&)[4]' to capability type 'const char * __capability' without an explicit cast}}
  // explicit-c-error@-2{{converting non-capability type 'char *' to capability type 'const char * __capability' without an explicit cast}}

  // but conversion from a pointer isn't
  const char *__capability cap2 = ptr;
  // implicit-warning@-1 {{converting non-capability type 'const char *' to capability type 'const char * __capability' without an explicit cast}}
  // explicit-error@-2 {{converting non-capability type 'const char *' to capability type 'const char * __capability' without an explicit cast}}

  // conversion to char* should be an error:
  char *nonconst_ptr = "foo";
  char *__capability nonconst_cap = "foo"; // explicit-error-re{{converting non-capability type '{{(const char \(&\)\[4\])|(char \*)}}' to capability type 'char * __capability' without an explicit cast}}
#ifdef __cplusplus
  // expected-warning@-3 {{ISO C++11 does not allow conversion from string literal to 'char *'}}
  // implicit-warning@-3 {{ISO C++11 does not allow conversion from string literal to 'char * __capability'}}
#endif
}

// make sure that we warn about const char[] -> char* instead of suggesting __cheri_{to,from}cap
char *flag2str[] = {
    "public ",    // expected-cxx-warning{{ISO C++11 does not allow conversion from string literal to 'char *'}}
    "private ",   // expected-cxx-warning{{ISO C++11 does not allow conversion from string literal to 'char *'}}
    "protected ", // expected-cxx-warning{{ISO C++11 does not allow conversion from string literal to 'char *'}}
    "static ",    // expected-cxx-warning{{ISO C++11 does not allow conversion from string literal to 'char *'}}
};

#ifdef __cplusplus
void test_references(int &ptrref, int &__capability capref) {
  int &ptr1 = ptrref; // okay
  int &ptr2 = capref; // expected-error {{converting capability type 'int & __capability' to non-capability type 'int &' without an explicit cast; if this is intended use __cheri_fromcap}}

  int &__capability cap1 = capref; // okay
  int &__capability cap2 = ptrref; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}

  int i;

  int &ptrref2 = i;
  int &__capability capref2 = i; //expected-error{{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}
#endif
