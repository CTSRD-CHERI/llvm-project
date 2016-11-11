// RUN: %clang %s -mabi=sandbox -cheri-linker -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s
#define A (__intcap_t)1
#define B (__intcap_t)(void*)2

int x(__intcap_t y)
{
	// CHECK: llvm.memcap.cap.offset.get
	// CHECK: switch i64 %1
	switch (y)
	{
		// CHECK: i64 1, label
		case A: return 4;
		// CHECK: i64 2, label
		case B: return 5;
	}
}
