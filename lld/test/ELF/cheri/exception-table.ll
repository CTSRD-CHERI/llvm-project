; RUN: %cheri_purecap_llc --relocation-model=pic < %s -filetype=obj -o %t.o
; RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=MIPS-OBJ-RELOCS
; RUN: %riscv64_cheri_purecap_llc --relocation-model=pic < %s -filetype=obj -o %t-riscv64.o
; RUN: llvm-readobj -r %t-riscv64.o | FileCheck %s --check-prefix=RV64-OBJ-RELOCS
;; Should have two relocations against a local alias for _Z4testll
; MIPS-OBJ-RELOCS:      Section ({{.+}}) .rela.gcc_except_table {
; MIPS-OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L_Z4testll$local 0x80
; MIPS-OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L_Z4testll$local 0x60
; MIPS-OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L_Z5test2ll$local 0x60
; MIPS-OBJ-RELOCS-NEXT:   R_MIPS_PC32/R_MIPS_NONE/R_MIPS_NONE .L_ZTIl.DW.stub 0x0
; MIPS-OBJ-RELOCS-NEXT: }
; RV64-OBJ-RELOCS:      Section ({{.+}}) .rela.gcc_except_table {
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_CHERI_CAPABILITY .L_Z4testll$local 0x5C
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_CHERI_CAPABILITY .L_Z4testll$local 0x48
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_CHERI_CAPABILITY .L_Z5test2ll$local 0x48
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
; RV64-OBJ-RELOCS-NEXT:   R_RISCV_32_PCREL .L_ZTIl.DW.stub 0x0
; RV64-OBJ-RELOCS-NEXT: }

;; This should work with both -z text and -z notext
;; Check that .gcc_except_table ends up in the relro section and the relocations are correct
; RUN: ld.lld -shared %t.o -o %t.so -z notext
; RUN: llvm-readelf -r --section-mapping --sections --program-headers --cap-relocs  %t.so | FileCheck %s --check-prefixes=HEADERS,MIPS-RELOCS
; RUN: ld.lld -shared %t.o -o %t.so -z text
; RUN: llvm-readelf -r --section-mapping %t.so
; RUN: llvm-readelf -r --section-mapping --sections --program-headers --cap-relocs  %t.so | FileCheck %s --check-prefixes=HEADERS,MIPS-RELOCS

; HEADERS-LABEL: There are 10 program headers, starting at
; HEADERS-EMPTY:
; HEADERS-NEXT: Program Headers:
; HEADERS-NEXT: Type           Offset   VirtAddr PhysAddr FileSiz  MemSiz   Flg Align
; HEADERS-NEXT: PHDR           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
; HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x10000
; HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R E 0x10000
; HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x10000
; HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x10000
; HEADERS-NEXT: DYNAMIC        0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
; HEADERS-NEXT: GNU_RELRO      0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x1
; HEADERS-NEXT: GNU_STACK      0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x0
; HEADERS-NEXT: OPTIONS        0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
; HEADERS-NEXT: ABIFLAGS       0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
; HEADERS-EMPTY:
; HEADERS-NEXT: Section to Segment mapping:
; HEADERS-NEXT: Segment Sections...
; HEADERS-NEXT: 00
; HEADERS-NEXT: 01     .MIPS.abiflags .MIPS.options .dynsym .hash .dynamic .dynstr .rel.dyn .rel.plt .eh_frame __cap_relocs {{$}}
; HEADERS-NEXT: 02     .text
; HEADERS-NEXT: 03     .gcc_except_table
; HEADERS-NEXT: 04     .data .captable .got
; HEADERS-NEXT: 05     .dynamic
; HEADERS-NEXT: 06     .gcc_except_table
; HEADERS-NEXT: 07
; HEADERS-NEXT: 08     .MIPS.options
; HEADERS-NEXT: 09     .MIPS.abiflags
; HEADERS-NEXT: None   .bss .mdebug.abi64 .pdr .comment .symtab .shstrtab .strtab

; MIPS-RELOCS-LABEL:      Relocation section '.rel.dyn' {{.+}} contains 2 entries:
; MIPS-RELOCS-NEXT: Offset             Info             Type                            Symbol's Value  Symbol's Name
; MIPS-RELOCS-NEXT:                     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 _ZTIl
; MIPS-RELOCS-NEXT:                     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __gxx_personality_v0
; MIPS-RELOCS-EMPTY:
; MIPS-RELOCS-NEXT: Relocation section '.rel.plt' at offset {{.+}} contains 3 entries:
; MIPS-RELOCS-NEXT: Offset             Info             Type                            Symbol's Value   Symbol's Name
; MIPS-RELOCS-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 _Z11external_fnl
; MIPS-RELOCS-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __cxa_begin_catch
; MIPS-RELOCS-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __cxa_end_catch

;; Local relocations for exception handling:
; MIPS-RELOCS-NEXT: CHERI __cap_relocs [
; MIPS-RELOCS-NEXT:   0x02{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+128) Length: 160 Perms: Function
; MIPS-RELOCS-NEXT:   0x02{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+96) Length: 160 Perms: Function
; MIPS-RELOCS-NEXT:   0x02{{.+}} Base: 0x1{{.+}} (.L_Z5test2ll$local+96) Length: 168 Perms: Function
; MIPS-RELOCS-NEXT: ]

;; Should also emit __cap_relocs for RISC-V:
; RUN: ld.lld -shared %t-riscv64.o -o %t.so
; RUN: llvm-readelf -r --cap-relocs %t.so | FileCheck %s --check-prefixes=RV64-RELOCS
; RV64-RELOCS: CHERI __cap_relocs [
; RV64-RELOCS-NEXT:   0x002{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+92) Length: 116 Perms: Function
; RV64-RELOCS-NEXT:   0x002{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+72) Length: 116 Perms: Function
; RV64-RELOCS-NEXT:   0x002{{.+}} Base: 0x1{{.+}} (.L_Z5test2ll$local+72) Length: 124 Perms: Function
; RV64-RELOCS-NEXT: ]

; IR was generated from the following code:
; long external_fn(long arg);
;
; long test(long arg, long arg2) {
;   long a = 0;
;   long b = 0;
;   try {
;     a = external_fn(arg);
;   } catch (...) {
;     return -1;
;   }
;   try {
;     b = external_fn(arg2);
;   } catch (...) {
;     return a;
;   }
;   return a + b;
; }
;
; long test2(long arg, long arg2) {
;   try {
;     return external_fn(arg) - external_fn(arg2);
;   } catch (long &err) {
;     return err;
;   } catch (...) {
;     return -1;
;   }
; }

@_ZTIl = external dso_local addrspace(200) constant ptr addrspace(200)

define dso_local noundef i64 @_Z4testll(i64 noundef %arg, i64 noundef %arg2) local_unnamed_addr addrspace(200) uwtable personality ptr addrspace(200) @__gxx_personality_v0 {
entry:
  %call = invoke noundef i64 @_Z11external_fnl(i64 noundef %arg)
          to label %invoke.cont unwind label %lpad

invoke.cont:
  %call3 = invoke noundef i64 @_Z11external_fnl(i64 noundef %arg2)
          to label %invoke.cont2 unwind label %lpad1

lpad:
  %0 = landingpad { ptr addrspace(200), i32 }
          catch ptr addrspace(200) null
  %1 = extractvalue { ptr addrspace(200), i32 } %0, 0
  %2 = tail call ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200) %1) nounwind
  tail call void @__cxa_end_catch()
  br label %cleanup

invoke.cont2:
  %add = add nsw i64 %call3, %call
  br label %cleanup

lpad1:
  %3 = landingpad { ptr addrspace(200), i32 }
          catch ptr addrspace(200) null
  %4 = extractvalue { ptr addrspace(200), i32 } %3, 0
  %5 = tail call ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200) %4) nounwind
  tail call void @__cxa_end_catch()
  br label %cleanup

cleanup:
  %retval.0 = phi i64 [ %add, %invoke.cont2 ], [ %call, %lpad1 ], [ -1, %lpad ]
  ret i64 %retval.0
}

declare dso_local noundef i64 @_Z11external_fnl(i64 noundef) local_unnamed_addr addrspace(200)

declare dso_local i32 @__gxx_personality_v0(...) addrspace(200)

declare dso_local ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200)) local_unnamed_addr addrspace(200)

declare dso_local void @__cxa_end_catch() local_unnamed_addr addrspace(200)

; Function Attrs: mustprogress uwtable
define dso_local noundef i64 @_Z5test2ll(i64 noundef %arg, i64 noundef %arg2) local_unnamed_addr addrspace(200) #0 personality ptr addrspace(200) @__gxx_personality_v0 {
entry:
  %call = invoke noundef i64 @_Z11external_fnl(i64 noundef %arg)
          to label %invoke.cont unwind label %lpad

invoke.cont:
  %call2 = invoke noundef i64 @_Z11external_fnl(i64 noundef %arg2)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:
  %sub = sub nsw i64 %call, %call2
  br label %return

lpad:
  %0 = landingpad { ptr addrspace(200), i32 }
          catch ptr addrspace(200) @_ZTIl
          catch ptr addrspace(200) null
  %1 = extractvalue { ptr addrspace(200), i32 } %0, 0
  %2 = extractvalue { ptr addrspace(200), i32 } %0, 1
  %3 = tail call i32 @llvm.eh.typeid.for(ptr addrspacecast (ptr addrspace(200) @_ZTIl to ptr)) nounwind
  %matches = icmp eq i32 %2, %3
  %4 = tail call ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200) %1) nounwind
  br i1 %matches, label %catch3, label %catch

catch3:
  %5 = load i64, ptr addrspace(200) %4, align 8
  tail call void @__cxa_end_catch() nounwind
  br label %return

catch:
  tail call void @__cxa_end_catch()
  br label %return

return:
  %retval.0 = phi i64 [ %sub, %invoke.cont1 ], [ %5, %catch3 ], [ -1, %catch ]
  ret i64 %retval.0
}

declare i32 @llvm.eh.typeid.for(ptr) addrspace(200)
