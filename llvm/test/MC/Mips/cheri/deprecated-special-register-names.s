# Check that it assembles if I no longer defined INCLUDE_BAD
# Check that we assemble successfully if we exclude the bad instrs
# RUN: not %cheri_llvm-mc %s -show-encoding -o - 2>&1 | FileCheck %s

cmove $c2, $kr1c
# CHECK: error: Register $kr1c is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}kr1c instead. If you really want to access $c27 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
cmove $c2, $kr2c
# CHECK: error: Register $kr2c is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}kr2c instead. If you really want to access $c28 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
cmove $c2, $kcc
# CHECK: error: Register $kcc is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}kcc instead. If you really want to access $c29 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
cmove $c2, $kdc
# CHECK: error: Register $kdc is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}kdc instead. If you really want to access $c30 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
cmove $c2, $epcc
# CHECK: error: Register $epcc is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}epcc instead. If you really want to access $c31 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
cmove $c2, $ddc
# CHECK: error: expected general-purpose CHERI register operand or $cnull
