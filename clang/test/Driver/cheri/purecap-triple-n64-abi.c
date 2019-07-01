// There was a bug where CheriBSD would invoke hybrid clang with a -purecap triple suffix.
// This should result in an error instead of nonsense output (it broke the debug info by emitting C17 as the return register!)

// First check that -cc1 gives errors:
// RUN: not %clang -cc1 -triple cheri-unknown-freebsd-purecap -cheri-size 128 -target-cpu mips4 -target-abi n64 %s -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix N64-ABI-WITH-PURECAP-TRIPLE
// RUN: not %clang -cc1as -triple cheri-unknown-freebsd-purecap -cheri-size 128 -target-cpu mips4 -target-abi n64 %s -o - 2>&1 | FileCheck %s --check-prefix N64-ABI-WITH-PURECAP-TRIPLE
// N64-ABI-WITH-PURECAP-TRIPLE: error: ABI 'n64' is incompatible with target triple 'cheri-unknown-freebsd-purecap'
// RUN: not %clang -cc1 -triple cheri-unknown-freebsd-gnuabi64 -cheri-size 128 -target-cpu mips4 -target-abi purecap %s -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix PURECAP-ABI-WITH-N64-TRIPLE
// RUN: not %clang -cc1as -triple cheri-unknown-freebsd-gnuabi64 -cheri-size 128 -target-cpu mips4 -target-abi purecap %s -o - 2>&1 | FileCheck %s --check-prefix PURECAP-ABI-WITH-N64-TRIPLE
// PURECAP-ABI-WITH-N64-TRIPLE: error: ABI 'purecap' is incompatible with target triple 'cheri-unknown-freebsd-gnuabi64'

// These ones are fine:
// RUN: %clang -cc1 -triple cheri-unknown-freebsd -cheri-size 128 -target-cpu mips4 -target-abi purecap %s -emit-llvm -o /dev/null
// RUN: %clang -cc1 -triple cheri-unknown-freebsd-purecap -cheri-size 128 -target-cpu mips4 %s -emit-llvm -o /dev/null
// RUN: %clang -cc1 -triple cheri-unknown-freebsd-gnuabi64 -cheri-size 128 -target-cpu mips4 %s -emit-llvm -o /dev/null
// RUN: %clang -cc1as -triple cheri-unknown-freebsd -cheri-size 128 -target-cpu mips4 -target-abi purecap %s -o /dev/null
// RUN: %clang -cc1as -triple cheri-unknown-freebsd-purecap -cheri-size 128 -target-cpu mips4 %s -o /dev/null
// RUN: %clang -cc1as -triple cheri-unknown-freebsd-gnuabi64 -cheri-size 128 -target-cpu mips4 %s -o /dev/null

// Check that we infer the correct mabi= from the triple if no ABI flag is set:
// RUN: %clang -target mips64-unknown-freebsd-purecap -cheri=128 -mcpu=mips4 -c %s -o /dev/null -### 2>&1 | FileCheck %s -check-prefix INFER-PURECAP
// RUN: %clang -target mips64-unknown-freebsd-gnuabi64 -cheri=128 -mcpu=mips4 -c %s -o /dev/null -### 2>&1 | FileCheck %s -check-prefix INFER-N64
// RUN: %clang -target mips64-unknown-freebsd -cheri=128 -mcpu=mips4 -c %s -o /dev/null -### 2>&1 | FileCheck %s -check-prefix INFER-N64
// RUN: %clang -target mips64-unknown-freebsd-purecap -cheri=128 -mcpu=mips4 -E %s -o /dev/null -### 2>&1 | FileCheck %s -check-prefix INFER-PURECAP
// RUN: %clang -target mips64-unknown-freebsd-gnuabi64 -cheri=128 -mcpu=mips4 -E %s -o /dev/null -### 2>&1 | FileCheck %s -check-prefix INFER-N64
// RUN: %clang -target mips64-unknown-freebsd -cheri=128 -mcpu=mips4 -E %s -o /dev/null -### 2>&1 | FileCheck %s -check-prefix INFER-N64
// INFER-PURECAP: "-triple" "mips64-unknown-freebsd-purecap"
// INFER-PURECAP-SAME: "-target-abi" "purecap"
// INFER-N64: "-triple" "mips64-unknown-freebsd{{(-gnuabi64)?}}"
// INFER-N64-SAME: "-target-abi" "n64"

// Now check that the driver converts the triple if -mabi= is passed (but with a warning)

// RUN: %clang --target=cheri-unknown-freebsd-purecap -cheri=128 -mcpu=mips4 -EB -mabi=64 %s -S -emit-llvm -o /dev/null -v 2>&1 | FileCheck %s --check-prefix OVERRIDING-PURECAP

// OVERRIDING-PURECAP: warning: overriding triple 'cheri-unknown-freebsd-purecap' environment 'purecap' since an explicit -mabi=64 flag was passed [-Woption-ignored]
// OVERRIDING-PURECAP: Target: cheri-unknown-freebsd-gnuabi64
// OVERRIDING-PURECAP: -cc1 -triple cheri-unknown-freebsd-gnuabi64
// OVERRIDING-PURECAP-SAME: -target-abi n64

// RUN: echo "" > %t.s
// RUN: %clang --target=cheri-unknown-freebsd-purecap -cheri=128 -mcpu=mips4 -EB -mabi=64 %t.s -c -o /dev/null -v 2>&1 | FileCheck %s --check-prefix OVERRIDING-PURECAP-ASM
// OVERRIDING-PURECAP-ASM: Target: cheri-unknown-freebsd-gnuabi64
// OVERRIDING-PURECAP-ASM: -cc1as -triple cheri-unknown-freebsd-gnuabi64
// OVERRIDING-PURECAP-ASM-SAME: -target-abi n64


// Check that n64 -> purecap also works:
// RUN: %clang --target=cheri-unknown-freebsd-gnuabi64 -cheri=128 -mcpu=mips4 -EB -mabi=purecap %s -S -emit-llvm -o /dev/null -v 2>&1 | FileCheck %s --check-prefix OVERRIDING-N64
// OVERRIDING-N64: warning: overriding triple 'cheri-unknown-freebsd-gnuabi64' environment 'gnuabi64' since an explicit -mabi=purecap flag was passed [-Woption-ignored]
// OVERRIDING-N64: Target: mips64c128-unknown-freebsd-purecap
// OVERRIDING-N64: -cc1 -triple mips64c128-unknown-freebsd-purecap
// OVERRIDING-N64-SAME: -target-abi purecap

// RUN: echo "" > %t.s
// RUN: %clang --target=cheri-unknown-freebsd-gnuabi64 -cheri=256 -mcpu=mips4 -EB -mabi=purecap %t.s -c -o /dev/null -v 2>&1 | FileCheck %s --check-prefix OVERRIDING-N64-ASM
// OVERRIDING-N64-ASM: Target: mips64c256-unknown-freebsd-purecap
// OVERRIDING-N64-ASM: -cc1as -triple mips64c256-unknown-freebsd-purecap
// OVERRIDING-N64-ASM-SAME: -target-abi purecap

// MIPS32 triple should be rejected with -purecap
// RUN: not %clang --target=mips-unknown-freebsd -cheri=128 -mcpu=mips4 -EB -mabi=purecap %s -S -emit-llvm -o /dev/null -v 2>&1 | FileCheck %s -check-prefix MIPS32-ERROR
// MIPS32-ERROR: error: unsupported option '-mabi=purecap' for target 'mips-unknown-freebsd-purecap'
