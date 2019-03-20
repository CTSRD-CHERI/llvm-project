// RUN: %cheri_purecap_cc1 -std=c++11 %s -verify 
// expected-no-diagnostics

static const __uintcap_t unusedPtr = 0xd1e7beef;

class A {
  public:
    static const unsigned int unused = unusedPtr;
};
