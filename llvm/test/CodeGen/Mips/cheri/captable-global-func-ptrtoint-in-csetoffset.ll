; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel -o - -verify-machineinstrs %s | FileCheck %s
; Check that we don't accidentally create a nonsense COPY node ($at_64 = COPY killed renamable $c1)
; when the result of the GlobalAddr goes through a ptrtoint

%class.A = type { i32 (...) addrspace(200)* addrspace(200)* }

; Function Attrs: noinline nounwind
declare i32 @foo(%class.A addrspace(200)*) #0

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1

; Function Attrs: noinline norecurse nounwind
define void @main(i8 addrspace(200)* %arg) #2 {
entry:
  %0 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %arg, i64 ptrtoint (i32 (%class.A addrspace(200)*)* @foo to i64))
  %1 = bitcast i8 addrspace(200)* %0 to i32 (%class.A addrspace(200)*) addrspace(200)*
  %call3 = call i32 %1(%class.A addrspace(200)* undef)
  ; CHECK: clcbi	$c1, %capcall20(foo)($c26)
  ; FIXME: should we really generate a ctoptr here?
  ; CHECK: ctoptr	$1, $c1, $ddc
  ; CHECK: csetoffset	$c12, $c3, $1
  ; CHECK: cjalr	$c12, $c17
  ret void
}
