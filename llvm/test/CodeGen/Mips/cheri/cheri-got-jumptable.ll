; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel -mxcaptable -o - %s | FileCheck %s -check-prefix PCREL

; Function Attrs: noinline nounwind
define i32 @get_next_format_from_precision(i8 addrspace(200)* addrspace(200)* %pf) #0 {
entry:
  %retval = alloca i32, align 4, addrspace(200)
  %pf.addr = alloca i8 addrspace(200)* addrspace(200)*, align 32, addrspace(200)
  %f = alloca i8 addrspace(200)*, align 32, addrspace(200)
  store i8 addrspace(200)* addrspace(200)* %pf, i8 addrspace(200)* addrspace(200)* addrspace(200)* %pf.addr, align 32
  %0 = load i8 addrspace(200)* addrspace(200)*, i8 addrspace(200)* addrspace(200)* addrspace(200)* %pf.addr, align 32
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %0, align 32
  store i8 addrspace(200)* %1, i8 addrspace(200)* addrspace(200)* %f, align 32
  %2 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %f, align 32
  %3 = load i8, i8 addrspace(200)* %2, align 1
  %conv = sext i8 %3 to i32
  ; PCREL: %captab_hi(.LJTI0_0)
  ; PCREL: %captab_lo(.LJTI0_0)
  switch i32 %conv, label %sw.epilog [
    i32 104, label %sw.bb
    i32 113, label %sw.bb1
    i32 116, label %sw.bb1
    i32 122, label %sw.bb1
  ]

sw.bb:                                            ; preds = %entry
  store i32 0, i32 addrspace(200)* %retval, align 4
  br label %do.end

sw.bb1:                                           ; preds = %entry, %entry, %entry
  %4 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %f, align 32
  %incdec.ptr = getelementptr inbounds i8, i8 addrspace(200)* %4, i32 1
  store i8 addrspace(200)* %incdec.ptr, i8 addrspace(200)* addrspace(200)* %f, align 32
  br label %sw.epilog

sw.epilog:                                        ; preds = %entry, %sw.bb1
  br label %do.body

do.body:                                          ; preds = %sw.epilog
  %5 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %f, align 32
  %6 = load i8 addrspace(200)* addrspace(200)*, i8 addrspace(200)* addrspace(200)* addrspace(200)* %pf.addr, align 32
  store i8 addrspace(200)* %5, i8 addrspace(200)* addrspace(200)* %6, align 32
  store i32 0, i32 addrspace(200)* %retval, align 4
  br label %do.end

do.end:                                           ; preds = %sw.bb, %do.body
  %7 = load i32, i32 addrspace(200)* %retval, align 4
  ret i32 %7
}

attributes #0 = { noinline nounwind  }
