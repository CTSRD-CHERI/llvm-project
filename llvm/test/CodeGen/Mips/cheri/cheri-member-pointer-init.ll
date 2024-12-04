; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - | %cheri_FileCheck %s

; CHECK-LABEL: global_nonvirt_ptr:
; CHECK-NEXT:  .chericap       _ZN1A7nonvirtEv
; CHECK-NEXT:  .8byte  0
; CHECK-NEXT:  .space  [[#CAP_SIZE - 8]]
; CHECK-NEXT:  .size   global_nonvirt_ptr, [[#CAP_SIZE * 2]]

; CHECK-LABEL: global_virt_ptr:
; CHECK-NEXT:  .chericap 0
; CHECK-NEXT:  .8byte  1
; CHECK-NEXT:  .space  [[#CAP_SIZE - 8]]
; CHECK-NEXT:  .size   global_virt_ptr, [[#CAP_SIZE * 2]]

; CHECK-LABEL: global_fn_ptr:
; CHECK-NEXT:  .chericap       _Z9global_fnv
; CHECK-NEXT:  .size   global_fn_ptr, [[#CAP_SIZE]]

; CHECK:       .type   _ZTV1A,@object          # @_ZTV1A
; CHECK-NEXT:  .section .data.rel.ro,"aw",@progbits
; CHECK-NEXT:  .weak   _ZTV1A
; CHECK-NEXT:  .p2align        5
; CHECK-LABEL: _ZTV1A:
; CHECK-NEXT:  .chericap 0
; CHECK-NEXT:  .chericap 0
; CHECK-NEXT:  .chericap       _ZN1A4virtEv
; CHECK-NEXT:  .chericap       _ZN1A5virt2Ev
; CHECK-NEXT:  .size   _ZTV1A, [[#CAP_SIZE * 4]]

@global_nonvirt_ptr = addrspace(200) global { ptr addrspace(200), i64 } { ptr addrspace(200) @_ZN1A7nonvirtEv, i64 0 }, align 32
@global_virt_ptr = addrspace(200) global { ptr addrspace(200), i64 } { ptr addrspace(200) null, i64 1 }, align 32
@global_fn_ptr = addrspace(200) global ptr addrspace(200) @_Z9global_fnv, align 32
@_ZTV1A = linkonce_odr unnamed_addr addrspace(200) constant { [4 x ptr addrspace(200)] } { [4 x ptr addrspace(200)] [ptr addrspace(200) null, ptr addrspace(200) null, ptr addrspace(200) @_ZN1A4virtEv, ptr addrspace(200) @_ZN1A5virt2Ev] }, align 32

define i32 @_Z9global_fnv() addrspace(200) noinline nounwind {
entry:
  ret i32 5
}

define linkonce_odr i32 @_ZN1A7nonvirtEv(ptr addrspace(200) %this) addrspace(200) noinline nounwind align 2 {
entry:
  unreachable
}

define i32 @_Z12call_nonvirtP1A(ptr addrspace(200) %a) addrspace(200) noinline nounwind {
entry:
  unreachable
}

define i32 @_Z9call_virtP1A(ptr addrspace(200) %a) addrspace(200) noinline nounwind {
entry:
  unreachable
}

define i32 @_Z18call_local_nonvirtP1A(ptr addrspace(200) %a) addrspace(200) noinline nounwind {
entry:
  unreachable
}

define linkonce_odr i32 @_ZN1A8nonvirt2Ev(ptr addrspace(200) %this) addrspace(200) noinline nounwind align 2 {
entry:
  unreachable
}

declare ptr addrspace(200) @llvm.cheri.pcc.get() addrspace(200)

declare ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200), i64) addrspace(200)

define i32 @_Z15call_local_virtP1A(ptr addrspace(200) %a) addrspace(200) noinline nounwind {
entry:
  unreachable
}

define i32 @_Z17call_local_fn_ptrP1A(ptr addrspace(200) %a) addrspace(200) noinline nounwind {
entry:
  unreachable
}

define i32 @main() addrspace(200) noinline nounwind {
entry:
  unreachable
}

define linkonce_odr void @_ZN1AC2Ev(ptr addrspace(200) %this) unnamed_addr addrspace(200) noinline nounwind align 2 {
entry:
  unreachable
}

define linkonce_odr i32 @_ZN1A4virtEv(ptr addrspace(200) %this) unnamed_addr addrspace(200) noinline nounwind align 2 {
entry:
  unreachable
}

define linkonce_odr i32 @_ZN1A5virt2Ev(ptr addrspace(200) %this) unnamed_addr addrspace(200) noinline nounwind align 2 {
entry:
  unreachable
}
