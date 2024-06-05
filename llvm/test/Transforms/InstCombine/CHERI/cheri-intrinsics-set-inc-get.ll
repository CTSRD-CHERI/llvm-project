; RUN: opt -S -passes=instcombine %s -o - | FileCheck %s
target datalayout = "pf200:128:128:128:64-A200-P200-G200"

declare i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)*, i64)
declare i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64)

define i64 @fold_set_and_inc_offset_get_offset(i8 addrspace(200)* %arg) #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = getelementptr i8, i8 addrspace(200)* %with_offset, i64 100
  %ret = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %inc_offset)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_and_inc_offset_get_offset(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 142
}

define i64 @no_fold_set_and_inc_offset_get_addr(i8 addrspace(200)* %arg) #1 {
  ; a getaddr can no be inferred from a setoffset since we don't know the base:
  ; However the offset increment can be folded into a single setoffset
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = getelementptr i8, i8 addrspace(200)* %with_offset, i64 100
  %ret = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %inc_offset)
  ret i64 %ret
  ; The %with_offset will be removed by dead code elimination later in the pipeline
  ; CHECK-LABEL: @no_fold_set_and_inc_offset_get_addr(i8 addrspace(200)* %arg)
  ; CHECK: %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg, i64 142)
  ; CHECK: %ret = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %with_offset)
  ; CHECK: ret i64 %ret
}

define i64 @fold_set_and_multiple_inc_offset_get_offset(i8 addrspace(200)* %arg) #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* nonnull %with_offset, i64 100)
  %inc_offset2 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* %inc_offset, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %inc_offset2)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_and_multiple_inc_offset_get_offset(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 242
}

define i64 @fold_set_addr_and_inc_offset_get_offset(i8 addrspace(200)* %arg) #1 {
  ; a getoffset can no be inferred from a setaddr since we don't know the base:
  ; But we acn fold the inc-offset into the setaddr:
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* nonnull %with_offset, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %inc_offset)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_addr_and_inc_offset_get_offset(i8 addrspace(200)* %arg)
  ; CHECK: %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg, i64 142)
  ; CHECK: %ret = tail call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* nonnull %with_offset)
  ; CHECK: ret i64 %ret
}

define i64 @fold_set_addr_and_inc_offset_get_addr(i8 addrspace(200)* %arg) #1 {
  ; a getoffset can no be inferred from a setaddr since we don't know the base:
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* nonnull %with_offset, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %inc_offset)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_addr_and_inc_offset_get_addr(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 142
}

define i64 @fold_set_addr_and_multiple_inc_offset_get_addr(i8 addrspace(200)* %arg) #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* nonnull %with_offset, i64 100)
  %inc_offset2 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* %inc_offset, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %inc_offset2)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_addr_and_multiple_inc_offset_get_addr(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 242
}
