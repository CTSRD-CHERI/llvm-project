; RUN: %cheri_purecap_llc %s -o -
; ModuleID = 'bit.c'
; Test that we can correctly legalise i128 and generate pointer arithmetic that
; doesn't crash the compiler.
source_filename = "bit.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.foo = type { i128 }

@x = internal addrspace(200) global %struct.foo zeroinitializer, align 4

; Function Attrs: noinline nounwind
define i32 @main(i32 signext %argc, i8 addrspace(200)* addrspace(200)* %argv) #0 {
entry:
  %retval = alloca i32, align 4, addrspace(200)
  %argc.addr = alloca i32, align 4, addrspace(200)
  %argv.addr = alloca i8 addrspace(200)* addrspace(200)*, align 32, addrspace(200)
  store i32 0, i32 addrspace(200)* %retval, align 4
  store i32 %argc, i32 addrspace(200)* %argc.addr, align 4
  store i8 addrspace(200)* addrspace(200)* %argv, i8 addrspace(200)* addrspace(200)* addrspace(200)* %argv.addr, align 32
  %bf.load = load i128, i128 addrspace(200)* getelementptr inbounds (%struct.foo, %struct.foo addrspace(200)* @x, i32 0, i32 0), align 4
  %bf.clear = and i128 %bf.load, -4294967295
  %bf.set = or i128 %bf.clear, 2
  store i128 %bf.set, i128 addrspace(200)* getelementptr inbounds (%struct.foo, %struct.foo addrspace(200)* @x, i32 0, i32 0), align 4
  ret i32 0
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang 36fb1dc82f2552b06fa268eab5513cf52c16f41b)"}
