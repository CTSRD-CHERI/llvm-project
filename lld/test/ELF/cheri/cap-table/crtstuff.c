// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crt1.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crt1.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crtbeginC.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crtbegin.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %S/Inputs/crtendC.c -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-crtend.o
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %s -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -o %t-main.o
// RUN: ld.lld --fatal-warnings -o %t.exe %t-crt1.o %t-crtbegin.o %t-main.o %t-crtend.o -verbose
// RUN: llvm-objdump -r -C -t %t.exe | FileCheck %s -check-prefixes CHECK,WITHOUT_CTORS
// Check that we also don't get any warnings if there are any init_array entries
// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable %s -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt -DINIT_ARRAY_ENTRIES -o %t-main-with-ctors.o
// RUN: ld.lld --fatal-warnings -o %t-ctors.exe %t-crt1.o %t-crtbegin.o %t-main-with-ctors.o %t-crtend.o -verbose
// RUN: llvm-objdump -r -C -h -t %t-ctors.exe
// RUN: llvm-objdump -r -C -t %t-ctors.exe | FileCheck %s -check-prefixes CHECK,WITH_CTORS

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

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK-NEXT:         Base: <Unnamed symbol> (0x00000001200001c8)	Offset: 0x0000000000000000	Length: 0x0000000000000001	Permissions: 0x00000000
// CHECK-NEXT:         Base: __auxargs (0x0000000120040000)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT:         Base: environ (0x0000000120040020)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT:         Base: __progname (0x0000000120020000)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT:         Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT:         Base: atexit (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base: _init_tls (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base: crt_call_constructors (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000080	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base: handle_static_init (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x00000000000001b0	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base: main (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base: exit (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base: finalizer (0x0000000120010{{.+}})	Offset: 0x0000000000000000	Length: 0x00000000000000c0	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT:         Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT:         Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// WITHOUT_CTORS-NEXT: Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// WITHOUT_CTORS-NEXT: Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// WITHOUT_CTORS-NEXT: Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// WITHOUT_CTORS-NEXT: Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// WITH_CTORS-NEXT:    Base: __init_array_start (0x0000000120030020)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
// This one is actually __init_array_end
// WITH_CTORS-NEXT:    Base: __fini_array_start (0x0000000120030028)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// WITH_CTORS-NEXT:    Base: __fini_array_start (0x0000000120030028)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
// This one is actually __finie_array_end
// WITH_CTORS-NEXT:    Base: __cap_table_start (0x0000000120030030)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT:         Base: __CTOR_LIST__ (0x0000000120030000)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
// CHECK-NEXT:         Base: __CTOR_END__ (0x0000000120030008)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
// CHECK-NEXT:         Base: __cap_table_end (0x00000001200301{{80|90}})	Offset: 0x0000000000000000	Length: 0x0000000000000398	Permissions: 0x00000000
// CHECK-NEXT:         Base: __stop___cap_relocs (0x00000001200305{{1|2}}8)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
// CHECK-NEXT:         Base: _int (0x0000000120040010)	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x00000000


// CHECK-LABEL: SYMBOL TABLE:
// CHECK: 00000001200301{{8|9}}0         .cap_table		 00000000 .hidden __cap_table_end
// CHECK: 00000001200300{{2|3}}0         .cap_table		 00000160 .hidden __cap_table_start
// WITHOUT_CTORS: 0000000000000000         *ABS*		 00000000 .hidden __fini_array_end
// WITHOUT_CTORS: 0000000000000000         *ABS*		 00000000 .hidden __fini_array_start
// WITHOUT_CTORS: 0000000000000000         *ABS*		 00000000 .hidden __init_array_end
// WITHOUT_CTORS: 0000000000000000         *ABS*		 00000000 .hidden __init_array_start
// WITH_CTORS: 0000000120030030         .fini_array		 00000000 .hidden __fini_array_end
// WITH_CTORS: 0000000120030028         .fini_array		 00000008 .hidden __fini_array_start
// WITH_CTORS: 0000000120030028         .init_array		 00000000 .hidden __init_array_end
// WITH_CTORS: 0000000120030020         .init_array		 00000008 .hidden __init_array_start
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __preinit_array_end
// CHECK: 0000000000000000         *ABS*		 00000000 .hidden __preinit_array_start
// CHECK: 00000001200301{{8|9}}0         __cap_relocs		 00000398 __start___cap_relocs
// CHECK: 00000001200305{{1|2}}8         __cap_relocs		 00000000 __stop___cap_relocs
