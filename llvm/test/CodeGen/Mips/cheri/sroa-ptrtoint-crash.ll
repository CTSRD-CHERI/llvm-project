; RUN: %cheri_opt -sroa -o - -S %s
; TODO: %cheri_opt -sroa -o - -S -O2 %s
; ModuleID = '/Users/alex/cheri/build/llvm-clion-build/libcxx-filesystem-crash.cpp-reduce.ll-reduced-simplified.bc'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cheri/libcxx-filesystem-crash.cpp-reduce.ll"
target datalayout = "Eme-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "x86_64-apple-darwin18.2.0"

%class.duration.i128 = type { i128 }

define i128 @c(%class.duration.i128* %d) {
entry:
  %i.e = alloca i8 addrspace(200)*
  %f = bitcast i8 addrspace(200)** %i.e to %class.duration.i128*
  %0 = bitcast %class.duration.i128* %d to i8 addrspace(200)**
  %1 = load i8 addrspace(200)*, i8 addrspace(200)** %0
  store i8 addrspace(200)* %1, i8 addrspace(200)** %i.e
  %call = call i128 @g(%class.duration.i128* %f)
  ret i128 %call
}

define i128 @g(%class.duration.i128* %h) {
entry:
  %as = getelementptr %class.duration.i128, %class.duration.i128* %h, i64 0, i32 0
  %0 = load i128, i128* %as, align 16
  ret i128 %0
}
