; RUN: %cheri_purecap_llc %s -o - | FileCheck %s

declare noalias nonnull ptr addrspace(200) @_Znwm(i64 zeroext) local_unnamed_addr

define void @_Z1rv() local_unnamed_addr {
entry:
  %call = tail call ptr addrspace(200) @_Znwm(i64 zeroext 80)
  ; CHECK: cincoffset [[DST:\$c.*]], $cnull, 80
  ; CHECK-NEXT: csc $c{{.*}}, $zero, 0([[DST]])
  store ptr addrspace(200) %call, ptr addrspace(200) inttoptr (i64 80 to ptr addrspace(200)), align 32
  ret void
}
