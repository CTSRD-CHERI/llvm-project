// RUN: %cheri_cc1 -fsyntax-only %s -verify

// TODO: we don't diagnose __cheri_cast correctly in template instantiations and would previously generate potentially
// wrong code...

template <typename b> long c(b arg) {
  return (__cheri_addr long)arg;  // expected-error{{__cheri_addr with dependent type 'b' is not yet supported}}
  // todo-expected-error{{invalid source type 'foo' for __cheri_addr: source must be a capability}}
}

struct foo {
  int a;
  int b;
};

int main() {
  int x1 = c<int>(10);
  int x2 = c<__intcap_t>(20);
  int x3 = c<char*>(0);
  int x4 = c<struct foo>({});
}


template <typename T1, typename T2> T1 test2(T2 arg) {
  return (__cheri_offset T1)arg; // expected-error{{__cheri_offset with dependent type 'T2' is not yet supported}}
  // todo-expected-error{{invalid source type 'foo' for __cheri_addr: source must be a capability}}
  // todo-expected-error{{invalid target type 'char*' for __cheri_addr: target must be a capability}}
}

void test2_instantiate() {
  (void)test2<char*, void*>(0); // todo-expected-note{{in instantiation of function template specialization 'test2<char*, void*>' requested here}}
  (void)test2<long, foo>({});
}


// TODO: would be nice to get a sane warning on instantiation
template <typename T1, typename T2> T1 test3(T2 arg) {
  return (__cheri_fromcap T1)arg; // expected-error{{__cheri_from_cap with dependent type 'T2' is not yet supported}}
}

void test3_instantiate() {
  (void)test3<char*, void*>(0); // todo-expected-note{{in instantiation of function template specialization 'test2<char*, void*>' requested here}}
  (void)test3<long, foo>({});
}
