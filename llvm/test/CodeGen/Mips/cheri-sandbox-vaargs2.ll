; RUN: clang -cc1 -triple cheri-unknown-bsd -cheri-linker -target-abi sandbox -O2 -S -o - %s | FileCheck %s
; ModuleID = 'libxo.i'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-bsd"

%struct.xo_handle_s = type { i8 addrspace(200)* }

@b = common addrspace(200) global %struct.xo_handle_s zeroinitializer, align 32

; Function Attrs: nounwind
; CHECK-LABEL: xo_emit:
; Check that locally creating a va_list and then storing it to a global works
; (Yes, this is an odd thing to do.  See libxo for a real-world example)
; This is similar to cheri-sandbox-vaargs.ll, but ensures CheriSandboxABI can
; handle the optimiser turning AddrSpaceCast instructions into ConstantExpr's
define void @xo_emit(i8 addrspace(200)* %fmt, ...) #0 {
entry:
  %fmt.addr = alloca i8 addrspace(200)*, align 32
  %c = alloca %struct.xo_handle_s addrspace(200)*, align 32
  store i8 addrspace(200)* %fmt, i8 addrspace(200)* addrspace(200)* %fmt.addr, align 32
  store %struct.xo_handle_s addrspace(200)* @b, %struct.xo_handle_s addrspace(200)* addrspace(200)* %c, align 32
  %0 = load %struct.xo_handle_s addrspace(200)*, %struct.xo_handle_s addrspace(200)* addrspace(200)* %c, align 32
  %xo_vap = getelementptr inbounds %struct.xo_handle_s, %struct.xo_handle_s addrspace(200)* %0, i32 0, i32 0
  %xo_vap1 = addrspacecast i8 addrspace(200)* addrspace(200)* %xo_vap to i8*
  ; Load the address of b
  ; CHECK: ld	$1, %got_disp(b)($1)
  ; CHECK: cfromptr $c1, $c0, $1
  ; Store the va_list (passed in $c13) in the global
  ; CHECK: csc	$c13, $zero, 0($c1)
  call void @llvm.va_start(i8* %xo_vap1)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.va_start(i8*) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (git@github.com:CTSRD-CHERI/clang 05f124e1657de7fd881aec2ad65f265f657b1edb) (git@github.com:CTSRD-CHERI/llvm daf436f19e3bee85398590f0e29ed5981de61c14)"}
