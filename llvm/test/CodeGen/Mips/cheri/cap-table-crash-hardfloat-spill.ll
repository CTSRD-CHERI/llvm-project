; RUN: %cheri_purecap_llc -o - %s
; XFAIL: *
; SDValue MipsTargetLowering::lowerConstantPool(SDValue Op, SelectionDAG &DAG) const needs updating
%struct.a = type { }
define void @b() {
  %c = fmul double undef, undef
  call void @d(%struct.a addrspace(200)* undef, %struct.a addrspace(200)* undef, i32 undef, double %c)
  unreachable
}
declare void @d(%struct.a addrspace(200)*, %struct.a addrspace(200)*, i32 , double)
