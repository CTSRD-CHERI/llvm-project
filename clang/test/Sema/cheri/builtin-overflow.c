// RUN: %cheri_purecap_cc1 %s -fsyntax-only -verify

#if __has_feature(__builtin_add_overflow)
#warning defined as expected
// expected-warning@-1 {{defined as expected}}
#endif

void test(void) {
  typedef __UINTPTR_TYPE__ uintptr_t;
  typedef __INTPTR_TYPE__ intptr_t;
  uintptr_t uptr;
  intptr_t sptr;

  // expected-error@+1 {{operand argument to overflow builtin must be an integer ('uintptr_t' (aka 'unsigned __intcap') invalid)}}
  __builtin_add_overflow(uptr, 0x20, &uptr); 

  // expected-error@+1 {{operand argument to overflow builtin must be an integer ('intptr_t' (aka '__intcap') invalid)}}
  __builtin_add_overflow(sptr, 0x20, &sptr);

  // expected-error@+1 {{operand argument to overflow builtin must be an integer ('uintptr_t' (aka 'unsigned __intcap') invalid)}}
  __builtin_sub_overflow(uptr, 0x20, &uptr); 

  // expected-error@+1 {{operand argument to overflow builtin must be an integer ('intptr_t' (aka '__intcap') invalid)}}
  __builtin_sub_overflow(sptr, 0x20, &sptr);

  // expected-error@+1 {{operand argument to overflow builtin must be an integer ('uintptr_t' (aka 'unsigned __intcap') invalid)}}
  __builtin_mul_overflow(uptr, 0x20, &uptr); 

  // expected-error@+1 {{operand argument to overflow builtin must be an integer ('intptr_t' (aka '__intcap') invalid)}}
  __builtin_mul_overflow(sptr, 0x20, &sptr);
}
