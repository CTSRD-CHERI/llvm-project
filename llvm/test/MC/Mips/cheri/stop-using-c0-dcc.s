# Check that it assembles if I no longer defined INCLUDE_BAD
# RUN: not %cheri_llvm-mc %s -defsym=INCLUDE_BAD=1 -show-encoding 2>&1 -cheri-strict-ddc-asm | FileCheck %s
# Check that we assemble successfully if we exclude the bad instrs
# RUN: %cheri_llvm-mc %s -show-encoding -cheri-strict-ddc-asm -o /dev/null 2>&1

.ifdef INCLUDE_BAD
cmove $c0, $c1  # CHECK: [[@LINE]]:7: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
cmove $ddc, $c1  # CHECK: [[@LINE]]:7: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
cmove $c2, $c0  # CHECK: [[@LINE]]:12: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
cmove $c2, $ddc  # CHECK: [[@LINE]]:12: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
cincoffset $c0, $c0, 2  # CHECK: [[@LINE]]:12: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.


# Using $c0 as a non-source-address register should be an error:
cfromptr $c0, $c1, $3  # CHECK: 39:10: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
ctoptr $3, $c1, $c0    # CHECK: [[@LINE]]:17: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
clc $c0, $zero, 0($c2) # CHECK: [[@LINE]]:5: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
csc $c0, $zero, 0($c2) # CHECK: [[@LINE]]:5: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
cllc $c0, $c1          # CHECK: [[@LINE]]:6: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
cscc $2, $c0, $c1      # CHECK: [[@LINE]]:10: error: Direct access to DDC is deprecated: use C(Get/Set)Default instead.
.endif


# However, cfromptr, ctoptr, cl* and cs* should still work with $c0 as the address register
cfromptr $c1, $c0, $3    # CHECK: cfromddc	$c1, $3             # encoding: [0x48,0x01,0x00,0xd3]
ctoptr $3, $c0, $c1      # CHECK: ctoptr	$3, $c0, $c1          # encoding: [0x48,0x03,0x00,0x52]
clc $c1, $zero, 0($c0)   # CHECK: clc	$c1, $zero, 0($c0)        # encoding: [0xd8,0x20,0x00,0x00]
clcbi $c1, 0($c0)        # CHECK: clcbi	$c1, 0($c0)             # encoding: [0x74,0x20,0x00,0x00]
clw $3, $zero, 0($c0)    # CHECK: clw	$3, $zero, 0($c0)         # encoding: [0xc8,0x60,0x00,0x06]
csc $c1, $zero, 0($c0)   # CHECK: csc	$c1, $zero, 0($c0)        # encoding: [0xf8,0x20,0x00,0x00]
cscbi $c1, 0($c0)        # CHECK: cscbi	$c1, 0($c0)             # encoding: [0x78,0x20,0x00,0x00]
csd $3, $zero, 0($c0)    # CHECK: csd	$3, $zero, 0($c0)         # encoding: [0xe8,0x60,0x00,0x03]
cllc $c1, $c0            # CHECK: cllc	$c1, $c0                # encoding: [0x4a,0x01,0x00,0x0f]
cllbu $2, $c1            # CHECK: cllbu	$2, $c1                 # encoding: [0x4a,0x02,0x08,0x08]
cscc $2, $c1, $c0        # CHECK: cscc	$2, $c1, $c0            # encoding: [0x4a,0x01,0x00,0x87]
cscb $2, $3, $c0         # CHECK: cscb	$2, $3, $c0             # encoding: [0x4a,0x03,0x00,0x80]



nop   # CHECK-LABEL: nop
