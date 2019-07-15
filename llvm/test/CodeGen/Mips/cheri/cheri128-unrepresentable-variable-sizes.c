// REQUIRES: clang, cheri_is_128
// TODO: convert to IR test (but that requires filling in the full @llvm.used array which is annoying
// RUN: %cheri_purecap_clang -ffunction-sections -fdata-sections %s -S -o - -emit-llvm | FileCheck %s -check-prefix IR
// RUN: %cheri_purecap_clang -ffunction-sections -fdata-sections %s -S -o -
// RUN: %cheri_purecap_clang -ffunction-sections -fdata-sections %s -S -o %t.s
// RUN: FileCheck %s -check-prefix ASM -input-file=%t.s
// RUN: %cheri_purecap_clang -ffunction-sections -fdata-sections %s -c -o - | llvm-readelf --sections --symbols - | FileCheck %s -check-prefix SECTIONS
// Check that we can compile the generated assembly and it matches the direct emission
// RUN: %cheri_purecap_clang %t.s -c -o - | llvm-readelf --sections --symbols - | FileCheck %s -check-prefix SECTIONS

// Check that all data sections in the ELF file are appropriately aligned and the size has been rounded up:
// SECTIONS-LABEL: Name                          Type            Address          Off          Size   ES Flg Lk Inf Al
// SECTIONS-NEXT:                               NULL            0000000000000000 000000        000000 00      0   0  0
// SECTIONS-NEXT: .strtab                       STRTAB          0000000000000000
// SECTIONS-NEXT: .text                         PROGBITS        0000000000000000 {{[0-9a-f]+}} 000000 00  AX  0   0 16
// SECTIONS-NEXT: .mdebug.abi64                 PROGBITS        0000000000000000 {{[0-9a-f]+}} 000000 00      0   0  1
// SECTIONS-NEXT: .data.char128                 PROGBITS        0000000000000000 {{[0-9a-f]+}} 000080 00  WA  0   0  1
// SECTIONS-NEXT: .data.char4096                PROGBITS        0000000000000000 {{[0-9a-f]+}} 001000 00  WA  0   0  8
// SECTIONS-NEXT: .data.char65535               PROGBITS        0000000000000000 {{[0-9a-f]+}} 010000 00  WA  0   0 128
// SECTIONS-NEXT: .data.char65537               PROGBITS        0000000000000000 {{[0-9a-f]+}} 010080 00  WA  0   0 128
// SECTIONS-NEXT: .data.char139267              PROGBITS        0000000000000000 {{[0-9a-f]+}} 022100 00  WA  0   0 256
// SECTIONS-NEXT: .tdata.thread_char128         PROGBITS        0000000000000000 {{[0-9a-f]+}} 000080 00 WAT  0   0  1
// SECTIONS-NEXT: .tdata.thread_char4096        PROGBITS        0000000000000000 {{[0-9a-f]+}} 001000 00 WAT  0   0  8
// SECTIONS-NEXT: .tdata.thread_char65535       PROGBITS        0000000000000000 {{[0-9a-f]+}} 010000 00 WAT  0   0 128
// SECTIONS-NEXT: .tdata.thread_char65537       PROGBITS        0000000000000000 {{[0-9a-f]+}} 010080 00 WAT  0   0 128
// SECTIONS-NEXT: .tdata.thread_char139267      PROGBITS        0000000000000000 {{[0-9a-f]+}} 022100 00 WAT  0   0 256
// SECTIONS-NEXT: .bss.zero128                  NOBITS          0000000000000000 {{[0-9a-f]+}} 000080 00  WA  0   0  1
// SECTIONS-NEXT: .bss.zero4096                 NOBITS          0000000000000000 {{[0-9a-f]+}} 001000 00  WA  0   0  8
// SECTIONS-NEXT: .bss.zero65535                NOBITS          0000000000000000 {{[0-9a-f]+}} 010000 00  WA  0   0 128
// SECTIONS-NEXT: .bss.zero65537                NOBITS          0000000000000000 {{[0-9a-f]+}} 010080 00  WA  0   0 128
// SECTIONS-NEXT: .bss.zero139267               NOBITS          0000000000000000 {{[0-9a-f]+}} 022100 00  WA  0   0 256
// SECTIONS-NEXT: .tbss.thread_zero128          NOBITS          0000000000000000 {{[0-9a-f]+}} 000080 00 WAT  0   0  1
// SECTIONS-NEXT: .tbss.thread_zero4096         NOBITS          0000000000000000 {{[0-9a-f]+}} 001000 00 WAT  0   0  8
// SECTIONS-NEXT: .tbss.thread_zero65535        NOBITS          0000000000000000 {{[0-9a-f]+}} 010000 00 WAT  0   0 128
// SECTIONS-NEXT: .tbss.thread_zero65537        NOBITS          0000000000000000 {{[0-9a-f]+}} 010080 00 WAT  0   0 128
// SECTIONS-NEXT: .tbss.thread_zero139267       NOBITS          0000000000000000 {{[0-9a-f]+}} 022100 00 WAT  0   0 256

// Check that the sizes of the symbols have not been rounded up:
// SECTIONS-LABEL:   Value          Size Type    Bind   Vis      Ndx Name
// SECTIONS-NEXT: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
// SECTIONS-NEXT: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS cheri128-unrepresentable-variable-sizes.c
// SECTIONS-NEXT: 0000000000000000   128 OBJECT  LOCAL  DEFAULT    4 char128
// SECTIONS-NEXT: 0000000000000000  4096 OBJECT  LOCAL  DEFAULT    5 char4096
// SECTIONS-NEXT: 0000000000000000   128 TLS     LOCAL  DEFAULT    9 thread_char128
// SECTIONS-NEXT: 0000000000000000  4096 TLS     LOCAL  DEFAULT   10 thread_char4096
// SECTIONS-NEXT: 0000000000000000   128 OBJECT  LOCAL  DEFAULT   14 zero128
// SECTIONS-NEXT: 0000000000000000 139267 OBJECT LOCAL  DEFAULT   18 zero139267
// SECTIONS-NEXT: 0000000000000000  4096 OBJECT  LOCAL  DEFAULT   15 zero4096
// SECTIONS-NEXT: 0000000000000000 65535 OBJECT  LOCAL  DEFAULT   16 zero65535
// SECTIONS-NEXT: 0000000000000000 65537 OBJECT  LOCAL  DEFAULT   17 zero65537
// SECTIONS-NEXT: 0000000000000000 139267 OBJECT GLOBAL PROTECTED   8 char139267
// SECTIONS-NEXT: 0000000000000000 65535 OBJECT  GLOBAL DEFAULT    6 char65535
// SECTIONS-NEXT: 0000000000000000 65537 OBJECT  GLOBAL HIDDEN     7 char65537
// SECTIONS-NEXT: 0000000000000001   128 OBJECT  GLOBAL DEFAULT  COM common128
// SECTIONS-NEXT: 0000000000000100 139267 OBJECT GLOBAL PROTECTED COM common139267
// SECTIONS-NEXT: 0000000000000008  4096 OBJECT  GLOBAL DEFAULT  COM common4096
// SECTIONS-NEXT: 0000000000000080 65535 OBJECT  GLOBAL DEFAULT  COM common65535
// SECTIONS-NEXT: 0000000000000080 65537 OBJECT  GLOBAL HIDDEN   COM common65537
// SECTIONS-NEXT: 0000000000000000 139267 TLS    GLOBAL PROTECTED  13 thread_char139267
// SECTIONS-NEXT: 0000000000000000 65535 TLS     GLOBAL DEFAULT   11 thread_char65535
// SECTIONS-NEXT: 0000000000000000 65537 TLS     GLOBAL HIDDEN    12 thread_char65537
// SECTIONS-NEXT: 0000000000000000   128 TLS     GLOBAL DEFAULT   19 thread_zero128
// SECTIONS-NEXT: 0000000000000000 139267 TLS    GLOBAL PROTECTED  23 thread_zero139267
// SECTIONS-NEXT: 0000000000000000  4096 TLS     GLOBAL DEFAULT   20 thread_zero4096
// SECTIONS-NEXT: 0000000000000000 65535 TLS     GLOBAL DEFAULT   21 thread_zero65535
// SECTIONS-NEXT: 0000000000000000 65537 TLS     GLOBAL HIDDEN   22 thread_zero65537

__attribute__((used)) static char char128[128] = {1};
__attribute__((used)) static char char4096[4096] = {2};
char char65535[65535] = {3};
__attribute__((visibility("hidden"))) char char65537[65537] = {4};
__attribute__((visibility("protected"))) char char139267[139267] = {5};

// In the IR all variables should have align 1
// IR: @char128 = internal addrspace(200) global <{ i8, [127 x i8] }> <{ i8 1, [127 x i8] zeroinitializer }>, align 1
// IR: @char4096 = internal addrspace(200) global <{ i8, [4095 x i8] }> <{ i8 2, [4095 x i8] zeroinitializer }>, align 1
// IR: @char65535 = addrspace(200) global <{ i8, [65534 x i8] }> <{ i8 3, [65534 x i8] zeroinitializer }>, align 1
// IR: @char65537 = hidden addrspace(200) global <{ i8, [65536 x i8] }> <{ i8 4, [65536 x i8] zeroinitializer }>, align 1
// IR: @char139267 = protected addrspace(200) global <{ i8, [139266 x i8] }> <{ i8 5, [139266 x i8] zeroinitializer }>, align 1

// TLS variables:
__attribute__((used)) __thread static char thread_char128[128] = {1};
__attribute__((used)) __thread static char thread_char4096[4096] = {2};
__thread char thread_char65535[65535] = {3};
__attribute__((visibility("hidden"))) __thread char thread_char65537[65537] = {4};
__attribute__((visibility("protected"))) __thread char thread_char139267[139267] = {5};
// IR: @thread_char128 = internal thread_local addrspace(200) global <{ i8, [127 x i8] }> <{ i8 1, [127 x i8] zeroinitializer }>, align 1
// IR: @thread_char4096 = internal thread_local addrspace(200) global <{ i8, [4095 x i8] }> <{ i8 2, [4095 x i8] zeroinitializer }>, align 1
// IR: @thread_char65535 = thread_local addrspace(200) global <{ i8, [65534 x i8] }> <{ i8 3, [65534 x i8] zeroinitializer }>, align 1
// IR: @thread_char65537 = hidden thread_local addrspace(200) global <{ i8, [65536 x i8] }> <{ i8 4, [65536 x i8] zeroinitializer }>, align 1
// IR: @thread_char139267 = protected thread_local addrspace(200) global <{ i8, [139266 x i8] }> <{ i8 5, [139266 x i8] zeroinitializer }>, align 1


// Also test bss variables
__attribute__((used)) static char zero128[128];
__attribute__((used)) static char zero4096[4096];
__attribute__((used)) static char zero65535[65535];
__attribute__((used, visibility("hidden"))) static char zero65537[65537];
__attribute__((used, visibility("protected"))) static char zero139267[139267];
// IR: @zero128 = internal addrspace(200) global [128 x i8] zeroinitializer, align 1
// IR: @zero4096 = internal addrspace(200) global [4096 x i8] zeroinitializer, align 1
// IR: @zero65535 = internal addrspace(200) global [65535 x i8] zeroinitializer, align 1
// IR: @zero65537 = internal addrspace(200) global [65537 x i8] zeroinitializer, align 1
// IR: @zero139267 = internal addrspace(200) global [139267 x i8] zeroinitializer, align 1

// Common symbols:
char common128[128];
char common4096[4096];
char common65535[65535];
__attribute__((visibility("hidden"))) char common65537[65537];
__attribute__((visibility("protected"))) char common139267[139267];

// IR: @common128 = common addrspace(200) global [128 x i8] zeroinitializer, align 1
// IR: @common4096 = common addrspace(200) global [4096 x i8] zeroinitializer, align 1
// IR: @common65535 = common addrspace(200) global [65535 x i8] zeroinitializer, align 1
// IR: @common65537 = common hidden addrspace(200) global [65537 x i8] zeroinitializer, align 1
// IR: @common139267 = common protected addrspace(200) global [139267 x i8] zeroinitializer, align 1

// zero-init TLS
__attribute__((used)) __thread char thread_zero128[128];
__attribute__((used)) __thread char thread_zero4096[4096];
__thread char thread_zero65535[65535];
__attribute__((visibility("hidden"))) __thread char thread_zero65537[65537];
__attribute__((visibility("protected"))) __thread char thread_zero139267[139267];
// IR: @thread_zero128 = thread_local addrspace(200) global [128 x i8] zeroinitializer, align 1
// IR: @thread_zero4096 = thread_local addrspace(200) global [4096 x i8] zeroinitializer, align 1
// IR: @thread_zero65535 = thread_local addrspace(200) global [65535 x i8] zeroinitializer, align 1
// IR: @thread_zero65537 = hidden thread_local addrspace(200) global [65537 x i8] zeroinitializer, align 1
// IR: @thread_zero139267 = protected thread_local addrspace(200) global [139267 x i8] zeroinitializer, align 1


// ASM-LABEL: 	.section	.data.char128,"aw",@progbits
// ASM-NEXT: char128:
// ASM-NEXT: 	.byte	1                       # 0x1
// ASM-NEXT: 	.space	127
// ASM-NEXT: 	.size	char128, 128

// No need for tail padding here but need to align section:
// ASM-LABEL: 	.section	.data.char4096,"aw",@progbits
// ASM-NEXT: 	.p2align	3
// ASM-NEXT: char4096:
// ASM-NEXT: 	.byte	2                       # 0x2
// ASM-NEXT: 	.space	4095
// ASM-NEXT: 	.size	char4096, 4096

// ASM-LABEL: 	.section	.data.char65535,"aw",@progbits
// ASM-NEXT: 	.globl	char65535
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: char65535:
// ASM-NEXT: 	.byte	3                       # 0x3
// ASM-NEXT: 	.space	65534
// ASM-NEXT: 	.space	1                       # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	char65535, 65535

// ASM-LABEL: 	.section	.data.char65537,"aw",@progbits
// ASM-NEXT: 	.globl	char65537
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: char65537:
// ASM-NEXT: 	.byte	4                       # 0x4
// ASM-NEXT: 	.space	65536
// ASM-NEXT: 	.space	127                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	char65537, 65537

// ASM-LABEL: 	.section	.data.char139267,"aw",@progbits
// ASM-NEXT: 	.globl	char139267
// ASM-NEXT: 	.p2align	8
// ASM-NEXT: char139267:
// ASM-NEXT: 	.byte	5                       # 0x5
// ASM-NEXT: 	.space	139266
// ASM-NEXT: 	.space	253                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	char139267, 139267
// ASM-EMPTY:
// ASM-NEXT: 	.type	thread_char128,@object  # @thread_char128
// ASM-NEXT: 	.section	.tdata.thread_char128,"awT",@progbits
// ASM-NEXT: thread_char128:
// ASM-NEXT: 	.byte	1                       # 0x1
// ASM-NEXT: 	.space	127
// ASM-NEXT: 	.size	thread_char128, 128
// ASM-EMPTY:
// ASM-NEXT: 	.type	thread_char4096,@object # @thread_char4096
// ASM-NEXT: 	.section	.tdata.thread_char4096,"awT",@progbits
// ASM-NEXT: 	.p2align	3
// ASM-NEXT: thread_char4096:
// ASM-NEXT: 	.byte	2                       # 0x2
// ASM-NEXT: 	.space	4095
// ASM-NEXT: 	.size	thread_char4096, 4096
// ASM-EMPTY:
// ASM-NEXT: 	.type	thread_char65535,@object # @thread_char65535
// ASM-NEXT: 	.section	.tdata.thread_char65535,"awT",@progbits
// ASM-NEXT: 	.globl	thread_char65535
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: thread_char65535:
// ASM-NEXT: 	.byte	3                       # 0x3
// ASM-NEXT: 	.space	65534
// ASM-NEXT: 	.space	1                       # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	thread_char65535, 65535
// ASM-EMPTY:
// ASM-NEXT: 	.hidden	thread_char65537        # @thread_char65537
// ASM-NEXT: 	.type	thread_char65537,@object
// ASM-NEXT: 	.section	.tdata.thread_char65537,"awT",@progbits
// ASM-NEXT: 	.globl	thread_char65537
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: thread_char65537:
// ASM-NEXT: 	.byte	4                       # 0x4
// ASM-NEXT: 	.space	65536
// ASM-NEXT: 	.space	127                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	thread_char65537, 65537
// ASM-EMPTY:
// ASM-NEXT: 	.protected	thread_char139267 # @thread_char139267
// ASM-NEXT: 	.type	thread_char139267,@object
// ASM-NEXT: 	.section	.tdata.thread_char139267,"awT",@progbits
// ASM-NEXT: 	.globl	thread_char139267
// ASM-NEXT: 	.p2align	8
// ASM-NEXT: thread_char139267:
// ASM-NEXT: 	.byte	5                       # 0x5
// ASM-NEXT: 	.space	139266
// ASM-NEXT: 	.space	253                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	thread_char139267, 139267
// ASM-EMPTY:
// ASM-NEXT: 	.type	zero128,@object         # @zero128
// ASM-NEXT: 	.section	.bss.zero128,"aw",@nobits
// ASM-NEXT: zero128:
// ASM-NEXT: 	.space	128
// ASM-NEXT: 	.size	zero128, 128
// ASM-EMPTY:
// ASM-NEXT: 	.type	zero4096,@object        # @zero4096
// ASM-NEXT: 	.section	.bss.zero4096,"aw",@nobits
// ASM-NEXT: 	.p2align	3
// ASM-NEXT: zero4096:
// ASM-NEXT: 	.space	4096
// ASM-NEXT: 	.size	zero4096, 4096
// ASM-EMPTY:
// ASM-NEXT: 	.type	zero65535,@object       # @zero65535
// ASM-NEXT: 	.section	.bss.zero65535,"aw",@nobits
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: zero65535:
// ASM-NEXT: 	.space	65535
// ASM-NEXT: 	.space	1                       # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	zero65535, 65535
// ASM-EMPTY:
// ASM-NEXT: 	.type	zero65537,@object       # @zero65537
// ASM-NEXT: 	.section	.bss.zero65537,"aw",@nobits
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: zero65537:
// ASM-NEXT: 	.space	65537
// ASM-NEXT: 	.space	127                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	zero65537, 65537
// ASM-EMPTY:
// ASM-NEXT: 	.type	zero139267,@object      # @zero139267
// ASM-NEXT: 	.section	.bss.zero139267,"aw",@nobits
// ASM-NEXT: 	.p2align	8
// ASM-NEXT: zero139267:
// ASM-NEXT: 	.space	139267
// ASM-NEXT: 	.space	253                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	zero139267, 139267
// ASM-EMPTY:
// ASM-NEXT: 	.type	common128,@object       # @common128
// ASM-NEXT: 	.comm	common128,128,1
// ASM-NEXT: 	.type	common4096,@object      # @common4096
// ASM-NEXT: 	.comm	common4096,4096,8
// ASM-NEXT: 	.type	common65535,@object     # @common65535
// ASM-NEXT: 	.comm	common65535,65536,128   # adding 1 bytes of tail padding for precise bounds.
// ASM-NEXT: 	.size	common65535, 65535      # explicit size directive required due to 1 bytes of tail padding for precise bounds.
// ASM-NEXT: 	.hidden	common65537             # @common65537
// ASM-NEXT: 	.type	common65537,@object
// ASM-NEXT: 	.comm	common65537,65664,128   # adding 127 bytes of tail padding for precise bounds.
// ASM-NEXT: 	.size	common65537, 65537      # explicit size directive required due to 127 bytes of tail padding for precise bounds.
// ASM-NEXT: 	.protected	common139267    # @common139267
// ASM-NEXT: 	.type	common139267,@object
// ASM-NEXT: 	.comm	common139267,139520,256 # adding 253 bytes of tail padding for precise bounds.
// ASM-NEXT: 	.size	common139267, 139267    # explicit size directive required due to 253 bytes of tail padding for precise bounds.
// ASM-NEXT: 	.type	thread_zero128,@object  # @thread_zero128
// ASM-NEXT: 	.section	.tbss.thread_zero128,"awT",@nobits
// ASM-NEXT: 	.globl	thread_zero128
// ASM-NEXT: thread_zero128:
// ASM-NEXT: 	.space	128
// ASM-NEXT: 	.size	thread_zero128, 128
// ASM-EMPTY:
// ASM-NEXT: 	.type	thread_zero4096,@object # @thread_zero4096
// ASM-NEXT: 	.section	.tbss.thread_zero4096,"awT",@nobits
// ASM-NEXT: 	.globl	thread_zero4096
// ASM-NEXT: 	.p2align	3
// ASM-NEXT: thread_zero4096:
// ASM-NEXT: 	.space	4096
// ASM-NEXT: 	.size	thread_zero4096, 4096
// ASM-EMPTY:
// ASM-NEXT: 	.type	thread_zero65535,@object # @thread_zero65535
// ASM-NEXT: 	.section	.tbss.thread_zero65535,"awT",@nobits
// ASM-NEXT: 	.globl	thread_zero65535
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: thread_zero65535:
// ASM-NEXT: 	.space	65535
// ASM-NEXT: 	.space	1                       # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	thread_zero65535, 65535
// ASM-EMPTY:
// ASM-NEXT: 	.hidden	thread_zero65537        # @thread_zero65537
// ASM-NEXT: 	.type	thread_zero65537,@object
// ASM-NEXT: 	.section	.tbss.thread_zero65537,"awT",@nobits
// ASM-NEXT: 	.globl	thread_zero65537
// ASM-NEXT: 	.p2align	7
// ASM-NEXT: thread_zero65537:
// ASM-NEXT: 	.space	65537
// ASM-NEXT: 	.space	127                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	thread_zero65537, 65537
// ASM-EMPTY:
// ASM-NEXT: 	.protected	thread_zero139267 # @thread_zero139267
// ASM-NEXT: 	.type	thread_zero139267,@object
// ASM-NEXT: 	.section	.tbss.thread_zero139267,"awT",@nobits
// ASM-NEXT: 	.globl	thread_zero139267
// ASM-NEXT: 	.p2align	8
// ASM-NEXT: thread_zero139267:
// ASM-NEXT: 	.space	139267
// ASM-NEXT: 	.space	253                     # Tail padding to ensure precise bounds
// ASM-NEXT: 	.size	thread_zero139267, 139267
