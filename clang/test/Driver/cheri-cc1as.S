// Check that the ABI is forwarded to the assembler
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=n64 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-N64 %s
// CHERI-ABI-N64: "-target-cpu" "cheri" "-target-abi" "n64"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=sandbox 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-SANDBOX %s
// CHERI-ABI-SANDBOX: "-target-cpu" "cheri" "-target-abi" "sandbox"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-NONE %s
// CHERI-ABI-NONE: "-target-cpu" "cheri" "-target-abi" "n64"

// RUN: %clang -target cheri-unknown-freebsd -c %s -o - | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=NONE-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=n64 -c %s -o - \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=N64-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -c %s -o - \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=SANDBOX-HEADERS %s

// NONE-HEADERS:     Arch: mips64
// NONE-HEADERS:     Flags [ (0x30C20006)
// NONE-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// NONE-HEADERS:       EF_MIPS_MACH_CHERI256

// N64-HEADERS:     Arch: mips64
// N64-HEADERS:     Flags [ (0x30C20006)
// N64-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// N64-HEADERS:       EF_MIPS_MACH_CHERI256


// SANDBOX-HEADERS: Arch: mips64
// SANDBOX-HEADERS: Flags [ (0x30C2C006)
// SANDBOX-HEADERS:    EF_MIPS_ABI_CHERIABI
// SANDBOX-HEADERS:    EF_MIPS_MACH_CHERI256



// Check that the ABI is forwarded to the assembler
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=n64 -mllvm -cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-N64 %s
// CHERI128-ABI-N64: "-target-cpu" "cheri128" "-target-abi" "n64"
// CHERI128-ABI-N64: "-mllvm" "-cheri128"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=sandbox -mllvm -cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-SANDBOX %s
// CHERI128-ABI-SANDBOX: "-target-cpu" "cheri128" "-target-abi" "sandbox"
// CHERI128-ABI-SANDBOX: "-mllvm" "-cheri128"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mllvm -cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-NONE %s
// CHERI128-ABI-NONE: "-target-cpu" "cheri128" "-target-abi" "n64"
// CHERI128-ABI-NONE: "-mllvm" "-cheri128"

// RUN: %clang -target cheri-unknown-freebsd -c %s -o - -mllvm -cheri128 | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=CHERI128-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=n64 -c %s -o - -mllvm -cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-N64-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -c %s -o - -mllvm -cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-SANDBOX-HEADERS %s

// -mcpu=cheri128 should have the same effect (and imply -mllvm -cheri128)
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=n64 -mcpu=cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-N64 %s
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=sandbox -mcpu=cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-SANDBOX %s
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mcpu=cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-NONE %s

// RUN: %clang -target cheri-unknown-freebsd -c %s -o - -mcpu=cheri128 | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=CHERI128-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=n64 -c %s -o - -mcpu=cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-N64-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -c %s -o - -mcpu=cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-SANDBOX-HEADERS %s

// CHERI128-HEADERS:     Arch: mips64
// CHERI128-HEADERS:     Flags [ (0x30C10006)
// CHERI128-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// CHERI128-HEADERS:       EF_MIPS_MACH_CHERI128

// CHERI128-N64-HEADERS:     Arch: mips64
// CHERI128-N64-HEADERS:     Flags [ (0x30C10006)
// CHERI128-N64-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// CHERI128-N64-HEADERS:       EF_MIPS_MACH_CHERI128


// CHERI128-SANDBOX-HEADERS: Arch: mips64
// CHERI128-SANDBOX-HEADERS: Flags [ (0x30C1C006)
// CHERI128-SANDBOX-HEADERS:    EF_MIPS_ABI_CHERIABI
// CHERI128-SANDBOX-HEADERS:    EF_MIPS_MACH_CHERI128



// RUN: %clang -target mips64-unknown-freebsd -mabi=sandbox -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=SANDBOX-WITHOUT-CHERI %s
// SANDBOX-WITHOUT-CHERI: error: invalid argument '-mabi=sandbox' not allowed with 'mips64-unknown-freebsd'

__start:
  nop
