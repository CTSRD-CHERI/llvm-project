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
