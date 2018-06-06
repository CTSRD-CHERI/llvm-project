# RUN: not %cheri_llvm-mc %s -show-encoding -cheri-strict-c0-asm 2>%t.err | FileCheck %s
# RUN: FileCheck %s -check-prefix ERRORS < %t.err

cincoffset $c1, $cnull, 12
# CHECK: cincoffset	$c1, $cnull, 12    # encoding: [0x4a,0x61,0x00,0x0c]
cincoffset $c1, $ddc, 13
# ERRORS: [[@LINE-1]]:17: error: expected general-purpose CHERI register operand or $cnull
cincoffset $c1, $c0, 14
# ERRORS: [[@LINE-1]]:17: error: register name $c0 is invalid as this operand. It will no longer refer to $ddc but instead be NULL.

cfromptr $c1, $c0, $1
# ERRORS: [[@LINE-1]]:15: warning: register name $c0 is deprecated. Use $ddc instead.
# CHECK: 	cfromddc	$c1, $1         # encoding: [0x48,0x01,0x00,0x53]
cfromptr $c1, $ddc, $2
# CHECK: 	cfromddc	$c1, $2         # encoding: [0x48,0x01,0x00,0x93]
cfromptr $c1, $cnull, $3
# ERRORS: [[@LINE-1]]:15: error: expected general-purpose CHERI register operand or $ddc
