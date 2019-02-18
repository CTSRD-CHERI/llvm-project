; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - | FileCheck %s -check-prefix PCREL
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=legacy %s -o - | FileCheck %s -check-prefix LEGACY

@x = common addrspace(200) global i32 0, align 4

; Function Attrs: nounwind
define void @foo(i32 signext %y) nounwind {
entry:
  ; LEGACY: 	ld	$1, %got_disp(x)($1)
  ; PCREL: 	clcbi $c1, %captab20(x)($c1)
  store i32 %y, i32 addrspace(200)* @x, align 4
  ret void
}
