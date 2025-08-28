// REQUIRES: riscv-registered-target

// RUN: %riscv64_bakewell_purecap_cc1 -target-feature +zcherihybrid %s -o /dev/null -Wall -Wextra -Wpedantic -Wsystem-headers -verify -emit-obj -Werror=undef
// RUN: %riscv32_bakewell_purecap_cc1 -target-feature +zcherihybrid %s -o /dev/null -Wall -Wextra -Wpedantic -Wsystem-headers -verify -emit-obj -Werror=undef
// RUN: %riscv64_bakewell_purecap_cc1 -target-feature +zcherihybrid %s -o /dev/null -Wall -Wextra -Wpedantic -Wsystem-headers -verify -emit-obj -Werror=undef -DOLD
// RUN: %riscv32_bakewell_purecap_cc1 -target-feature +zcherihybrid %s -o /dev/null -Wall -Wextra -Wpedantic -Wsystem-headers -verify -emit-obj -Werror=undef -DOLD


// expected-no-diagnostics
#ifdef OLD
#include <cheri_init_globals_bw.h>
#else
#include <cheri_init_globals.h>
#endif

#ifdef __cplusplus
extern "C"
#endif
void _start(void);


void _start(void) {
  cheri_init_globals();
  cheri_init_globals_cbuildcap(__builtin_cheri_global_data_get(),
                               __builtin_cheri_program_counter_get(), 
                               __builtin_cheri_global_data_get());
}
