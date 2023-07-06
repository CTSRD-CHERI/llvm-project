// RUN: %cheri_cc1 -DDEPRECATED=1 -fsyntax-only -verify %s
// RUN: %cheri_cc1 -DDEPRECATED=1 -fsyntax-only -fdiagnostics-parseable-fixits -ast-dump %s 2>&1 | FileCheck -check-prefix=CHECK-DEPRECATED %s
// RUN: %cheri_cc1 -DAMBIGUOUS=1 -fsyntax-only -verify %s
// RUN: %cheri_cc1 -DNORMAL=1 -fsyntax-only -verify -ast-dump %s | FileCheck -check-prefix=CHECK-NORMAL %s
// RUN: %cheri_cc1 -DNOT_POINTER=1 -fsyntax-only -verify %s
// RUN: %cheri_cc1 -DTYPEDEF=1 -fsyntax-only -verify -ast-dump %s | FileCheck -check-prefix=CHECK-TYPEDEF %s
// RUN: %cheri_cc1 -DLIST=1  -fsyntax-only -verify -ast-dump %s | FileCheck -check-prefix=CHECK-LIST %s

// Test expected compiler warnings/errors for the __capability qualifier 

#ifdef DEPRECATED
__capability int *x; // expected-warning{{use of __capability before}}
// CHECK-DEPRECATED: fix-it:{{.*}}:{[[@LINE-1]]:1-[[@LINE-1]]:14}:""
// CHECK-DEPRECATED: fix-it:{{.*}}:{[[@LINE-2]]:19-[[@LINE-2]]:19}:" __capability "

__capability int (*f)(void); // expected-warning{{use of __capability before}}
// CHECK-DEPRECATED: fix-it:{{.*}}:{[[@LINE-1]]:1-[[@LINE-1]]:14}:""
// CHECK-DEPRECATED: fix-it:{{.*}}:{[[@LINE-2]]:20-[[@LINE-2]]:20}:" __capability "

// CHECK-DEPRECATED: x 'int * __capability'
// CHECK-DEPRECATED: f 'int (* __capability)(void)'
#endif

#ifdef AMBIGUOUS
__capability int ** x; // expected-error{{ambiguous}}
__capability int * __capability * y; // expected-error{{ambiguous}}
#endif 

#ifdef NORMAL
// expected-no-diagnostics
int * __capability x; // CHECK-NORMAL: x 'int * __capability'
int * __capability *y; // CHECK-NORMAL: y 'int * __capability *'
int * __capability * __capability z; // CHECK-NORMAL: z 'int * __capability * __capability'
#endif

#ifdef NOT_POINTER
__capability int var1; // expected-error{{only applies to pointers}}
#endif

#ifdef TYPEDEF
// expected-no-diagnostics
typedef int* intptr;
__capability intptr x;  // CHECK-TYPEDEF: line:[[#@LINE]]:1, col:21> col:21 x 'int * __capability'
__capability intptr *y; // CHECK-TYPEDEF:line:[[#@LINE]]:1, col:22> col:22 y 'int * __capability *'
#endif

#ifdef LIST
__capability int *a, *b; // expected-warning 2{{use of __capability before the pointer type is deprecated}}
// CHECK-LIST: a 'int * __capability'
// CHECK-LIST: b 'int * __capability'
#endif
