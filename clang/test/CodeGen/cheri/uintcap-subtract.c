// RUN: %cheri_purecap_clang -cheri-uintcap=offset %s -S -emit-llvm -o /dev/null -O0
// RUN: %cheri_purecap_cc1 -verify=offset,expected -cheri-uintcap=offset %s -emit-llvm -o /dev/null -O0
// RUN: %cheri_purecap_cc1 -verify=address,expected -cheri-uintcap=addr %s -emit-llvm -o /dev/null -O0

// Subtracting two __intcap_t values yields surprising results in offset mode.
// In address mode the result is correct (but might be a tagged value since we
// don't know if it was meant to be pointer arithmetic or pointer difference).
// This can causes traps when running QEMU with the trap-on-unrepresetable mode

typedef __PTRDIFF_TYPE__ ptrdiff_t;

void want_ptrdiff(ptrdiff_t p);
void want_intcap(__intcap_t u);

void check_sub(__intcap_t cap1, __intcap_t cap2, ptrdiff_t ptrdiff) {
  // ptrdiff is promoted to __intcap_t here so the warning triggers:
  __intcap_t int_and_cap = ptrdiff - cap1; // fine in address-mode
  // offset-warning@-1{{subtracting '__intcap_t' from 'ptrdiff_t' (aka 'long') may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // offset-note@-2{{in order to perform pointer arithmetic}}
  __intcap_t cap_and_int = cap1 - ptrdiff;
  __intcap_t cap_and_cap = cap1 - cap2;
  // offset-warning@-1{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // address-warning@-2{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the left-hand side is derived from a pointer since it may result in an unintended tagged value}}
  // expected-note@-3{{in order to perform pointer arithmetic}}

  // Now cast the result to ptrdiff_t
  ptrdiff_t ptrdiff_int_and_cap = ptrdiff - cap1; // fine in address-mode
  // offset-warning@-1{{subtracting '__intcap_t' from 'ptrdiff_t' (aka 'long') may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // offset-note@-2{{in order to perform pointer arithmetic}}
  ptrdiff_t ptrdiff_cap_and_int = cap1 - ptrdiff; // fine
  ptrdiff_t ptrdiff_cap_and_cap = cap1 - cap2;
  // offset-warning@-1{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // address-warning@-2{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the left-hand side is derived from a pointer since it may result in an unintended tagged value}}
  // expected-note@-3{{in order to perform pointer arithmetic}}

  // And call a function
  want_ptrdiff(ptrdiff - cap1); // fine in address-mode
  // offset-warning@-1{{subtracting '__intcap_t' from 'ptrdiff_t' (aka 'long') may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // offset-note@-2{{in order to perform pointer arithmetic}}
  want_ptrdiff(cap1 - ptrdiff);
  want_ptrdiff(cap1 - cap2);
  // offset-warning@-1{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // address-warning@-2{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the left-hand side is derived from a pointer since it may result in an unintended tagged value}}
  // expected-note@-3{{in order to perform pointer arithmetic}}

  want_intcap(ptrdiff - cap1); // fine in address-mode
  // offset-warning@-1{{subtracting '__intcap_t' from 'ptrdiff_t' (aka 'long') may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // offset-note@-2{{in order to perform pointer arithmetic}}
  want_intcap(cap1 - ptrdiff);
  want_intcap(cap1 - cap2);
  // offset-warning@-1{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // address-warning@-2{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the left-hand side is derived from a pointer since it may result in an unintended tagged value}}
  // expected-note@-3{{in order to perform pointer arithmetic}}

  ptrdiff -= ptrdiff; // fine
  ptrdiff -= cap1; // inefficient in addr mode, possibly wrong in offset mode
  // offset-warning@-1{{subtracting '__intcap_t' from 'ptrdiff_t' (aka 'long') may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // offset-note@-2{{in order to perform pointer arithmetic}}
  cap1 -= ptrdiff; // fine
  cap1 -= cap2; // inefficient, possibly wrong in offset mode and might yield tagged value in addr mode
  // offset-warning@-1{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the right-hand side is derived from a pointer since it will only consider the capability offset}}
  // address-warning@-2{{subtracting '__intcap_t' from '__intcap_t' may yield incorrect results if the left-hand side is derived from a pointer since it may result in an unintended tagged value}}
  // expected-note@-3{{in order to perform pointer arithmetic}}
}

void test_pointers(char* ptr, __intcap_t intcap, ptrdiff_t ptrdiff) {

  char* p1 = ptr - intcap;
  // offset-warning@-1{{subtracting '__intcap_t' from 'char *' may yield an incorrect result if the '__intcap_t' value is derived from a pointer since it will only consider the capability offset}}}
  char* p2 = ptr - ptrdiff;

  // ptr -= ptr;
  ptr -= intcap;
  // offset-warning@-1{{subtracting '__intcap_t' from 'char *' may yield an incorrect result if the '__intcap_t' value is derived from a pointer since it will only consider the capability offset}}}
  ptr -= ptrdiff;
}


__intcap_t test_unary() {
  return -1;
}
