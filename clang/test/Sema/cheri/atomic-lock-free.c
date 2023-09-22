/// Check that we report true for __atomic_always_lock_free(sizeof(uintptr_t)).
/// For example libc++'s std::atomic includes a is_always_lock_free member defined as
/// static _LIBCPP_CONSTEXPR bool is_always_lock_free = __atomic_always_lock_free(sizeof(__a_), 0);
/// This was incorrectly being set to false for purecap std::atomic<uintptr_t>.
/// Ideally the builtin would take a type rather than a size but unfortunately it's too late to change that.
/// See also CodeGen/cheri/atomic-lock-free.c to show that we generate the appropriate code.
// RUN: %riscv64_cheri_purecap_cc1 -target-feature +a %s -fsyntax-only -verify=purecap
// RUN: %riscv64_cheri_cc1 -target-feature +a %s -fsyntax-only -verify=hybrid
// RUN: %riscv32_cheri_purecap_cc1 -target-feature +a %s -fsyntax-only -verify=purecap
// RUN: %riscv32_cheri_cc1 -target-feature +a %s -fsyntax-only -verify=hybrid
// purecap-no-diagnostics

_Static_assert(__atomic_always_lock_free(sizeof(char), 0), "");
_Static_assert(__atomic_always_lock_free(sizeof(short), 0), "");
_Static_assert(__atomic_always_lock_free(sizeof(int), 0), "");
_Static_assert(__atomic_always_lock_free(sizeof(__INTPTR_TYPE__), 0), "");
_Static_assert(__atomic_always_lock_free(sizeof(__UINTPTR_TYPE__), 0), "");
_Static_assert(__atomic_always_lock_free(sizeof(void *), 0), "");
/// TODO: it would be nice if hybrid mode also allowed lock-free sizeof(void * __capability)
/// but this is not currently true since atomic RMW/CMPXCHG with capability
/// pointers are not supported.
_Static_assert(__atomic_always_lock_free(sizeof(void * __capability), 0), "");  // hybrid-error{{static assertion failed due to requirement '__atomic_always_lock_free(sizeof(void * __capability), 0)'}}
