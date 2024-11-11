// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,cheri.SubObjectRepresentability

struct R1 {
  struct {
    struct {
      char c;
      char a[0x9FF]; // no warn
    } f1good;
    struct {
      char c; // expected-note{{}}
      char a[0x1000]; // expected-warning{{Field 'a' of type 'char[4096]' (size 4096) requires 8 byte alignment for precise bounds; field offset is 1}}
    } f2bad;
    struct {
      int c[2];
      char a[0x1000]; // no warn
    } f3good __attribute__((aligned(8)));
  } s2;
} s1;

struct S2 {
  int x[3];
  int *px;
};

struct R2 {
  char x[0x50]; // expected-note{{16/80}}
  struct S2 s2; // expected-note{{32/32 bytes exposed (may expose capability!)}}
  char c; // expected-note{{1}}
  char a[0x8000]; // expected-warning{{Field 'a' of type 'char[32768]' (size 32768) requires 64 byte alignment for precise bounds; field offset is 113 (aligned to 1); Current bounds: 64-32896}}
  char y[32]; // expected-note{{15/32}}
};
