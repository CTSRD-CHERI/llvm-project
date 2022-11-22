; RUN: %cheri128_llc -O2 -o - -relocation-model=pic %s | FileCheck %s

@array = common global [2 x i8 addrspace(200)*] zeroinitializer, align 16

; Function Attrs: nounwind readonly
define i8 addrspace(200)* @get(i32 signext %x) #0 {
entry:
  %idxprom = sext i32 %x to i64
  ; Check that we convert the index to an offset by multiplying by 16:
  ; CHECK: dsll	$2, $4, 4
  %arrayidx = getelementptr inbounds [2 x i8 addrspace(200)*], [2 x i8 addrspace(200)*]* @array, i64 0, i64 %idxprom
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %arrayidx, align 16
  ret i8 addrspace(200)* %0
}
