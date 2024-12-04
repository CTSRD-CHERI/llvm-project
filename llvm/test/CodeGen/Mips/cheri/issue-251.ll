; RUN: %cheri_purecap_llc %s -o - -disable-cheri-addressing-mode-folder | FileCheck %s -check-prefix CHECK-NOFOLD
; RUN: %cheri_purecap_llc %s -o - | FileCheck %s -check-prefix CHECK-NOFOLD

; Can't fold the immediate in csetoffset on NULL
; https://github.com/CTSRD-CHERI/llvm/issues/251

define void @func() noinline nounwind optnone {
entry:
  %testCap = alloca ptr addrspace(200), align 32, addrspace(200)
  %0 = call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) null, i64 4096)
  %1 = bitcast ptr addrspace(200) %0 to ptr addrspace(200)
  %2 = load volatile ptr addrspace(200), ptr addrspace(200) %1, align 32
  store ptr addrspace(200) %2, ptr addrspace(200) %testCap, align 32
  ret void
  ; CHECK-NOFOLD:      daddiu  $1, $zero, 4096
  ; CHECK-NOFOLD-NEXT: cincoffset [[CREG:\$c[0-9]+]], $cnull, $1
  ; CHECK-NOFOLD-NEXT: clc $c{{[0-9+]}}, $zero, 0([[CREG]])
  ; Note: this could be more efficient if we fold the constant into the clc.
}

declare ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200), i64)
