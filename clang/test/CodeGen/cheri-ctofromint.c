// RUN: %clang_cc1 %s "-target-abi" "purecap" -emit-llvm -triple cheri-unknown-freebsd -o - -cheri-linker | FileCheck --check-prefix=CHECK-PURECAP %s
// RUN: %clang_cc1 %s -emit-llvm -triple cheri-unknown-freebsd -o - -cheri-linker | FileCheck --check-prefix=CHECK-HYBRID %s


__capability void* foo(__capability void *x){
  // CHECK-HYBRID: ptrtoint
  // CHECK-PURECAP: llvm.cheri.cap.offset.get
  int pi = (int)x; // pi contains the result of CToPtr x, which is probably null
  // CHECK-HYBRID: inttoptr
  // CHECK-PURECAP: llvm.cheri.cap.offset.set
  return (__capability void*)pi;
}
