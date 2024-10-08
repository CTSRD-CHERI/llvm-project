// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only %s -o - -verify
// RUN: %riscv32_cheri_purecap_cc1 -fsyntax-only %s -o - -verify
//
// This previously causes a miscompilation in linux. It would fail to select
// the correct register for the type.

register unsigned long current_stack_pointer __asm__("csp"); //expected-error{{bad type for named register variable}}
register void * not_cap_stack_pointer __asm__("sp"); //expected-error{{bad type for named register variable}}

__uintcap_t f(void)
{
  return current_stack_pointer;
}

__uintcap_t b(void)
{
  return not_cap_stack_pointer;
}


