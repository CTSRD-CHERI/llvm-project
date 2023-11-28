// RUN: %riscv32_bakewell_hybrid_cc1 -fsyntax-only %s -verify
// RUN: %riscv64_bakewell_hybrid_cc1 -fsyntax-only %s -verify


void standard(void * __capability cap){
    volatile __SIZE_TYPE__ x;

    x = __builtin_cheri_cap_load_tags(cap); // expected-error{{builtin unsupported when using bakewell : Unsupported Builtin}}
    void * __capability volatile z;
    z = __builtin_cheri_tag_clear(cap); // expected-error{{builtin unsupported when using bakewell : Unsupported Builtin}}
    z = __builtin_cheri_unseal(cap, cap); // expected-error{{builtin unsupported when using bakewell : Unsupported Builtin}}
    z = __builtin_cheri_cap_type_copy(cap, cap); // expected-error{{builtin unsupported when using bakewell : Unsupported Builtin}}
    z = __builtin_cheri_seal(cap, cap); // expected-error{{builtin unsupported when using bakewell : Unsupported Builtin}}
    z = __builtin_cheri_conditional_seal(z, z); // expected-error{{builtin unsupported when using bakewell : Unsupported Builtin}}
}
