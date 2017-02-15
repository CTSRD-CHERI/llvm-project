// Check that the ABI is forwarded to the assembler
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=n64 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-N64 %s
// FIXME: should "-target-cpu" "cheri" also be passed?
// CHERI-ABI-N64: "-target-abi" "n64"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=sandbox 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-SANDBOX %s
// CHERI-ABI-SANDBOX: "-target-abi" "sandbox"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-NONE %s
// CHERI-ABI-NONE: "-target-abi" "n64"

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
// SANDBOX-HEADERS: Flags [ (0x30C25006)
// SANDBOX-HEADERS:    EF_MIPS_ABI_CHERIABI
// SANDBOX-HEADERS:    EF_MIPS_MACH_CHERI256



// Check that the ABI is forwarded to the assembler
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=n64 -mllvm -cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-N64 %s
// FIXME: should "-target-cpu" "cheri" also be passed?
// CHERI128-ABI-N64: "-target-abi" "n64"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mabi=sandbox -mllvm -cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-SANDBOX %s
// CHERI128-ABI-SANDBOX: "-target-abi" "sandbox"
// RUN: %clang -target cheri-unknown-freebsd -### -c %s -mllvm -cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-NONE %s
// CHERI128-ABI-NONE: "-target-abi" "n64"

// RUN: %clang -target cheri-unknown-freebsd -c %s -o - -mllvm -cheri128 | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=CHERI128-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=n64 -c %s -o - -mllvm -cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-N64-HEADERS %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -c %s -o - -mllvm -cheri128 \
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
// CHERI128-SANDBOX-HEADERS: Flags [ (0x30C15006)
// CHERI128-SANDBOX-HEADERS:    EF_MIPS_ABI_CHERIABI
// CHERI128-SANDBOX-HEADERS:    EF_MIPS_MACH_CHERI128


__start:
  nop
