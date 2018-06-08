// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crt1.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crt1.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crtbeginC.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crtbegin.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crtendC.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crtend.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %s -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-main.o
// RUN: ld.lld --fatal-warnings -o %t.exe %t-crt1.o %t-crtbegin.o %t-main.o %t-crtend.o -verbose
// RUN: llvm-objdump -d -r -C -t %t.exe | FileCheck %s


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

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK-NEXT: 0x0000000120020000	Base: <Unnamed symbol> (0x00000001200001c8)	Offset: 0x0000000000000000	Length: 0x0000000000000001	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030020	Base: __auxargs (0x0000000120040000)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030030	Base: environ (0x0000000120040020)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030040	Base: __progname (0x0000000120020000)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030050	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030060	Base: atexit (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x0000000120030070	Base: _init_tls (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x0000000120030080	Base: crt_call_constructors (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000080	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x0000000120030090	Base: handle_static_init (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x00000000000001b0	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x00000001200300a0	Base: main (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x00000001200300b0	Base: exit (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x00000001200300c0	Base: finalizer (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x00000000000000c0	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: 0x00000001200300d0	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x00000001200300e0	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x00000001200300f0	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030100	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030110	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030120	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030130	Base: __CTOR_LIST__ (0x0000000120030000)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030140	Base: __CTOR_END__ (0x0000000120030008)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030150	Base: __cap_table_end (0x0000000120030180)	Offset: 0x0000000000000000	Length: 0x0000000000000398	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030160	Base: __stop___cap_relocs (0x0000000120030518)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT: 0x0000000120030170	Base: _int (0x0000000120040010)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x00000000


// CHECK-LABEL: SYMBOL TABLE:
// CHECK: 0000000120030180         .cap_table		 00000000 .hidden __cap_table_end
// CHECK: 0000000120030020         .cap_table		 00000160 .hidden __cap_table_start
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __fini_array_end
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __fini_array_start
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __init_array_end
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __init_array_start
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __preinit_array_end
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __preinit_array_start
// CHECK: 0000000120030180         __cap_relocs		 00000398 __start___cap_relocs
// CHECK: 0000000120030518         __cap_relocs		 00000000 __stop___cap_relocs
