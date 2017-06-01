// RUN: %clangxx %s -### -o %t.o -target cheri-unknown-freebsd -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s
// RUN: %clangxx %s -### -o %t.o -target cheri-unknown-freebsd9 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s
// RUN: %clangxx %s -### -o %t.o -target cheri-unknown-freebsd12 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s

// RUN: %clangxx %s -### -o %t.o -target mips64-unknown-freebsd -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBSTDCXX %s
// RUN: %clangxx %s -### -o %t.o -target mips64-unknown-freebsd9 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBSTDCXX %s
// RUN: %clangxx %s -### -o %t.o -target mips64-unknown-freebsd12 -stdlib=platform 2>&1 | FileCheck --check-prefix=LIBCXX %s

// LIBCXX-NOT: "-lstdc++"
// LIBCXX: "-lc++"
// LIBCXX-NOT: "-lstdc++"

// LIBSTDCXX: "-lstdc++"

