// RUN: %cheri_cc1 -std=c++11 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify
// RUN: %cheri_purecap_cc1 -std=c++11 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -std=c++11
// previous changes broke constructors that take one pointer argument and treated them as a functional C style case (-Wcapability-to-integer-cast)
// expected-no-diagnostics

template<class T>
class future {
public:
  typedef T(*Func)();
  future(Func f);
};

void test_func() {
  return;
}

future<void> test1() {
  return future<void>(test_func);
}
future<void> test2() {
  return future<void>(&test_func);  // this used to warn about casting a capability to future<void>
}
