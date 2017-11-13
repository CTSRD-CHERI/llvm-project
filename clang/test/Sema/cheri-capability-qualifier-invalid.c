// RUN: %cheri_cc1 -DTYPEDEF -o - %s -fsyntax-only -verify

// __capability after the name crashes -> make it an error

struct foo;
typedef struct foo* err1 __capability; // expected-error {{'__capability' type specifier must precede the declarator}}
typedef int err2 __capability; // expected-error {{'__capability' type specifier must precede the declarator}}
typedef __capability int err3; // expected-error {{__capability only applies to pointers; type here is 'int'}}
typedef int __capability err4; // expected-error {{__capability only applies to pointers; type here is 'int'}}
typedef __uintcap_t __capability err6;  // expected-error {{__capability only applies to pointers; type here is '__uintcap_t'}}


void* a __capability; // expected-error {{'__capability' type specifier must precede the declarator}}
// the original test case from  https://github.com/CTSRD-CHERI/clang/issues/134:
__END_DECLS a __capability; // expected-error {{unknown type name '__END_DECLS'}} expected-error {{'__capability' type specifier must precede the declarator}}
