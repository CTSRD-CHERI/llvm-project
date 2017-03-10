// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -std=c++11 -o - %s | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -S -std=c++11 -o - %s | FileCheck -check-prefix=ASM %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -ast-dump %s | FileCheck -check-prefix=AST %s
// reduced testcase for libcxx exception_fallback.ipp/new_handler_fallback.ipp


typedef void (*handler)();
__attribute__((__require_constant_initialization__)) static handler __handler;

handler set_handler_sync(handler func) noexcept {
  //ASM: cllc
  //ASM: cscc
  return __sync_lock_test_and_set(&__handler, func);
  // CHECK-NOT: ptrtoint void () addrspace(200)* %0 to i256
  // CHECK-NOT: atomicrmw xchg i256 addrspace(200)* bitcast (void () addrspace(200)* addrspace(200)* @_ZL9__handler to i256 addrspace(200)*), i256 %1 seq_cst
  // CHECK-NOT: inttoptr i256 %2 to void () addrspace(200)*
  // CHECK: atomicrmw xchg i256 addrspace(200)* bitcast (void () addrspace(200)* addrspace(200)* @_ZL9__handler to i256 addrspace(200)*), i256 %1 seq_cst
  // AST-NOT: DeclRefExpr {{.*}} '__sync_lock_test_and_set_16' '__int128 (volatile __int128 * __capability, __int128, ...) noexcept'
}

handler get_handler_sync() noexcept {
  // CHECK-NOT: atomicrmw add i256 addrspace(200)* bitcast (void () addrspace(200)* addrspace(200)* @_ZL9__handler to i256 addrspace(200)*), i256 0 seq_cst
  return __sync_fetch_and_add(&__handler, (handler)0);
  // AST-NOT: DeclRefExpr {{.*}} '__sync_fetch_and_add_16' '__int128 (volatile __int128 * __capability, __int128, ...) noexcept'
}

handler set_handler_atomic(handler func) noexcept {
  // CHECK-NOT: bitcast void () addrspace(200)* addrspace(200)* %.atomictmp to i256 addrspace(200)*
  // CHECK-NOT: bitcast void () addrspace(200)* addrspace(200)* %atomic-temp to i256 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %1 to i8 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %2 to i8 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %2 to void () addrspace(200)* addrspace(200)*
  return __atomic_exchange_n(&__handler, func, __ATOMIC_SEQ_CST);
}

handler get_handler_atomic() noexcept {
  // CHECK-NOT: bitcast void () addrspace(200)* addrspace(200)* %atomic-temp to i256 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %0 to i8 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %0 to void () addrspace(200)* addrspace(200)*
  return __atomic_load_n(&__handler, __ATOMIC_SEQ_CST);
}

__attribute__((__require_constant_initialization__)) static _Atomic(handler) __atomic_handler;

handler set_handler_c11_atomic(handler func) noexcept {
  // CHECK-NOT: bitcast void () addrspace(200)* addrspace(200)* %.atomictmp to i256 addrspace(200)*
  // CHECK-NOT: bitcast void () addrspace(200)* addrspace(200)* %atomic-temp to i256 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %1 to i8 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %2 to i8 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %2 to void () addrspace(200)* addrspace(200)*
  return __c11_atomic_exchange(&__atomic_handler, func, __ATOMIC_SEQ_CST);
}

handler get_handler_c11_atomic() noexcept {
  // CHECK-NOT: bitcast void () addrspace(200)* addrspace(200)* %atomic-temp to i256 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %0 to i8 addrspace(200)*
  // CHECK-NOT: bitcast i256 addrspace(200)* %0 to void () addrspace(200)* addrspace(200)*
  return __c11_atomic_load(&__atomic_handler, __ATOMIC_SEQ_CST);
}

