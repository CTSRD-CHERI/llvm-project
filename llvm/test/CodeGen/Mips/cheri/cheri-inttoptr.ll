; RUN: %cheri_purecap_llc %s -o - | FileCheck %s

declare noalias nonnull i8 addrspace(200)* @_Znwm(i64 zeroext) local_unnamed_addr

define void @_Z1rv() local_unnamed_addr {
entry:
  %call = tail call i8 addrspace(200)* @_Znwm(i64 zeroext 80)
  ; CHECK: cincoffset [[DST:\$c.*]], $cnull, 80
  ; CHECK-NEXT: csc $c{{.*}}, $zero, 0([[DST]])
  store i8 addrspace(200)* %call, i8 addrspace(200)* addrspace(200)* inttoptr (i64 80 to i8 addrspace(200)* addrspace(200)*), align 32
  ret void
}
