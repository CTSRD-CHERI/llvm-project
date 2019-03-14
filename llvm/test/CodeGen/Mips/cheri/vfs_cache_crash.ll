; RUN: %cheri_purecap_llc -O2 %s -o /dev/null

; From this reduced C source code (and then reduced the IR):
; static int __attribute__((__section__(".data.read_mostly"))) a;
; int b() {
;   if (b)
;     return 0;
; c:
;   d();
;   a++;
;   goto c;
; }

; ModuleID = '/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/CHERI/vfs_cache-crash.c'
source_filename = "/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/CHERI/vfs_cache-crash.c"

@a = internal unnamed_addr addrspace(200) global i32 0, section ".data.read_mostly", align 4

; Function Attrs: nounwind
define void @b() #0 {
entry:
  br i1 undef, label %c, label %if.then

if.then:                                          ; preds = %entry
  ret void

c:                                                ; preds = %c, %entry
  %0 = load i32, i32 addrspace(200)* @a, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32 addrspace(200)* @a, align 4
  br label %c
}

attributes #0 = { nounwind }
