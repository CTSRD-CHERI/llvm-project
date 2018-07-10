; RUN: %cheri128_purecap_llc -cheri-cap-table-abi=pcrel -O2 -o - %s | FileCheck %s -check-prefixes CHECK,PCREL
; RUN: %cheri128_purecap_llc -cheri-cap-table-abi=legacy -O2 -o - %s | FileCheck %s -check-prefixes CHECK,LEGACY
; ModuleID = 'libxo.i'

%struct.xo_handle_s = type { i8 addrspace(200)* }

@b = common addrspace(200) global %struct.xo_handle_s zeroinitializer, align 32

; Function Attrs: nounwind
; Check that locally creating a va_list and then storing it to a global works
; (Yes, this is an odd thing to do.  See libxo for a real-world example)
; This is similar to cheri-sandbox-vaargs.ll, but ensures CheriPureCapABI can
; handle the optimiser turning AddrSpaceCast instructions into ConstantExpr's
define void @xo_emit(i8 addrspace(200)* %fmt, ...) {
; CHECK-LABEL: xo_emit:
; CHECK: # %bb.0: # %entry
; LEGACY:  ld [[SIZE_OF_B:\$[0-9]+]], %got_disp(.size.b)($1)
; Save the incoming varargs in a temporary register:
; CHECK:   cmove [[VARARGS_CAP:\$c[0-9]+]],  $c13
; PCREL:   clcbi [[CAP_FOR_B:\$c[0-9]+]], %captab20(b)($c26)
; LEGACY:  ld [[ADDR_OF_B:\$[0-9]+]], %got_disp(b)($1)
; LEGACY:  cfromddc	[[TMP:\$c[0-9]+]], [[ADDR_OF_B]]
; LEGACY:  csetbounds	[[CAP_FOR_B:\$c[0-9]+]], [[TMP]], [[SIZE_OF_B]]
; Now store in the global:
; CHECK: csc	[[VARARGS_CAP]], $zero, 0([[CAP_FOR_B]])

entry:
  %fmt.addr = alloca i8 addrspace(200)*, align 32, addrspace(200)
  %c = alloca %struct.xo_handle_s addrspace(200)*, align 32, addrspace(200)
  store i8 addrspace(200)* %fmt, i8 addrspace(200)* addrspace(200)* %fmt.addr, align 32
  store %struct.xo_handle_s addrspace(200)* @b, %struct.xo_handle_s addrspace(200)* addrspace(200)* %c, align 32
  %0 = load %struct.xo_handle_s addrspace(200)*, %struct.xo_handle_s addrspace(200)* addrspace(200)* %c, align 32
  %xo_vap = getelementptr inbounds %struct.xo_handle_s, %struct.xo_handle_s addrspace(200)* %0, i32 0, i32 0
  %xo_vap1 = bitcast i8 addrspace(200)* addrspace(200)* %xo_vap to i8 addrspace(200)*
  ; Load the address of b
  ; Store the va_list (passed in $c13) in the global
  call void @llvm.va_start.p200i8(i8 addrspace(200)* %xo_vap1)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.va_start.p200i8(i8 addrspace(200)*) #0

attributes #0 = { nounwind }

