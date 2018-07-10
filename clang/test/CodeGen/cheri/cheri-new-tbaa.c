// REQUIRES: asserts
// RUN: %cheri_cc1 -emit-llvm -no-struct-path-tbaa -target-abi n64 -O2 -o - %s | FileCheck %s -check-prefixes NO-STRUCT-PATH
// RUN: %cheri_cc1 -emit-llvm -new-struct-path-tbaa -target-abi n64 -O2 -o - %s | FileCheck %s -check-prefixes STRUCT-PATH
// Check that this no longer crashes:
// RUN: %cheri_cc1 -S -no-struct-path-tbaa -target-abi n64 -O2 -o /dev/null %s
// RUN: %cheri_cc1 -S -new-struct-path-tbaa -target-abi n64 -O2 -o /dev/null %s
long a;
extern int c(void);

void b() {
  if (*(unsigned *)a)
    c();
}

// NO-STRUCT-PATH: !2 = !{!3, !3, i64 0}
// NO-STRUCT-PATH: !3 = !{!"long", !4, i64 0}
// NO-STRUCT-PATH: !4 = !{!"omnipotent char", !5, i64 0}
// NO-STRUCT-PATH: !5 = !{!"Simple C/C++ TBAA"}
// NO-STRUCT-PATH: !6 = !{!7, !7, i64 0}
// NO-STRUCT-PATH: !7 = !{!"int", !4, i64 0}

// STRUCT-PATH:    !2 = !{!3, !3, i64 0, i64 8}
// STRUCT-PATH:    !3 = !{!4, i64 8, !"long"}
// STRUCT-PATH:    !4 = !{!5, i64 1, !"omnipotent char"}
// STRUCT-PATH:    !5 = !{!"Simple C/C++ TBAA"}
// STRUCT-PATH:    !6 = !{!7, !7, i64 0, i64 4}
// STRUCT-PATH:    !7 = !{!4, i64 4, !"int"}
