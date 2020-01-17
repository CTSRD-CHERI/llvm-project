// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crt1.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crt1.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crtbeginC.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crtbegin.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crtendC.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crtend.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %s -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-main.o
// RUN: ld.lld --fatal-warnings -o %t.exe %t-crt1.o %t-crtbegin.o %t-main.o %t-crtend.o -verbose
// RUN: llvm-readobj -r --cap-relocs %t.exe | FileCheck %s -check-prefix WITHOUT-CTORS
// RUN: llvm-objdump -t %t.exe | FileCheck %s -check-prefix WITHOUT-CTORS-SYMS
// Check that we also don't get any warnings if there are any init_array entries
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %s -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -DINIT_ARRAY_ENTRIES -o %t-main-with-ctors.o
// RUN: ld.lld --fatal-warnings -o %t-ctors.exe %t-crt1.o %t-crtbegin.o %t-main-with-ctors.o %t-crtend.o -verbose
// RUN: llvm-readobj -r --cap-relocs %t-ctors.exe | FileCheck %s -check-prefix WITH-CTORS
// RUN: llvm-objdump -t %t-ctors.exe | FileCheck %s -check-prefix WITH-CTORS-SYMS

#ifdef INIT_ARRAY_ENTRIES
#define CTOR_ATTR __attribute__((constructor))
#define DTOR_ATTR __attribute__((destructor))
#else
#define CTOR_ATTR
#define DTOR_ATTR
#endif

int CTOR_ATTR ctor_fn(void) { return 1; }
int DTOR_ATTR dtor_fn(void) { return 2; }

int main(void) {
  return 0;
}

int atexit(void (*function)(void)) {
  return 1;
}
void _init_tls(void) {}
__attribute__((noreturn)) void exit(int code) {
  __builtin_trap();
}

// WITHOUT-CTORS-LABEL: CHERI __cap_relocs [
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__auxargs@CAPTABLE) Base: 0x12003{{.+}} (__auxargs+0) Length: 16 Perms: Object
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (environ@CAPTABLE) Base: 0x12003{{.+}} (environ+0) Length: 16 Perms: Object
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (_DYNAMIC@CAPTABLE) Base: 0x0 (<unknown symbol>+0) Length: 0 Perms: Object
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (atexit@CAPTABLE) Base: 0x12001{{.+}} (atexit+0) Length: 8 Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (_init_tls@CAPTABLE) Base: 0x12001{{.+}} (_init_tls+0) Length: 8 Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (crt_call_constructors@CAPTABLE) Base: 0x12001{{.+}} (crt_call_constructors+0) Length: {{.+}} Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (handle_static_init@CAPTABLE.6) Base: 0x12001{{.+}} (handle_static_init+0) Length: {{.+}} Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (main@CAPTABLE) Base: 0x12001{{.+}} (main+0) Length: 8 Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (exit@CAPTABLE) Base: 0x12001{{.+}} (exit+0) Length: 4 Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__progname@CAPTABLE) Base: 0x12003{{.+}} (__progname+0) Length: 16 Perms: Object
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (finalizer@CAPTABLE.10) Base: 0x12001{{.+}} (finalizer+0) Length: 196 Perms: Function
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__preinit_array_start@CAPTABLE) Base: 0x120010670 (__fini_array_end+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__preinit_array_end@CAPTABLE) Base: 0x120010670 (__fini_array_end+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__init_array_start@CAPTABLE) Base: 0x120010670 (__fini_array_end+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__init_array_end@CAPTABLE) Base: 0x120010670 (__fini_array_end+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__fini_array_start@CAPTABLE) Base: 0x120010670 (__fini_array_end+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__fini_array_end@CAPTABLE) Base: 0x120010670 (__fini_array_end+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__CTOR_LIST__@CAPTABLE.17) Base: 0x120020d80 (__CTOR_LIST__+0) Length: 8 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__CTOR_END__@CAPTABLE) Base: 0x120020d88 (__CTOR_END__+0) Length: 8 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__start___cap_relocs@CAPTABLE) Base: 0x1200002d0 (__start___cap_relocs+0) Length: 920 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (__stop___cap_relocs@CAPTABLE) Base: 0x120000668 (__stop___cap_relocs+0) Length: 0 Perms: Constant
// WITHOUT-CTORS-NEXT:    0x12002{{.+}} (_int@CAPTABLE) Base: 0x12003{{.+}} (_int+0) Length: 4 Perms: Object
// WITHOUT-CTORS-NEXT:    0x12003{{.+}} (__progname)    Base: 0x1200002c8 (<unknown symbol>+0) Length: 1 Perms: Constant
// WITHOUT-CTORS-NEXT: ]

// WITH-CTORS-LABEL: CHERI __cap_relocs [
// WITH-CTORS-NEXT:    0x12002{{.+}} (__auxargs@CAPTABLE) Base: 0x12003{{.+}} (__auxargs+0) Length: 16 Perms: Object
// WITH-CTORS-NEXT:    0x12002{{.+}} (environ@CAPTABLE) Base: 0x12003{{.+}} (environ+0) Length: 16 Perms: Object
// WITH-CTORS-NEXT:    0x12002{{.+}} (_DYNAMIC@CAPTABLE) Base: 0x0 (<unknown symbol>+0) Length: 0 Perms: Object
// WITH-CTORS-NEXT:    0x12002{{.+}} (atexit@CAPTABLE) Base: 0x12001{{.+}} (atexit+0) Length: 8 Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (_init_tls@CAPTABLE) Base: 0x12001{{.+}} (_init_tls+0) Length: 8 Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (crt_call_constructors@CAPTABLE) Base: 0x12001{{.+}} (crt_call_constructors+0) Length: {{.+}} Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (handle_static_init@CAPTABLE.6) Base: 0x12001{{.+}} (handle_static_init+0) Length: {{.+}} Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (main@CAPTABLE) Base: 0x12001{{.+}} (main+0) Length: 8 Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (exit@CAPTABLE) Base: 0x12001{{.+}} (exit+0) Length: 4 Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (__progname@CAPTABLE) Base: 0x12003{{.+}} (__progname+0) Length: 16 Perms: Object
// WITH-CTORS-NEXT:    0x12002{{.+}} (finalizer@CAPTABLE.10) Base: 0x12001{{.+}} (finalizer+0) Length: 196 Perms: Function
// WITH-CTORS-NEXT:    0x12002{{.+}} (__preinit_array_start@CAPTABLE) Base: 0x120010670 (__preinit_array_end+0) Length: 0 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__preinit_array_end@CAPTABLE) Base: 0x120010670 (__preinit_array_end+0) Length: 0 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__init_array_start@CAPTABLE) Base: 0x120020da0 (__init_array_start+0) Length: 8 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__init_array_end@CAPTABLE) Base:  0x120020da8 (__fini_array_start+0) Length: 0 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__fini_array_start@CAPTABLE) Base: 0x120020da8 (__fini_array_start+0)  Length: 8 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__fini_array_end@CAPTABLE) Base: 0x120020db0 (__auxargs@CAPTABLE+0) Length: 0 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__CTOR_LIST__@CAPTABLE.17) Base: 0x120020d80 (__CTOR_LIST__+0) Length: 8 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__CTOR_END__@CAPTABLE) Base: 0x120020d88 (__CTOR_END__+0) Length: 8 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__start___cap_relocs@CAPTABLE) Base: 0x1200002d0 (__start___cap_relocs+0) Length: 920 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (__stop___cap_relocs@CAPTABLE) Base: 0x120000668 (__stop___cap_relocs+0) Length: 0 Perms: Constant
// WITH-CTORS-NEXT:    0x12002{{.+}} (_int@CAPTABLE) Base: 0x12003{{.+}} (_int+0) Length: 4 Perms: Object
// WITH-CTORS-NEXT:    0x12003{{.+}} (__progname)    Base: 0x1200002c8 (<unknown symbol>+0) Length: 1 Perms: Constant
// WITH-CTORS-NEXT: ]

// WITHOUT-CTORS-SYMS-LABEL: SYMBOL TABLE:
// WITHOUT-CTORS-SYMS:       0000000120020f00 .captable		 00000000 .hidden __cap_table_end
// WITHOUT-CTORS-SYMS:       0000000120020da0 .captable		 00000160 .hidden __cap_table_start
// WITHOUT-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __fini_array_end
// WITHOUT-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __fini_array_start
// WITHOUT-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __init_array_end
// WITHOUT-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __init_array_start
// WITHOUT-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __preinit_array_end
// WITHOUT-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __preinit_array_start
// WITHOUT-CTORS-SYMS:       00000001200002d0 __cap_relocs		 00000398 .protected __start___cap_relocs
// WITHOUT-CTORS-SYMS:       0000000120000668 __cap_relocs		 00000000 .protected __stop___cap_relocs

// WITH-CTORS-SYMS-LABEL: SYMBOL TABLE:
// WITH-CTORS-SYMS:       0000000120020f10 .captable		 00000000 .hidden __cap_table_end
// WITH-CTORS-SYMS:       0000000120020db0 .captable		 00000160 .hidden __cap_table_start
// WITH-CTORS-SYMS:       0000000120020db0 .fini_array		 00000000 .hidden __fini_array_end
// WITH-CTORS-SYMS:       0000000120020da8 .fini_array		 00000008 .hidden __fini_array_start
// WITH-CTORS-SYMS:       0000000120020da8 .init_array		 00000000 .hidden __init_array_end
// WITH-CTORS-SYMS:       0000000120020da0 .init_array		 00000008 .hidden __init_array_start
// WITH-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __preinit_array_end
// WITH-CTORS-SYMS:       0000000120010670 .text		 00000000 .hidden __preinit_array_start
// WITH-CTORS-SYMS:       00000001200002d0 __cap_relocs		 00000398 .protected __start___cap_relocs
// WITH-CTORS-SYMS:       0000000120000668 __cap_relocs		 00000000 .protected __stop___cap_relocs
