// RUN: %cheri256_cc1 -S -o - -O1 -emit-llvm %s | FileCheck -check-prefixes=CHECK256,HYBRID256 %s
// RUN: %cheri128_cc1 -S -o - -O1 -emit-llvm %s | FileCheck -check-prefixes=CHECK128,HYBRID128 %s
// RUN: %cheri256_purecap_cc1 -S -o - -O1 -emit-llvm %s | FileCheck -check-prefixes=CHECK256,PURE256 %s
// RUN: %cheri128_purecap_cc1 -S -o - -O1 -emit-llvm %s | FileCheck -check-prefixes=CHECK128,PURE128 %s

// HYBRID128: target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
// HYBRID256: target datalayout = "E-m:e-pf200:256:256:256:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
// PURE128: target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
// PURE256: target datalayout = "E-m:e-pf200:256:256:256:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"

void * __capability array[2];

void * __capability get(int x)
{
	// CHECK128: align 16
	// CHECK256: align 32
	return array[x];
}

int size(void)
{
	// CHECK128: ret i32 16
	// CHECK256: ret i32 32
	return sizeof(void* __capability);
}
