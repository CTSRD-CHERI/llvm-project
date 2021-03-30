// REQUIRES: clang

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -DFNPTR1 -emit-obj -O2 %s -o %t-fnptr1.o
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -DFNPTR2 -emit-obj -O2 %s -o %t-fnptr2.o
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -DCALL1 -emit-obj -O2 %s -o %t-call1.o
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -DCALL2 -emit-obj -O2 %s -o %t-call2.o
// RUN: llvm-readobj -r %t-fnptr1.o | FileCheck --check-prefix OBJ-FNPTR1 %s
// OBJ-FNPTR1: Relocations [
// OBJ-FNPTR1-NEXT: Section ({{.+}}) .rela.text {
// OBJ-FNPTR1-NEXT:    0x0 R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE extern1 0x0
// OBJ-FNPTR1-NEXT:  }
// RUN: llvm-readobj -r %t-fnptr2.o | FileCheck --check-prefix OBJ-FNPTR2 %s
// OBJ-FNPTR2: Relocations [
// OBJ-FNPTR2-NEXT: Section ({{.+}}) .rela.data {
// OBJ-FNPTR2-NEXT:    0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE extern2 0x0
// OBJ-FNPTR2-NEXT:  }
// RUN: llvm-readobj -r %t-call1.o | FileCheck --check-prefix OBJ-CALL1 %s
// OBJ-CALL1: Relocations [
// OBJ-CALL1-NEXT: Section ({{.+}}) .rela.text {
// OBJ-CALL1-NEXT:    0x10 R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE extern1 0x0
// OBJ-CALL1-NEXT:  }
// RUN: llvm-readobj -r %t-call2.o| FileCheck --check-prefix OBJ-CALL2 %s
// OBJ-CALL2: Relocations [
// OBJ-CALL2-NEXT: Section ({{.+}}) .rela.text {
// OBJ-CALL2-NEXT:    0x10 R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE extern2 0x0
// OBJ-CALL2-NEXT:  }

// Check the resulting output relocations for a single fnptr/call
// RUN: ld.lld -shared -o %t-fnptr1.so %t-fnptr1.o
// RUN: llvm-readobj -r %t-fnptr1.so | FileCheck --check-prefix ONE-FNPTR %s
// ONE-FNPTR: Relocations [
// ONE-FNPTR-NEXT: Section ({{.+}}) .rel.dyn {
// ONE-FNPTR-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE extern1{{$}}
// ONE-FNPTR-NEXT:  }

// If the symbol is only ever called we can emit a R_MIPS_CHERI_CAPABILITY_CALL which will allow the
// runtime linker to lazily resolve the symbol since the address is not taken. If the address is taken,
// all instances of this function pointer must compare equal so they must point to a unique plt stub!
// RUN: ld.lld -shared -o %t-call1.so %t-call1.o
// RUN: llvm-readobj --dynamic-table -r %t-call1.so | FileCheck --check-prefix ONE-CALL %s
// ONE-CALL-LABEL: DynamicSection [
// ONE-CALL-NEXT: Tag                Type                 Name/Value
// ONE-CALL-NEXT: 0x0000000000000017 JMPREL               0x3F8
// ONE-CALL-NEXT: 0x0000000000000002 PLTRELSZ             16 (bytes)
// ONE-CALL-NEXT: 0x0000000070000032 MIPS_PLTGOT          0x0
// ONE-CALL-NEXT: 0x0000000000000014 PLTREL               REL
// ONE-CALL-LABEL: Relocations [
// ONE-CALL-NEXT: Section ({{.+}}) .rel.plt {
// ONE-CALL-NEXT:    R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern1{{$}}
// ONE-CALL-NEXT:  }

// RUN: ld.lld -shared -o %t-2calls.so %t-call1.o %t-call2.o
// RUN: llvm-readobj --dynamic-table -r %t-2calls.so | FileCheck --check-prefix TWO-CALLS %s
// TWO-CALLS-LABEL: DynamicSection [
// TWO-CALLS-NEXT: Tag                Type                 Name/Value
// TWO-CALLS-NEXT: 0x0000000000000017 JMPREL               0x448
// TWO-CALLS-NEXT: 0x0000000000000002 PLTRELSZ             32 (bytes)
// TWO-CALLS-NEXT: 0x0000000070000032 MIPS_PLTGOT          0x0
// TWO-CALLS-NEXT: 0x0000000000000014 PLTREL               REL
// TWO-CALLS-LABEL: Relocations [
// TWO-CALLS-NEXT: Section ({{.+}}) .rel.plt {
// TWO-CALLS-NEXT:    R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern1{{$}}
// TWO-CALLS-NEXT:    R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern2{{$}}
// TWO-CALLS-NEXT:  }



// If the same symbol is both called and used as a function pointer we must use R_MIPS_CHERI_CAPABILITY
// and not R_MIPS_CHERI_CAPABILITY_CALL since currently there is no way to lazily resolve function
// pointers (they must always point to a plt stub). This is necessary since they could be invoked
// from another context with different globals (e.g. libc qsort(), kernel signal()) and must therefore
// load the correct globals.
// Note: for function call only symbols we can have a different stub for each call, but for function pointers
// we must only ever hand out a unique pointer value to ensure equality comparision works as required by the C standard.
// RUN: ld.lld -shared -o %t-call-and-fnptr.so %t-call1.o %t-fnptr1.o
// RUN: llvm-readobj -r %t-call-and-fnptr.so | FileCheck --check-prefix CALL-AND-FNPTR %s
// CALL-AND-FNPTR: Relocations [
// CALL-AND-FNPTR-NEXT: Section ({{.+}}) .rel.dyn {
// CALL-AND-FNPTR-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE extern1{{$}}
// CALL-AND-FNPTR-NEXT:  }

// RUN: ld.lld -shared -o %t-all.so %t-call1.o %t-fnptr1.o %t-call2.o %t-fnptr2.o
// RUN: llvm-readobj --dynamic-table -r %t-all.so  | FileCheck --check-prefix ALL %s
// ALL-LABEL: DynamicSection [
// ALL-NEXT:  Tag                Type                 Name/Value
// ALL-NEXT:  0x0000000000000011 REL                  0x4D0
// ALL-NEXT:  0x0000000000000012 RELSZ                32 (bytes)
// ALL-NEXT:  0x0000000000000013 RELENT               16 (bytes)
// ALL-NEXT:  0x0000000000000017 JMPREL               0x4F0
// ALL-NEXT:  0x0000000000000002 PLTRELSZ             16 (bytes)
// ALL-NEXT:  0x0000000070000032 MIPS_PLTGOT          0x0
// ALL-NEXT:  0x0000000000000014 PLTREL               REL
// ALL: Relocations [
// ALL-NEXT: Section ({{.+}}) .rel.dyn {
// ALL-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE extern1{{$}}
// Since this is a data symbol we have to eagerly resolve it to a stub
// ALL-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE extern2{{$}}
// ALL-NEXT:  }
// FIXME: we should to avoid another PLT relocation here since we are already resolving the symbol!
// However, it is a data symbol and not just a local value loaded from the captable so it will require a bit more logic.
// ALL-NEXT:  Section ({{.+}}) .rel.plt {
// ALL-NEXT:    R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern2{{$}}
// ALL-NEXT:  }

extern void* extern1(void);
extern void* extern2(void);

#ifdef FNPTR1
void* return_fnptr() {
  return &extern1;
}
#endif

#ifdef FNPTR2
void* (*global_fnptr)(void) = &extern2;
#endif

#ifdef CALL1
void* call1() {
  return extern1();
}
#endif

#ifdef CALL2
void* call2() {
  return extern2();
}
#endif
