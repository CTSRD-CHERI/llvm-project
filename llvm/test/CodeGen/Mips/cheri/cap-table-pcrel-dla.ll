; RUN: not %cheri_purecap_llc -cheri-cap-table-abi=pcrel -o - -O0 %s 2>&1 | FileCheck %s
; This previously caused an assertion since in PIC mode the dla would expand to
; to a sequence using $gp but in cap-table code that is (currently) only allowed for TLS accesses

; C source code:
; long a;
; void b() { __asm__("dla %0, _DYNAMIC\n\t" : "=r"(a)); }

@a = common addrspace(200) global i64 0, align 8

; Function Attrs: noinline nounwind optnone
define void @b() #0 {
entry:
  %0 = call i64 asm "dla $0, _DYNAMIC\0A\09", "=r,~{$1}"() #1, !srcloc !2

  ; CHECK: <inline asm>:1:2: error: Can't expand $gp-relative in cap-table mode
  ; CHECK:       dla $2, _DYNAMIC
  store i64 %0, i64 addrspace(200)* @a, align 8
  ret void
}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (https://github.com/llvm-mirror/clang.git e83ef0dd2a36e229703cf8403d43e0c20ee029c0) (https://github.com/llvm-mirror/llvm.git 838c200be608b48865fbf4b018a2264f047a82ee)"}
!2 = !{i32 437, i32 456}
