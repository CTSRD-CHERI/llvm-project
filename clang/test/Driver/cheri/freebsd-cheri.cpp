// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64-unknown-freebsd -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64-unknown-freebsd9 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBSTDCXX %s
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64-unknown-freebsd12 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s

// purecap should always use libstdc++
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64-unknown-freebsd9 -stdlib=platform -mabi=purecap 2>&1 | FileCheck --check-prefix=LIBCXX %s
// So should any triple with a CHERI CPU
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64c128hybrid-unknown-freebsd9 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s

// And all cheri-*-*- triples
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target cheri-unknown-freebsd -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target cheri-unknown-freebsd9 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target cheri-unknown-freebsd12 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s

// Also check the -cheri flag toggles libc++
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64-unknown-freebsd12 -cheri=128 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s
// RUN: %plain_clang_cheri_triple_allowed --driver-mode=g++ %s -### -o %t.o -target mips64-unknown-freebsd12 -cheri=256 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s


// LIBCXX-NOT: "-lstdc++"
// LIBCXX: "-lc++"
// LIBCXX-NOT: "-lstdc++"

// LIBSTDCXX: "-lstdc++"

