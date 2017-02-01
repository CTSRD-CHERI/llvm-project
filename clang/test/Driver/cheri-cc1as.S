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
