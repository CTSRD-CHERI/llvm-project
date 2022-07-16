// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -emit-llvm -std=c++11 -fsyntax-only -verify %s
// RUN: %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | %cheri_FileCheck %s
// RUN: %cheri_purecap_cc1 -O2 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -S -o - %s | %cheri_FileCheck -check-prefix=ASM -enable-var-scope %s
// RUN: %cheri_purecap_cc1 -std=c++11 -ast-dump %s | %cheri_FileCheck -check-prefix=AST %s
// reduced testcase for libcxx exception_fallback.ipp/new_handler_fallback.ipp

// Module ID and source_filename might contain i256 so we explicitly capture this line
// CHECK: ; ModuleID = '{{.+}}/cheri-std-unexpected-atomicrmw.cpp'
// CHECK: source_filename = "{{.+}}"

// expected-no-diagnostics

typedef void (*handler)();
__attribute__((__require_constant_initialization__)) static handler __handler;
handler set_handler_sync(handler func) noexcept {
  return __sync_lock_test_and_set(&__handler, func);
  // AST: DeclRefExpr {{.*}} '__sync_lock_test_and_set_cap' '__intcap (volatile __intcap *, __intcap, ...) noexcept'
}

handler get_handler_sync() noexcept {
  return __sync_fetch_and_add(&__handler, (handler)0);
  // AST: DeclRefExpr {{.*}} '__sync_fetch_and_add_cap' '__intcap (volatile __intcap *, __intcap, ...) noexcept'
}

handler set_handler_atomic(handler func) noexcept {
  // CHECK-LABEL: @_Z18set_handler_atomicPFvvE(
  // CHECK: %func.addr = alloca void () addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK: %.atomictmp = alloca void () addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK: %atomic-temp = alloca void () addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK: store void () addrspace(200)* %func, void () addrspace(200)* addrspace(200)* %func.addr, align [[#CAP_SIZE]]
  // CHECK: [[VAR_0:%.+]] = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* %func.addr, align [[#CAP_SIZE]]
  // CHECK: store void () addrspace(200)* [[VAR_0]], void () addrspace(200)* addrspace(200)* %.atomictmp, align [[#CAP_SIZE]]
  // CHECK: [[VAR_1:%.+]] = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* %.atomictmp, align [[#CAP_SIZE]]
  // CHECK: atomicrmw xchg void () addrspace(200)* addrspace(200)* @_ZL9__handler, void () addrspace(200)* [[VAR_1]] seq_cst
  return __atomic_exchange_n(&__handler, func, __ATOMIC_SEQ_CST);

  // ASM-LABEL: _Z18set_handler_atomicPFvvE:
  // ASM:      clcbi   $c2, %captab20(_ZL9__handler)($c{{.+}})
  // ASM-NEXT: sync
  // ASM-NEXT: .LBB2_1: # %entry
  // ASM-NEXT: # =>This Inner Loop Header: Depth=1
  // ASM-NEXT: cllc    $c1, $c2
  // ASM-NEXT: cscc    $1, $c3, $c2
  // ASM-NEXT: beqz    $1, .LBB2_1
  // ASM-NEXT: nop
  // ASM-NEXT: # %bb.2: # %entry
  // ASM-NEXT: sync
  // ASM: .end _Z18set_handler_atomicPFvvE
}

handler get_handler_atomic() noexcept {
  // CHECK-LABEL: @_Z18get_handler_atomicv(
  // CHECK: load atomic void () addrspace(200)*, void () addrspace(200)* addrspace(200)* @_ZL9__handler seq_cst
  return __atomic_load_n(&__handler, __ATOMIC_SEQ_CST);

  // ASM-LABEL: _Z18get_handler_atomicv:
  // This should just use a load + sync
  // ASM: clcbi   [[HANDLER_ADDR:\$c[0-9]+]], %captab20(_ZL9__handler)($c{{.+}})
  // ASM: clc     $c3, $zero, 0([[HANDLER_ADDR]])
  // ASM: sync
  // ASM: .end _Z18get_handler_atomicv

}

__attribute__((__require_constant_initialization__)) static _Atomic(handler) __atomic_handler;

handler set_handler_c11_atomic(handler func) noexcept {
  // CHECK-LABEL: @_Z22set_handler_c11_atomicPFvvE(
  // CHECK: %func.addr = alloca void () addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK: %.atomictmp = alloca void () addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK: %atomic-temp = alloca void () addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK: store void () addrspace(200)* %func, void () addrspace(200)* addrspace(200)* %func.addr, align [[#CAP_SIZE]]
  // CHECK: [[VAR_0:%.+]] = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* %func.addr, align [[#CAP_SIZE]]
  // CHECK: store void () addrspace(200)* [[VAR_0]], void () addrspace(200)* addrspace(200)* %.atomictmp, align [[#CAP_SIZE]]
  // CHECK: [[VAR_1:%.+]] = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* %.atomictmp, align [[#CAP_SIZE]]
  // CHECK: atomicrmw xchg void () addrspace(200)* addrspace(200)* @_ZL16__atomic_handler, void () addrspace(200)* [[VAR_1]] seq_cst
  return __c11_atomic_exchange(&__atomic_handler, func, __ATOMIC_SEQ_CST);

  // ASM-LABEL: _Z22set_handler_c11_atomicPFvvE:
  // ASM:      clcbi   $c2, %captab20(_ZL16__atomic_handler)($c{{.+}})
  // ASM-NEXT: sync
  // ASM-NEXT: .LBB4_1: # %entry
  // ASM-NEXT: # =>This Inner Loop Header:
  // ASM-NEXT: cllc    $c1, $c2
  // ASM-NEXT: cscc    $1, $c3, $c2
  // ASM-NEXT: beqz    $1, .LBB4_1
  // ASM-NEXT: nop
  // ASM-NEXT: # %bb.2: # %entry
  // ASM-NEXT: sync
  // ASM: .end _Z22set_handler_c11_atomicPFvvE

}

handler get_handler_c11_atomic() noexcept {
  // CHECK-LABEL: @_Z22get_handler_c11_atomicv()
  // CHECK: load atomic void () addrspace(200)*, void () addrspace(200)* addrspace(200)* @_ZL16__atomic_handler seq_cst

  return __c11_atomic_load(&__atomic_handler, __ATOMIC_SEQ_CST);
  // ASM-LABEL: _Z22get_handler_c11_atomicv:
  // This should just use a load + sync
  // ASM: clcbi   [[HANDLER_ADDR:\$c[0-9]+]], %captab20(_ZL16__atomic_handler)($c{{.+}})
  // ASM: clc     $c3, $zero, 0([[HANDLER_ADDR]])
  // ASM: sync
  // ASM: .end _Z22get_handler_c11_atomicv
}
