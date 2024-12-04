; RUN: %cheri_opt -passes=sroa -o - -S %s
; TODO: %cheri_opt -passes=sroa -o - -S -O2 %s
; ModuleID = '/Users/alex/cheri/build/llvm-clion-build/libcxx-filesystem-crash.cpp-reduce.ll-reduced-simplified.bc'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cheri/libcxx-filesystem-crash.cpp-reduce.ll"
target datalayout = "Eme-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "x86_64-apple-darwin18.2.0"

%class.duration.i128 = type { i128 }

define i128 @c(ptr %d) {
entry:
  %i.e = alloca ptr addrspace(200), align 16
  %f = bitcast ptr %i.e to ptr
  %0 = bitcast ptr %d to ptr
  %1 = load ptr addrspace(200), ptr %0, align 16
  store ptr addrspace(200) %1, ptr %i.e, align 16
  %call = call i128 @g(ptr %f)
  ret i128 %call
}

define i128 @g(ptr %h) {
entry:
  %as = getelementptr %class.duration.i128, ptr %h, i64 0, i32 0
  %0 = load i128, ptr %as, align 16
  ret i128 %0
}
