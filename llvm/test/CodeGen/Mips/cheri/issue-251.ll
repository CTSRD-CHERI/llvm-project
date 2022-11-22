; RUN: %cheri_purecap_llc %s -o - -disable-cheri-addressing-mode-folder | FileCheck %s -check-prefix CHECK-NOFOLD
; RUN: %cheri_purecap_llc %s -o - | FileCheck %s -check-prefix CHECK-NOFOLD

; Can't fold the immediate in csetoffset on NULL
; https://github.com/CTSRD-CHERI/llvm/issues/251

; Function Attrs: noinline nounwind optnone
define void @func() #0 {
entry:
  %testCap = alloca i8 addrspace(200)*, align 32, addrspace(200)
  %0 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* null, i64 4096)
  %1 = bitcast i8 addrspace(200)* %0 to i8 addrspace(200)* addrspace(200)*
  %2 = load volatile i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %1, align 32
  store i8 addrspace(200)* %2, i8 addrspace(200)* addrspace(200)* %testCap, align 32
  ret void
  ; CHECK-NOFOLD:      daddiu  $1, $zero, 4096
  ; CHECK-NOFOLD-NEXT: cincoffset [[CREG:\$c[0-9]+]], $cnull, $1
  ; CHECK-NOFOLD-NEXT: clc $c{{[0-9+]}}, $zero, 0([[CREG]])
  ; Note: this could be more efficient if we fold the constant into the clc.
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64) #1

attributes #0 = { noinline nounwind optnone }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git c500a9dcaeb85031f3ccd8df1fa91a4127971645) (https://github.com/llvm-mirror/llvm.git 4e9420d4c2f41eba87860fc2da7cca4d16d57b82)"}
