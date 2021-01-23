; RUN: %cheri_purecap_llc -O0 %s -o - | FileCheck %s


define void @dynamic_alloca(i32 signext %x) local_unnamed_addr nounwind {
entry:
  ; CHECK: cgetaddr	$[[SPOFFSET:([0-9]+|sp)]], $c11
  ; CHECK: daddiu	$[[MASK:([0-9]+|sp)]], $zero, -64
  ; CHECK: and	$[[NEWSPOFFSET:([0-9]+|sp)]], $[[SPOFFSET]], $[[MASK]]
  ; CHECK: csetaddr	$c11, $c11, $[[NEWSPOFFSET]]

  %0 = zext i32 %x to i64
  %vla = alloca i32, i64 %0, align 64, addrspace(200)
  call void @use_arg(i32 addrspace(200)* nonnull %vla) nounwind
  ret void
}

declare void @use_arg(i32 addrspace(200)*) nounwind
