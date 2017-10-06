// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s
// This used to crash the compiler instead of emitting an invalid syntax error

__END_DECLS a __capability
// expected-error@-1 {{unknown type name '__END_DECLS'}}
// expected-error@-2 {{'__capability' type specifier must precede the declarator}}
// expected-error@-3 {{expected ';' after top level declarator}}
