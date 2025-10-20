// RUN: %cheri_cc1 -fsyntax-only -verify %s

// expected-no-diagnostics

_Static_assert(__is_same(__make_signed(__intcap), __intcap), "");
_Static_assert(__is_same(__make_signed(signed __intcap), __intcap), "");
_Static_assert(__is_same(__make_signed(unsigned __intcap), __intcap), "");

_Static_assert(__is_same(__make_unsigned(__intcap), unsigned __intcap), "");
_Static_assert(__is_same(__make_unsigned(signed __intcap), unsigned __intcap), "");
_Static_assert(__is_same(__make_unsigned(unsigned __intcap), unsigned __intcap), "");
