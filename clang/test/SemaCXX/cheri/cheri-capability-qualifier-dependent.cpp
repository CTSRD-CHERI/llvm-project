// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify

template<typename T>
T __capability test_template(T p) {
  // expected-note@-1 2 {{candidate template ignored: substitution failure [with T = int]: __capability only applies to pointers; type here is 'int'}}
  T __capability c = (__cheri_tocap T __capability)p;
  return c;
}

template void * __capability test_template<void *>(void *);
template int __capability test_template<int>(int);
// expected-error@-1 {{__capability only applies to pointers; type here is 'int'}}
// expected-error@-2 {{explicit instantiation of 'test_template' does not refer to a function template, variable template, member function, member class, or static data member}}
template int test_template<int>(int);
// expected-error@-1 {{explicit instantiation of 'test_template' does not refer to a function template, variable template, member function, member class, or static data member}}

void * __capability test_func_redundant(void * __capability x) {
    return test_template(x);
}

template<typename T, typename E>
E test_template2(T p) {
  T __capability c = (__cheri_tocap T __capability)p;
  return *c;
}

template char test_template2<char *, char>(char *);
template int test_template2<int *, int>(int *);

template<typename T>
T * __capability test_template3(T *p) {
  T * __capability c = (__cheri_tocap T * __capability)p;
  return c;
}

template void * __capability test_template3<void>(void *);
template int * __capability test_template3<int>(int *);

template<typename T>
T * __capability test_template4(T *p) {
  T __capability c = (__cheri_tocap T __capability)p;
  // expected-error@-1 2 {{__capability only applies to pointers; type here is 'void'}}
  // expected-error@-2 2 {{__capability only applies to pointers; type here is 'int'}}
  // expected-error@-3 {{variable has incomplete type 'void'}}
  // expected-error@-4 {{cast to incomplete type 'void'}}
  // expected-error@-5 {{invalid target type 'int' for __cheri_tocap: target must be a capability}}
  return c;
  // expected-error@-1 {{cannot initialize return object of type 'void ** __capability' with an lvalue of type 'void * __capability'}}
}

template void * __capability test_template4<void>(void *);
// expected-note@-1 {{in instantiation of function template specialization 'test_template4<void>' requested here}}
template int * __capability test_template4<int>(int *);
// expected-note@-1 {{in instantiation of function template specialization 'test_template4<int>' requested here}}
template void ** __capability test_template4<void *>(void **);
// expected-note@-1 {{in instantiation of function template specialization 'test_template4<void *>' requested here}}
