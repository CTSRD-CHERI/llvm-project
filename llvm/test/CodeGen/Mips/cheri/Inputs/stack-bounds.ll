; ModuleID = '/Users/alex/cheri/llvm-project/llvm/test/CodeGen/Mips/cheri/stack-bounds-pass.c'
source_filename = "/Users/alex/cheri/llvm-project/llvm/test/CodeGen/Mips/cheri/stack-bounds-pass.c"
target datalayout = "E-m:e-pf200:256:256:256:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
target triple = "cheri-unknown-freebsd-purecap"

; Function Attrs: noinline nounwind
define i8 addrspace(200)* @return_stack() addrspace(200) #0 {
entry:
  %buffer = alloca [16 x i8], align 1, addrspace(200)
  %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
  ret i8 addrspace(200)* %arraydecay
}

; Function Attrs: noinline nounwind
define signext i32 @pass_arg() addrspace(200) #0 {
entry:
  %x = alloca i32, align 4, addrspace(200)
  %result = alloca i32 addrspace(200)*, align 32, addrspace(200)
  store i32 5, i32 addrspace(200)* %x, align 4
  %call = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
  store i32 addrspace(200)* %call, i32 addrspace(200)* addrspace(200)* %result, align 32
  %0 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %result, align 32
  %1 = load i32, i32 addrspace(200)* %0, align 4
  %2 = load i32, i32 addrspace(200)* %x, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32 addrspace(200)* %x, align 4
  %call1 = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
  %3 = load i32, i32 addrspace(200)* %x, align 4
  ret i32 %3
}

declare i32 addrspace(200)* @call(i32 addrspace(200)*) addrspace(200) #1

; Function Attrs: noinline nounwind
define void @store_in_bounds_min() addrspace(200) #0 {
entry:
  %buffer = alloca [3 x i8], align 1, addrspace(200)
  %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
  store i8 0, i8 addrspace(200)* %arrayidx, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_in_bounds_max() addrspace(200) #0 {
entry:
  %buffer = alloca [3 x i8], align 1, addrspace(200)
  %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 2
  store i8 0, i8 addrspace(200)* %arrayidx, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_out_of_bounds_1() addrspace(200) #0 {
entry:
  %buffer = alloca [3 x i8], align 1, addrspace(200)
  %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 -1
  store i8 0, i8 addrspace(200)* %arrayidx, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_out_of_bounds_2() addrspace(200) #0 {
entry:
  %buffer = alloca [3 x i8], align 1, addrspace(200)
  %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 3
  store i8 0, i8 addrspace(200)* %arrayidx, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_out_of_bounds_3() addrspace(200) #0 {
entry:
  %buffer = alloca [3 x i8], align 1, addrspace(200)
  %arraydecay = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
  %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
  store i32 42, i32 addrspace(200)* %0, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_out_of_bounds_4() addrspace(200) #0 {
entry:
  %buffer = alloca [16 x i8], align 1, addrspace(200)
  %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -1
  %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
  store i32 42, i32 addrspace(200)* %0, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_in_bounds_cast() addrspace(200) #0 {
entry:
  %buffer = alloca [16 x i8], align 1, addrspace(200)
  %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
  %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
  store i32 42, i32 addrspace(200)* %0, align 1
  %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 12
  %1 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
  store i32 42, i32 addrspace(200)* %1, align 1
  ret void
}

; Function Attrs: noinline nounwind
define void @store_int_bounds_cast_tmp_out_of_bounds() addrspace(200) #0 {
entry:
  %buffer = alloca [16 x i8], align 1, addrspace(200)
  %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -4
  %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(200)* %0, i64 1
  store i32 42, i32 addrspace(200)* %arrayidx1, align 1
  ret void
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri256" "target-features"="+cheri256,+chericap,+soft-float,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri256" "target-features"="+cheri256,+chericap,+soft-float,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 1}
!2 = !{!"clang version 9.0.0 (https://github.com/CTSRD-CHERI/llvm-project 15aa4de05c4ef4e116df6c1a3a5ec06b6a23e088)"}
