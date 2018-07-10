// CHERI triple without any flags should set 128 defines
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11  \
// RUN:  -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI128 %s
// or with -cheri
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI128 %s
// or with -cheri=128
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri=128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI128 %s
// or with both
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri=128 -mcpu=cheri128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI128 %s

// now try -mcpu=256:
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -mcpu=cheri256 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI256 %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -mcpu=cheri256 -mabi=purecap \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI256,CHERI256-PURECAP %s
// or with -cheri=256
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri=256 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI256 %s
// or with both
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -cheri=256 -mcpu=cheri256 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI256 %s

// Try with -march=mips64
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -mcpu=mips4 -cheri=128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI128 %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -mcpu=mips4 -cheri=256 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI256 %s
// and -mcpu=mips4
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -march=mips64 -cheri=128 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI128 %s
// RUN: %plain_clang_cheri_triple_allowed -target cheri-unknown-freebsd11 -march=mips64 -cheri=256 \
// RUN:   -mabi=purecap -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=CHERI256 %s

// now try a non-cheri triple but -mcpu=cheri{128,256}
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -mcpu=cheri128 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS-CHERI128 -DARCHNAME=CHERI128 %s
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -mcpu=cheri256 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS-CHERI256 -DARCHNAME=CHERI256 %s

// now try a non-cheri triple with -cheri
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -cheri \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS-CHERI128 -DARCHNAME=MIPS64R2 %s
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -cheri=128 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS-CHERI128 -DARCHNAME=MIPS64R2 %s
// RUN: %plain_clang_cheri_triple_allowed -target mips64-unknown-freebsd11 -cheri=256 \
// RUN:   -mabi=n64 -E -dM -xc /dev/null 2>&1 | FileCheck --check-prefixes=MIPS-CHERI256 -DARCHNAME=MIPS64R2 %s


// CHERI128: #define _MIPS_CAP_ALIGN_MASK 0xfffffffffffffff0
// CHERI128: #define _MIPS_SZCAP 128
// CHERI128: #define __CHERI_PURE_CAPABILITY__ 2
// CHERI128: #define __CHERI__ 1
// CHERI128: #define __SIZEOF_POINTER__ 16
// CHERI128: #define __UINTPTR_TYPE__ __uintcap_t
// CHERI128  #define __UINTPTR_MAX__ 340282366920938463463374607431768211455

// CHERI256: #define _MIPS_CAP_ALIGN_MASK 0xffffffffffffffe0
// CHERI256: #define _MIPS_SZCAP 256
// CHERI256: #define __CHERI_PURE_CAPABILITY__ 2
// CHERI256: #define __CHERI__ 1
// CHERI256: #define __SIZEOF_POINTER__ 32
// CHERI256: #define __UINTPTR_TYPE__ __uintcap_t
// CHERI256  #define __UINTPTR_MAX__ 115792089237316195423570985008687907853269984665640564039457584007913129639935

// MIPS-CHERI128: #define _MIPS_ARCH_[[ARCHNAME]] 1
// MIPS-CHERI128: #define _MIPS_CAP_ALIGN_MASK 0xfffffffffffffff0
// MIPS-CHERI128: #define _MIPS_SZCAP 128
// MIPS-CHERI128-NOT: #define __CHERI_PURE_CAPABILITY__
// MIPS-CHERI128: #define __CHERI__ 1
// MIPS-CHERI128: #define __SIZEOF_POINTER__ 8
// MIPS-CHERI128: #define __UINTPTR_TYPE__ long unsigned int


// MIPS-CHERI256: #define _MIPS_ARCH_[[ARCHNAME]] 1
// MIPS-CHERI256: #define _MIPS_CAP_ALIGN_MASK 0xffffffffffffffe0
// MIPS-CHERI256: #define _MIPS_SZCAP 256
// MIPS-CHERI256-NOT: #define __CHERI_PURE_CAPABILITY__
// MIPS-CHERI256: #define __CHERI__ 1
// MIPS-CHERI256: #define __SIZEOF_POINTER__ 8
// MIPS-CHERI256: #define __UINTPTR_TYPE__ long unsigned int
