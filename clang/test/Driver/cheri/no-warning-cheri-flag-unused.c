// RUN: touch %t.o
// Check that cheri flags don't cause warnings when linking:
// RUN: %clang -g -integrated-as -Werror -Werror=unused-command-line-argument --target=cheri-unknown-freebsd -msoft-float \
// RUN:   -cheri=128 \
// RUN:   -cheri-cap-table-abi=plt \
// RUN:   -cheri \
// RUN:   -cheri-uintcap=offset \
// RUN:   -cheri-uintcap=addr \
// RUN:   -cheri \
// RUN:   -Xclang -cheri-bounds=subobject-safe \
// RUN:   -mxcaptable \
// RUN:   -no-mxcaptable \
// RUN:   -mabi=purecap -shared %t.o -### 2>&1 | FileCheck %s
// CHECK-NOT: [-Werror,

