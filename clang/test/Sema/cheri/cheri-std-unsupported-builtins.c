/// In the RISC-V standard compat mode not all builtins are supported since we do not perform the expansion in the backend.
// RUN: %riscv64_cheri_purecap_cc1 -target-feature +xcheri %s -fsyntax-only -verify=xcheri
// RUN: %riscv64_cheri_purecap_cc1 -target-feature +xcheri-std-compat %s -fsyntax-only -verify=std-compat
// xcheri-no-diagnostics

void *cseal(void *a, void *b) {
  _Static_assert(__has_builtin(__builtin_cheri_seal), ""); // TODO: would be nice to report false here
  return __builtin_cheri_seal(a, b); // std-compat-error{{builtin requires at least one of the following extensions to be enabled: xcheri (without std-compat)}}
}
void *cunseal(void *a, void *b) {
  _Static_assert(__has_builtin(__builtin_cheri_unseal), ""); // TODO: would be nice to report false here
  return __builtin_cheri_unseal(a, b); // std-compat-error{{builtin requires at least one of the following extensions to be enabled: xcheri (without std-compat)}}
}
void *ccseal(void *a, void *b) {
  _Static_assert(__has_builtin(__builtin_cheri_conditional_seal), ""); // TODO: would be nice to report false here
  return __builtin_cheri_conditional_seal(a, b); // std-compat-error{{builtin requires at least one of the following extensions to be enabled: xcheri (without std-compat)}}
}
void *ccopytype(void *a, void *b) {
  _Static_assert(__has_builtin(__builtin_cheri_cap_type_copy), ""); // TODO: would be nice to report false here
  return __builtin_cheri_cap_type_copy(a, b); // std-compat-error{{builtin requires at least one of the following extensions to be enabled: xcheri (without std-compat)}}
}
unsigned long cloadtags(void *a) {
  _Static_assert(__has_builtin(__builtin_cheri_cap_load_tags), ""); // TODO: would be nice to report false here
  return __builtin_cheri_cap_load_tags(a); // std-compat-error{{builtin requires at least one of the following extensions to be enabled: xcheri (without std-compat)}}
}
