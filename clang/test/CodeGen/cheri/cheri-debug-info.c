// REQUIRES: asserts
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -emit-llvm -O0 -o - %s | FileCheck %s -check-prefix CHECK-IR
// RUN: %cheri_cc1 -mrelocation-model pic "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -emit-llvm -O0 -o - %s | FileCheck %s -check-prefix CHECK-IR

// Generate a .o file and compare the debug info:
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -emit-obj -O0 -o %t-cheri.o %s -mllvm -debug-only=dwarfdebug
// RUN: %clang_cc1 -triple=mips64-unknown-freebsd "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -emit-obj -O0 -o %t-mips.o %s
// RUN: %cheri_cc1 "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -emit-obj -O0 -o %t-hybrid.o %s

// Check that hybrid and MIPS generated asm is the same:
// RUN: %clang_cc1 -triple=mips64-unknown-freebsd "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -S -O0 -o %t-mips.s %s
// RUN: %cheri_cc1 "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb"  -S -O0 -o %t-hybrid.s %s
// Should be the same other than some ELF flags:
// RUN: diff -u %t-mips.s %t-hybrid.s

// Previoulsy llvm-dwarfdump would not handle MIPS relocations for a CHERI triple:
// RUN: llvm-dwarfdump -verify %t-mips.o 2>&1 | FileCheck %s -check-prefix DWARFDUMP-VERIFY
// RUN: llvm-dwarfdump -verify %t-hybrid.o 2>&1 | FileCheck %s -check-prefix DWARFDUMP-VERIFY
// RUN: llvm-dwarfdump -verify %t-cheri.o 2>&1 | FileCheck %s -check-prefix DWARFDUMP-VERIFY
// DWARFDUMP-VERIFY-NOT: failed to compute relocation
// DWARFDUMP-VERIFY: No errors.
// DWARFDUMP-VERIFY-NOT: failed to compute relocation

// Now comapre the dwarfdump output for CHERI and MIPS:
// RUN: llvm-dwarfdump -all %t-cheri.o > %t.dump.cheri
// RUN: llvm-dwarfdump -all %t-mips.o > %t.dump.mips
// RUN: not diff -u %t.dump.mips %t.dump.cheri
// TODO: verify the output (it seems broken)

int foo(int* i) {
	if (i) {
		int j = 2;
	}
	else {
		int j = 3;
	}
	return (int)i;
// CHECK-IR: ret i32 %{{.+}}, !dbg !28
// Both MIPS and purecap should have the same number of metadata nodes:
// CHECK-IR: !28 = !DILocation(line: [[@LINE-3]], column: 2, scope: !9)
}
