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

void g(void){
  register __uintcap_t a0 asm("a4") = 0; //expected-error{{bad type for named register variable}}
  asm volatile ("nop": "=C"(a0));
}

void h(void) {
  register unsigned long a0 asm("ca4") = 0; //expected-error{{bad type for named register variable}}
  asm volatile ("nop": "=r"(a0));
}

