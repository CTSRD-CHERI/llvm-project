; RUN: %cheri_opt -S -cheri-fold-intrisics %s -o -
; RUN: %cheri_opt -S -cheri-fold-intrisics %s -o - | FileCheck %s

declare i64 @check_fold(i64) #1
declare i64 @llvm.cheri.cap.base.get.i64(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.base.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*) #1
declare i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64) #1
declare i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)*, i64) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.length.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)*) #1


define i8 addrspace(200)* @fold_offset_get_set(i8 addrspace(200)* %arg) #1 {
  %offset = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %arg)
  %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg, i64 %offset)
  ret i8 addrspace(200)* %ret
  ; This is a no-op and should be folded to ret %arg
  ; CHECK-LABEL: @fold_offset_get_set(i8 addrspace(200)* %arg)
  ; CHECK: ret i8 addrspace(200)* %arg
}

; But check that we don't do the same folding when using different base caps for get/set
define i8 addrspace(200)* @fold_offset_get_set_different_cap(i8 addrspace(200)* %arg, i8 addrspace(200)* %arg2) #1 {
  %offset = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %arg)
  %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg2, i64 %offset)
  ret i8 addrspace(200)* %ret
  ; CHECK-LABEL: @fold_offset_get_set_different_cap(
  ; CHECK-NEXT: %offset = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %arg)
  ; CHECK-NEXT: %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg2, i64 %offset)
  ; CHECK-NEXT: ret i8 addrspace(200)* %ret
}

define i8 addrspace(200)* @fold_offset_get_add_set(i8 addrspace(200)* %arg) #1 {
  %offset = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %arg)
  %new_offset = add i64 %offset, 22
  %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg, i64 %new_offset)
  ret i8 addrspace(200)* %ret
  ; CHECK-LABEL: @fold_offset_get_add_set(i8 addrspace(200)* %arg)
  ; CHECK: [[RESULT:%.+]] = getelementptr i8, i8 addrspace(200)* %arg, i64 22
  ; CHECK: ret i8 addrspace(200)* [[RESULT]]
}

define i8 addrspace(200)* @fold_addr_get_set(i8 addrspace(200)* %arg) #1 {
  %addr = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %arg)
  %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg, i64 %addr)
  ret i8 addrspace(200)* %ret
  ; CHECK-LABEL: @fold_addr_get_set(i8 addrspace(200)* %arg)
  ; CHECK: ret i8 addrspace(200)* %arg
}

; But check that we don't do the same folding when using different base caps for get/set
define i8 addrspace(200)* @fold_addr_get_set_different_cap(i8 addrspace(200)* %arg, i8 addrspace(200)* %arg2) #1 {
  %addr = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %arg)
  %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg2, i64 %addr)
  ret i8 addrspace(200)* %ret
  ; CHECK-LABEL: @fold_addr_get_set_different_cap(
  ; CHECK-NEXT: %addr = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %arg)
  ; CHECK-NEXT: %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg2, i64 %addr)
  ; CHECK-NEXT: ret i8 addrspace(200)* %ret
}

define i8 addrspace(200)* @fold_addr_get_add_set(i8 addrspace(200)* %arg) #1 {
  %offset = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %arg)
  %new_offset = add i64 %offset, 25
  %ret = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg, i64 %new_offset)
  ret i8 addrspace(200)* %ret
  ; CHECK-LABEL: @fold_addr_get_add_set(i8 addrspace(200)* %arg)
  ; CHECK: [[RESULT:%.+]] = getelementptr i8, i8 addrspace(200)* %arg, i64 25
  ; CHECK: ret i8 addrspace(200)* [[RESULT]]
}



attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
