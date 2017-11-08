// RUN: not --crash %cheri_purecap_cc1 -fsyntax-only -ast-dump -std=c++1z -fdeprecated-macro -fmessage-length 121 -o -  %s
// REQUIRES: asserts
int strncmp_c(__attribute__((cheri_capability)) const char *, __attribute__((cheri_capability)) const char *);
