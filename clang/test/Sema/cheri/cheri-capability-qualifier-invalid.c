// RUN: %cheri_cc1 -DTYPEDEF -o - %s -fsyntax-only -verify

// __capability after the name crashes -> make it an error

struct foo;
typedef struct foo* err1 __capability;
// expected-error@-1 {{expected ';' after top level declarator}}
// expected-warning@-2 {{declaration does not declare anything}}
typedef int err2 __capability;
// expected-error@-1 {{expected ';' after top level declarator}}
// expected-warning@-2 {{declaration does not declare anything}}
typedef __capability int err3; // expected-error {{__capability only applies to pointers; type here is 'int'}}
typedef int __capability err4; // expected-error {{__capability only applies to pointers; type here is 'int'}}
typedef __uintcap_t __capability err6;  // expected-error {{__capability only applies to pointers; type here is '__uintcap_t'}}


void* a __capability;
// expected-error@-1 {{expected ';' after top level declarator}}
// expected-warning@-2 {{declaration does not declare anything}}
// the original test case from  https://github.com/CTSRD-CHERI/clang/issues/134:
__END_DECLS a __capability;
// expected-error@-1 {{unknown type name '__END_DECLS'}}
// expected-error@-2 {{expected ';' after top level declarator}}
// expected-warning@-3 {{declaration does not declare anything}}
