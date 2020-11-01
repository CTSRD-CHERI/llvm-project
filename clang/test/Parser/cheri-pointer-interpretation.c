// RUN: %cheri_purecap_cc1 -fsyntax-only %s -verify

#pragma pointer_interpretation push
#pragma pointer_interpretation pop
#pragma pointer_interpretation default
#pragma pointer_interpretation integer
#pragma pointer_interpretation capability

#pragma pointer_interpretation // expected-error {{missing argument to '#pragma pointer_interpretation'; expected 'push', 'pop', 'default', 'integer' or 'capability'}}
#pragma pointer_interpretation 1 // expected-error {{unexpected argument '1' to '#pragma pointer_interpretation'; expected 'push', 'pop', 'default', 'integer' or 'capability'}}
#pragma pointer_interpretation push push // expected-warning {{extra tokens at end of '#pragma pointer_interpretation' - ignored}}
