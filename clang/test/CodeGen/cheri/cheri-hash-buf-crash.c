// Reproducer for a crash in the new folding intrinsics
// For some reason the generated IR does not crash when using llc, so let's keep this test in clang

// RUN: %cheri_purecap_cc1 -triple cheri-unknown-freebsd -emit-obj -O2 -std=gnu99 -ftls-model=local-exec -x c -o /dev/null %s
typedef a;
typedef b;
#define c(d) (b *)(a) d
*f;
e() { f = c(e); }
