// RUN: %cheri_purecap_cc1 %legacy_caprelocs_flag %s -emit-obj -o %t.o
// RUN: ld.lld -process-cap-relocs %t.o -static -o %t.exe --fatal-warnings
// RUN: llvm-objdump -h -r -t -C %t.exe | FileCheck -check-prefix WITH-CTORS %s

// RUN: %cheri_purecap_cc1 %legacy_caprelocs_flag %s -DEMPTY_CTORS=1 -emit-obj -o %t-with-ctors.o
// RUN: ld.lld -process-cap-relocs %t-with-ctors.o -static -o %t-with-ctors.exe --fatal-warnings
// RUN: llvm-objdump -h -r -t -C %t-with-ctors.exe | FileCheck -check-prefix EMPTY-CTORS %s

typedef unsigned long long mips_function_ptr;
typedef void (*cheri_function_ptr)(void);

extern mips_function_ptr __ctors_start[];
extern mips_function_ptr __ctors_end;

extern mips_function_ptr __dtors_start[];
extern mips_function_ptr __dtors_end;

extern void *__dso_handle;
void *__dso_handle;

#ifndef EMPTY_CTORS
__attribute__((section(".ctors"))) mips_function_ptr dummy_ctor = -1;
#endif

void crt_call_constructors(void) {
  mips_function_ptr *func = &__ctors_start[0];
  mips_function_ptr *end =
      __builtin_cheri_offset_set(func, (char *)&__ctors_end - (char *)func);
  for (; func != end; func++) {
    if (*func != (mips_function_ptr)-1) {
      cheri_function_ptr cheri_func =
          (cheri_function_ptr)__builtin_cheri_offset_set(
              __builtin_cheri_program_counter_get(), *func);
      cheri_func();
    }
  }
}

void __start(void) {
  crt_call_constructors();
}

// WITH-CTORS-LABEL: Sections:
// WITH-CTORS:       .ctors        00000008 0000000120030000 DATA
// WITH-CTORS-LABEL: SYMBOL TABLE:
// WITH-CTORS-NEXT:  0000000000000000         *UND*		 00000000
// WITH-CTORS-NEXT:  0000000000000000 l df    *ABS* 00000000 load-linker-created-symbol.c
// WITH-CTORS-NEXT:  0000000120030008         .ctors		 00000000 .hidden __ctors_end
// WITH-CTORS-NEXT:  0000000120030000         .ctors		 00000008 .hidden __ctors_start

// EMPTY-CTORS-LABEL: Section
// EMPTY-CTORS-NOT: .ctors
// EMPTY-CTORS-LABEL: SYMBOL TABLE:
// EMPTY-CTORS-NEXT:  0000000000000000         *UND*		 00000000
// EMPTY-CTORS-NEXT:  0000000000000000 l df    *ABS* 00000000
// load-linker-created-symbol.c EMPTY-CTORS-NEXT:  0000000120010000 .text
// 00000000 .hidden __ctors_end EMPTY-CTORS-NEXT:  0000000120010000 .text
// 00000000 .hidden __ctors_start
