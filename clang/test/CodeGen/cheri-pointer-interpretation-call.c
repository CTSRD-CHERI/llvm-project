// RUN: %cheri_cc1 -o - %s -S -emit-llvm | FileCheck %s
#define CHERI_CCALL(suffix, cls) \
	__attribute__((cheri_ccall))\
	__attribute__((cheri_method_suffix(suffix)))\
	__attribute__((cheri_method_class(cls)))

struct cheri_class
{
  void * __capability a;
  void * __capability b;
};

struct cheri_class def;
struct cheri_class other;


CHERI_CCALL("_cap", def)
__attribute__((pointer_interpretation_capabilities))
void *foo(void*, void*);

CHERI_CCALL("_cap", def)
void *baz(void*, void*);

void *bar(void *a, void *b)
{
	// CHECK: call chericcallcc i8* @cheri_invoke(i8 addrspace(200)* %{{.*}}, i8 addrspace(200)* %{{.*}}, i64 zeroext %{{.*}}, i8* %{{.*}}, i8* %{{.*}})
	baz(a, b);
	// CHECK: [[CALL:%.+]] = call chericcallcc i8 addrspace(200)* bitcast (i8* (i8 addrspace(200)*, i8 addrspace(200)*, i64, i8*, i8*)* @cheri_invoke to i8 addrspace(200)* (i8 addrspace(200)*, i8 addrspace(200)*, i64, i8 addrspace(200)*, i8 addrspace(200)*)*)(i8 addrspace(200)* %{{.*}}, i8 addrspace(200)* %{{.*}}, i64 zeroext %{{.*}}, i8 addrspace(200)* %{{.*}}, i8 addrspace(200)* %{{.*}})
	// CHECK:  %{{.*}} = addrspacecast i8 addrspace(200)* [[CALL]] to i8*
	return (__cheri_fromcap void*)foo((__cheri_tocap void * __capability)a, (__cheri_tocap void * __capability)b);
}

