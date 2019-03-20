// RUN: %cheri_clang -### -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic \
// RUN:   -c %s 2>&1 | FileCheck --check-prefix=PURECAP_FNOPIC %s
// RUN: %cheri_clang -c -### -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic \
// RUN:   %s 2>&1 | FileCheck --check-prefix=PURECAP_FNOPIC %s
// RUN: not %cheri_clang -c -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic %s -o /dev/null
// RUN: not %cheri_clang -S -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic %s -o /dev/null
// preprocessing is fine
// RUN: %cheri_clang -E -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic %s -o /dev/null
// RUN: %cheri_clang -E -### -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic \
// RUN:   %s 2>&1 | FileCheck --check-prefix=PREPROCESS_FNOPIC %s

// Also don't make it an error for cc1as:
// RUN: echo "" > %t.S
// RUN: %cheri_clang -### -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic \
// RUN:  -xassembler-with-cpp -c %t.S 2>&1 | FileCheck --check-prefix=PURECAP_ASM_FNOPIC %s
// RUN: %cheri_clang -c -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic %t.S

// allow downgrading it to a warning
// RUN: %cheri_clang -### -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic \
// RUN:   -Wno-error=mips-cheri-bugs -c %s 2>&1 | FileCheck --check-prefix=PURECAP_FNOPIC_WARNING %s
// RUN: %cheri_clang -c -### -target cheri-unknown-freebsd11 -cheri=256 -mabi=purecap -fno-pic \
// RUN:   -Wno-error=mips-cheri-bugs %s 2>&1 | FileCheck --check-prefix=PURECAP_FNOPIC_WARNING %s


// PURECAP_FNOPIC: error: Using -fno-pic with the purecap ABI will generate completely broken code.  [-Wmips-cheri-bugs]
// PREPROCESS_FNOPIC-NOT: error:
// PURECAP_ASM_FNOPIC-NOT: error:
// PURECAP_FNOPIC_WARNING: warning: Using -fno-pic with the purecap ABI will generate completely broken code.  [-Wmips-cheri-bugs]
