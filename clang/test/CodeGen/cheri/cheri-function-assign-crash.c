// check that it doesn't crash
// RUN: %cheri_purecap_cc1 -w -emit-obj -O2 -std=gnu99 -o /dev/null %s
// next check that it emits sensible IR:
// RUN: %cheri_purecap_cc1 -w -O2 -std=gnu99 -o - -emit-llvm %s -mllvm -cheri-cap-table-abi=legacy | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_purecap_cc1 -w -O2 -std=gnu99 -DNEW_ABI -o - -emit-llvm %s -mllvm -cheri-cap-table-abi=pcrel | FileCheck %s -check-prefix PCREL

// https://github.com/CTSRD-CHERI/clang/issues/144

int a();
#ifdef NEW_ABI
// unless we mark a as weak the compiler will optimize out the if (a) case since it assumed to be non-null
#pragma weak a
#endif
int d();
int e();
int *b() {
  int *c;
  if (a)
    d();
  c = a;
  if (c == 0)
    return 0;
  e();
  return c;
}
// LEGACY: %0 = tail call i8 addrspace(200)* @llvm.cheri.pcc.get()
// LEGACY: %1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %0, i64 ptrtoint (i32 (...)* @a to i64))
// LEGACY: %2 = bitcast i8 addrspace(200)* %1 to i32 (...) addrspace(200)*
// LEGACY: [[A_ADDR:%3]] = bitcast i32 (...) addrspace(200)* %2 to i32 addrspace(200)*
// LEGACY: {{%.+}} = phi i32 addrspace(200)* [ [[A_ADDR]], {{%.+}} ], [ null, {{%.+}}]{{$}}
// With pcrel the optimizer can ignore all the intrinsic calls:
// PCREL: {{%.+}} = phi i32 addrspace(200)* [ bitcast (i32 (...) addrspace(200)* @a to i32 addrspace(200)*), {{%.+}} ], [ null, {{%.+}}]{{$}}
