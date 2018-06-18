// Check that the ABI is forwarded to the assembler
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mabi=n64 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-N64 %s
// CHERI-ABI-N64: "-target-cpu" "{{cheri(128)?}}" {{.*}} "-target-abi" "n64"
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mabi=purecap 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-SANDBOX %s
// CHERI-ABI-SANDBOX: "-target-cpu" "{{cheri(128)?}}" {{.*}} "-target-abi" "purecap"
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI-ABI-NONE %s
// CHERI-ABI-NONE: "-target-cpu" "{{cheri(128)?}}" {{.*}} "-target-abi" "n64"

// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mcpu=cheri256 -c %s -o - | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=NONE-HEADERS %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mcpu=cheri256 -mabi=n64 -c %s -o - \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=N64-HEADERS %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mcpu=cheri256 -mabi=purecap -c %s -o - \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=SANDBOX-HEADERS %s

// NONE-HEADERS:     Arch: cheri
// NONE-HEADERS:     Flags [ (0x30C20006)
// NONE-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// NONE-HEADERS:       EF_MIPS_MACH_CHERI256

// N64-HEADERS:     Arch: cheri
// N64-HEADERS:     Flags [ (0x30C20006)
// N64-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// N64-HEADERS:       EF_MIPS_MACH_CHERI256


// SANDBOX-HEADERS: Arch: cheri
// SANDBOX-HEADERS: Flags [ (0x30C2C006)
// SANDBOX-HEADERS:    EF_MIPS_ABI_CHERIABI
// SANDBOX-HEADERS:    EF_MIPS_MACH_CHERI256



// Check that the ABI is forwarded to the assembler
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mabi=n64  2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-N64 %s
// CHERI128-ABI-N64: "-target-cpu" "cheri128" {{.*}} "-target-abi" "n64"
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mabi=purecap  2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-SANDBOX %s
// CHERI128-ABI-SANDBOX: "-target-cpu" "cheri128" {{.*}} "-target-abi" "purecap"
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s  2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-NONE %s
// CHERI128-ABI-NONE: "-target-cpu" "cheri128" {{.*}} "-target-abi" "n64"

// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -c %s -o -  | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=CHERI128-HEADERS %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mabi=n64 -c %s -o -  \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-N64-HEADERS %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mabi=purecap -c %s -o -  \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-SANDBOX-HEADERS %s

// -mcpu=cheri128 should have the same effect (and imply )
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mabi=n64 -mcpu=cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-N64 %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mabi=purecap -mcpu=cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-SANDBOX %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -### -c %s -mcpu=cheri128 2>&1 \
// RUN:   | FileCheck -check-prefix=CHERI128-ABI-NONE %s

// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -c %s -o - -mcpu=cheri128 | llvm-readobj -h - \
// RUN:   | FileCheck -check-prefix=CHERI128-HEADERS %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mabi=n64 -c %s -o - -mcpu=cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-N64-HEADERS %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd -mabi=purecap -c %s -o - -mcpu=cheri128 \
// RUN:   | llvm-readobj -h - | FileCheck -check-prefix=CHERI128-SANDBOX-HEADERS %s

// CHERI128-HEADERS:     Arch: cheri
// CHERI128-HEADERS:     Flags [ (0x30C10006)
// CHERI128-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// CHERI128-HEADERS:       EF_MIPS_MACH_CHERI128

// CHERI128-N64-HEADERS:     Arch: cheri
// CHERI128-N64-HEADERS:     Flags [ (0x30C10006)
// CHERI128-N64-HEADERS-NOT:   EF_MIPS_ABI_CHERIABI
// CHERI128-N64-HEADERS:       EF_MIPS_MACH_CHERI128


// CHERI128-SANDBOX-HEADERS: Arch: cheri
// CHERI128-SANDBOX-HEADERS: Flags [ (0x30C1C006)
// CHERI128-SANDBOX-HEADERS:    EF_MIPS_ABI_CHERIABI
// CHERI128-SANDBOX-HEADERS:    EF_MIPS_MACH_CHERI128



// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd -mabi=purecap -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=SANDBOX-WITHOUT-CHERI %s
// SANDBOX-WITHOUT-CHERI: "-target-abi" "purecap" "-mllvm" "-cheri-cap-table-abi=legacy"

__start:
  nop
