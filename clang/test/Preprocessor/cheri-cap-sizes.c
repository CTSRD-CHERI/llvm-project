// CHERI triple without any flags should set 128 defines
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11  \
// RUN:  -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=PURECAP,PURECAP-128,CHERI128 %s
// or with -cheri
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=PURECAP,PURECAP-128,CHERI128 %s
// or with -cheri=128
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri=128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=PURECAP,PURECAP-128,CHERI128 %s
// or with both
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri=128 -mcpu=cheri128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=PURECAP,PURECAP-128,CHERI128 %s

// Try with -march=mips64
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -mcpu=mips4 -cheri=128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=PURECAP,PURECAP-128,CHERI128 %s
// and -mcpu=mips4
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -march=mips64 -cheri=128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=PURECAP,PURECAP-128,CHERI128 %s

// now try a non-cheri triple but -mcpu=cheri128
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -mcpu=cheri128 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS,CHERI128 -DARCHNAME=CHERI128 %s

// now try a non-cheri triple with -cheri
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -cheri \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS,CHERI128 -DARCHNAME=MIPS3 %s
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -cheri=128 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS,CHERI128 -DARCHNAME=MIPS3 %s
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -cheri=128 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS,128 -DARCHNAME=MIPS3 %s


// MIPS: #define _MIPS_ARCH_CHERI128 1
// CHERI128: #define _MIPS_CAP_ALIGN_MASK 0xfffffffffffffff0
// CHERI128: #define _MIPS_SZCAP 128
// CHECK: #define __CHERI_ADDRESS_BITS__ 64
// CHERI128: #define __CHERI_CAPABILITY_WIDTH__ 128
// PURECAP: #define __CHERI_PURE_CAPABILITY__ 2
// MIPS-NOT: __CHERI_PURE_CAPABILITY__
// CHECK: #define __CHERI__ 1
// Note: 64-bit range for intcap makes more sense than the full range for pointers
// CHECK: #define __INTCAP_MAX__ 9223372036854775807L

// CHERI128: #define __INTCAP_WIDTH__ 128

// Using the 128/256-bit integer type for __INTPTR_MAX__ does not make sense
// See https://github.com/CTSRD-CHERI/llvm-project/issues/316
// CHECK:     #define __INTPTR_MAX__ 9223372036854775807L
// CHECK:     #define __INTPTR_TYPE__ __intcap_t

// CHERI128: #define __SIZEOF_CHERI_CAPABILITY__ 16
// CHERI128: #define __SIZEOF_INTCAP__ 16

// MIPS: #define __SIZEOF_POINTER__ 8
// PURECAP-128: #define __SIZEOF_POINTER__ 16

// CHERI128: #define __SIZEOF_UINTCAP__ 16
// CHECK:    #define __UINTCAP_MAX__ 18446744073709551615UL
// CHERI128: #define __UINTCAP_WIDTH__ 128

// MIPS:        #define __UINTPTR_MAX__ 18446744073709551615UL
// Using the 128/256-bit integer type for __UINTPTR_MAX__ does not make sense
// See https://github.com/CTSRD-CHERI/llvm-project/issues/316
// CHECK:     #define __UINTPTR_MAX__ 18446744073709551615UL

// MIPS:        #define __UINTPTR_TYPE__ long unsigned int
// PURECAP:     #define __UINTPTR_TYPE__ __uintcap_t
// MIPS:        #define __UINTPTR_WIDTH__ 64
// PURECAP-128: #define __UINTPTR_WIDTH__ 128
