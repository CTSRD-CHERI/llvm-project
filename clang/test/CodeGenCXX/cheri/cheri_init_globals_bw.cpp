// REQUIRES: riscv-registered-target

// RUN: %riscv64_bakewell_purecap_cc1 -target-feature +zcherihybrid %s -o /dev/null -Wall -Wextra -Wpedantic -Wsystem-headers -verify -emit-obj -Werror=undef
// RUN: %riscv32_bakewell_purecap_cc1 -target-feature +zcherihybrid %s -o /dev/null -Wall -Wextra -Wpedantic -Wsystem-headers -verify -emit-obj -Werror=undef


// expected-no-diagnostics
#include <cheri_init_globals_bw.h>

#ifdef __cplusplus
extern "C"
#endif
void _start(void);


void _start(void) {
  cheri_init_globals();
}
