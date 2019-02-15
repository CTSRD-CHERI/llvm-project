// RUN: %cheri128_cc1 -Wexcess-padding -verify %s

struct IntAndPtr { // this one is fine
  int i;
  void *ptr;
};

struct IntAndCapPadEnd { // expected-warning {{'IntAndCapPadEnd' contains 12 bytes of padding in a 32 byte structure}}
  void *__capability cap;
  int i;
};

struct IntAndCapPadMiddle { // expected-warning {{'IntAndCapPadMiddle' contains 12 bytes of padding in a 32 byte structure}}
  int i;
  void *__capability cap;
};

struct MultiIntAndPtrOK { // fine
  int i;
  void *ptr;
  int i2;
  void *ptr2;
};

struct MultiIntAndPtrBad { // expected-warning {{'MultiIntAndPtrBad' contains 12 bytes of padding in a 48 byte structure}}
  int i;
  void *ptr;
  int i2;
  void *ptr2;
  int i3;
  void *ptr3;
};

// Try nesting a struct with alignas:
struct alignas(32) BigStruct { // expected-warning{{'BigStruct' contains 31 bytes of padding in a 32 byte structure}}
  char c;
};

struct BadBigStruct { // expected-warning {{'BadBigStruct' contains 31 bytes of padding in a 64 byte structure}}
  char c;
  BigStruct big;
};

struct BadBigStruct2 { // expected-warning {{'BadBigStruct2' contains 31 bytes of padding in a 64 byte structure}}
  BigStruct big;
  char c;
};

// The following are 1 byte padding with 1 byte total size but should not warn:
struct Empty1 {
}; // fine
struct Empty2 : public Empty1 {
}; // fine
struct Empty3 : private Empty1 {
}; // fine


// This used to crash:
template <class mapT> class Dependent {
  mapT val;
  void *__capability cap;
};

template <class mapT> class NonDependent {
   int val;
   void *__capability cap;
};

// TODO: these should probably warn on instantiation!
Dependent<char> D1;
NonDependent<char> N1;
