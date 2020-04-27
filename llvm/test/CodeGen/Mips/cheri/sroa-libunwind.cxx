// RUNs: %cheri_purecap_cc1 -munwind-tables -mllvm -cheri-cap-table-abi=pcrel -dwarf-column-info -debug-info-kind=line-tables-only -dwarf-version=5 -debugger-tuning=gdb -nobuiltininc -cheri-uintcap=offset -O2 -std=c++11 -mstack-alignment=16 -S %s -emit-llvm -o %s.opt.ll
// RUNs: %cheri128_purecap_cc1 -munwind-tables -mllvm -cheri-cap-table-abi=pcrel -dwarf-column-info -debug-info-kind=standalone -dwarf-version=2 -debugger-tuning=gdb -nobuiltininc -cheri-uintcap=offset -O2 -std=c++11 -mstack-alignment=16 -o - -S %s
// RUN: %cheri128_purecap_cc1 -cheri-uintcap=offset -O2 -std=c++11 -o - -emit-llvm %s | FileCheck %s
// Check that this no longer crashes:
// RUN: %cheri128_purecap_cc1 -cheri-uintcap=offset -O2 -std=c++11 -o /dev/null -S %s -verify
// REQUIRES: clang


// This previously caused crashes when casting __int128 to __uintcap_t

extern "C" __uintcap_t minimal_test_case(__int128 y) {
   // expected-warning@-1{{found underaligned load of capability type (aligned to 1 bytes}}
   // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  __uintcap_t r;
  __builtin_memcpy(&r, (void *)(__uintcap_t)y, sizeof(r));
  return r;
  // CHECK:      [[CONV:%.+]] = trunc i128 %y to i64
  // CHECK-NEXT: [[CAP:%.+]] = getelementptr i8, i8 addrspace(200)* null, i64 [[CONV]]
  // CHECK-NEXT: [[SROA_CAST:%.+]] = bitcast i8 addrspace(200)* [[CAP]] to i8 addrspace(200)* addrspace(200)*
  // Alignment is 1 here since we can't know what y contained:
  // CHECK-NEXT: %r.0.copyload = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[SROA_CAST]], align 1
  // CHECK-NEXT: ret i8 addrspace(200)* %r.0.copyload
}
extern "C" __uintcap_t minimal_test_case2(__int128 y) {
  __uintcap_t r;
  __builtin_memcpy(&r, (__uintcap_t*)(void *)(__uintcap_t)y, sizeof(r));
  return r;
  // CHECK:      [[CONV:%.+]] = trunc i128 %y to i64
  // CHECK-NEXT: [[CAP:%.+]] = getelementptr i8, i8 addrspace(200)* null, i64 [[CONV]]
  // CHECK-NEXT: [[SROA_CAST:%.+]] = bitcast i8 addrspace(200)* [[CAP]] to i8 addrspace(200)* addrspace(200)*
  // Alignment is 16/32 due to the explicit cast to __uintcap_t*
  // CHECK-NEXT: %r.0.copyload = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[SROA_CAST]], align 16
  // CHECK-NEXT: ret i8 addrspace(200)* %r.0.copyload
}

extern "C" __uintcap_t minimal_test_case3(__int128 y) {
  __uintcap_t r;
  // This is out of bounds for CHERI256: __builtin_memcpy(&r, &y, sizeof(r));
  __builtin_memcpy(&r, &y, sizeof(y));
  // CHECK:      %y.addr.sroa.0 = alloca i8 addrspace(200)*, align 16, addrspace(200)
  // CHECK-NEXT: %tmpcast = bitcast i8 addrspace(200)* addrspace(200)* %y.addr.sroa.0 to i128 addrspace(200)*
  // CHECK-NEXT: store i128 %y, i128 addrspace(200)* %tmpcast, align 16,
  // CHECK-NEXT: %y.addr.sroa.0.0.y.addr.sroa.0.0.y.addr.0.r.0.copyload = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %y.addr.sroa.0, align 16
  // CHECK-NEXT: ret i8 addrspace(200)* %y.addr.sroa.0.0.y.addr.sroa.0.0.y.addr.0.r.0.copyload
  return r;
}

// extern "C" __uintcap_t minimal_test_case2(char* buf) {
//   __uintcap_t r;
//   memcpy(&r, buf, sizeof(r));
//   return r;
// }
