// RUN: %cheri_purecap_cc1 -std=c++1z -fdeprecated-macro -fmessage-length 121 -verify %s
// REQUIRES: asserts
int strncmp_c(__attribute__((cheri_capability)) const char *, __attribute__((cheri_capability)) const char *);
// expected-warning@-1 {{use of __capability before the pointer type}}
// expected-warning@-2 {{use of __capability before the pointer type}}
