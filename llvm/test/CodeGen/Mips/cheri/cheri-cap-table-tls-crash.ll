; RUN: llc -o /dev/null -mtriple=cheri-unknown-rtems5 -relocation-model=pic -thread-model=posix -mattr=-noabicalls -mattr=+chericap -mattr=+cheri128 -target-abi purecap -float-abi=soft -mcpu=cheri128 -O0 -cheri-cap-table-abi=pcrel -verify-machineinstrs %s -print-after-all -debug
; ModuleID = '***/build/init-a31849-bugpoint-reduce.ll-reduced-simplified.bc'
; See https://github.com/CTSRD-CHERI/llvm/issues/275
; This needs real TLS support
; XFAIL: *
source_filename = "/tmp/init-a31849.c"
target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
target triple = "cheri-unknown-rtems5"

@tls_item = external dso_local thread_local(initialexec) global i8, align 1
@.str.6 = external dso_local unnamed_addr addrspace(200) constant [15 x i8], align 1

declare void @printf(i8 addrspace(200)*, ...) addrspace(200) #0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @check_tls_item() addrspace(200) #1 {
entry:
  call void (i8 addrspace(200)*, ...) @printf(i8 addrspace(200)* getelementptr inbounds ([15 x i8], [15 x i8] addrspace(200)* @.str.6, i32 0, i32 0), i32 signext undef)
  %0 = load volatile i8, i8 addrspace(200)* addrspacecast (i8* @tls_item to i8 addrspace(200)*), align 1
  unreachable
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap,+soft-float,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap,+soft-float,-noabi
calls" "unsafe-fp-math"="false" "use-soft-float"="true" }
