// RUN: %cheri128_purecap_cc1 -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUN: %cheri128_purecap_cc1 -emit-obj -DCRTEND -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t-crtend.o
// RUN: ld.lld -o %t.exe %t.o %t-crtend.o
// RUN: llvm-objdump -d -r -cap-relocs -t %t.exe | FileCheck %s

// Check that we add a symbol for __ctors_start and size it correctly

typedef __UINT64_TYPE__ mips_function_ptr;
typedef void (*cheri_function_ptr)(void);
typedef __attribute((memory_address)) __UINT64_TYPE__ vaddr_t;


#ifdef CRTEND
mips_function_ptr __attribute__((used))
    __attribute__((section(".ctors")))
    __CTOR_END__[2] = { (mips_function_ptr)(-1), (mips_function_ptr)(-1) };

mips_function_ptr __attribute__((used))
    __attribute__((section(".dtors")))
    __DTOR_END__[1] = { (mips_function_ptr)(-1) };

#else

extern mips_function_ptr __ctors_start[];
extern mips_function_ptr __ctors_end;
extern mips_function_ptr __dtors_start[];
extern mips_function_ptr __dtors_end;

void
crt_call_constructors(void) {
  mips_function_ptr *func = &__ctors_start[0];
  mips_function_ptr *end = __builtin_cheri_offset_set(func,
      (char*)&__ctors_end - (char*)func);
  for (;func != end; func++) {
    if (*func != (mips_function_ptr) -1) {
      cheri_function_ptr cheri_func =
        (cheri_function_ptr) __builtin_cheri_offset_set(
          __builtin_cheri_program_counter_get(), *func);
      cheri_func();
    }
  }
}
#endif
// CHECK-LABEL: SYMBOL TABLE:
// CHECK: 0000000120020380         .ctors		 00000000 .hidden __ctors_end
// CHECK: 0000000120020370         .ctors		 00000010 .hidden __ctors_start
// CHECK: 0000000120020370 g     O .ctors		 00000010 __CTOR_END__

