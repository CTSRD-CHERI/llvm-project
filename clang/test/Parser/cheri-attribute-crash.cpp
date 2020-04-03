// RUN: %cheri_purecap_cc1 -fsyntax-only -std=c++1z -fdeprecated-macro -fmessage-length=120 -o - %s 2>&1 | FileCheck -check-prefix TRUNCATED %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -std=c++1z -fdeprecated-macro -fmessage-length=121 -o - %s 2>&1 | FileCheck -check-prefix FULL %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -std=c++1z -fdeprecated-macro -fmessage-length=122 -o - %s 2>&1 | FileCheck -check-prefix FULL %s
int strncmp_c(__attribute__((cheri_capability)) const char *, __attribute__((cheri_capability)) const char *);

// TRUNCATED: ...const char *, __attribute__((cheri_capability)) const char *);
// FULL: int strncmp_c(__attribute__((cheri_capability)) const char *, __attribute__((cheri_capability)) const char *);
