// PR691
// RUN: %clang_cc1 -no-opaque-pointers %s -emit-llvm -o - | FileCheck %s
// CHECK: call i8* @llvm.stacksave.p0i8()

void test(int N) {
  int i;
  for (i = 0; i < N; ++i) {
    int VLA[i];
    external(VLA);
  }
}
