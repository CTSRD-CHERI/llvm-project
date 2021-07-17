// RUN: %cheri_purecap_cc1 -fsyntax-only -std=c++1z -fdeprecated-macro -fmessage-length=78 -o - %s 2>&1 | FileCheck -check-prefix TRUNCATED %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -std=c++1z -fdeprecated-macro -fmessage-length=79 -o - %s 2>&1 | FileCheck -check-prefix FULL %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -std=c++1z -fdeprecated-macro -fmessage-length=80 -o - %s 2>&1 | FileCheck -check-prefix FULL %s
int strncmp_c(__capability const char *, __capability const char *);

// TRUNCATED: ...const char *, __capability const char *);
// FULL: int strncmp_c(__capability const char *, __capability const char *);
