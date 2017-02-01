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


__start:
  nop
