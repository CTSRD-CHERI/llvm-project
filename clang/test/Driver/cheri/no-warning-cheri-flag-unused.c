// RUN: touch %t.o
// RUN: %clang -g -integrated-as -Werror --target=cheri-unknown-freebsd -msoft-float -cheri=128 -Wl,--no-warn-mismatch -EB -mabi=purecap -Wl,-melf64btsmip_cheri_fbsd -fuse-ld=lld -Wl,-preemptible-caprelocs=elf -Wl,-z,norelro -Wl,--version-script=Version.map -shared -Wl,-x -Wl,--fatal-warnings -Wl,--warn-shared-textrel -o libprocstat.so.1 -Wl,-soname,libprocstat.so.1 %t.o -### 2>&1 | FileCheck %s
// CHECK-NOT: [-Werror,

