// RUN: %cheri_clang -### -no-canonical-prefixes %s 2>&1 | FileCheck -check-prefix HYBRID %s
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -r
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 | FileCheck -check-prefix PURECAP %s
// HYBRID-NOT: "-pie"
// PURECAP: "{{.+}}/ld" "-pie"

// But for -static, -r, and -pie we don't want to add -pie
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -static | FileCheck -check-prefix PURECAP-STATIC %s
// PURECAP-STATIC: "{{.+}}/ld"
// PURECAP-STATIC-NOT: "-pie"
// PURECAP-STATIC:     "-Bstatic"
// PURECAP-STATIC-NOT: "-pie"

// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -shared | FileCheck -check-prefix PURECAP-SHARED %s
// PURECAP-SHARED: "{{.+}}/ld"
// PURECAP-SHARED-NOT: "-pie"
// PURECAP-SHARED: "-Bshareable"
// PURECAP-SHARED-NOT: "-pie"

// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -r | FileCheck -check-prefix PURECAP-RELOCATABLE %s
// PURECAP-RELOCATABLE: "{{.+}}/ld"
// PURECAP-RELOCATABLE-NOT: "-pie"
// PURECAP-RELOCATABLE: "-r"
// PURECAP-RELOCATABLE-NOT: "-pie"
