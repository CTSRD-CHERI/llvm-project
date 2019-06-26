
# RUN: %cheri256_purecap_llvm-mc -filetype=obj %s -o %t-cheri256-main.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t-cheri128-main.o
# RUN: %cheri256_llvm-mc -filetype=obj -target-abi n64 %s -o %t-cheri256-hybrid-main.o
# RUN: %cheri128_llvm-mc -filetype=obj -target-abi n64 %s -o %t-cheri128-hybrid-main.o
# RUN: llvm-mc -triple=mips64-unknown-freebsd -filetype=obj -target-abi n64 -mcpu=beri %s -o %t-beri-main.o
# RUN: %cheri256_purecap_llvm-mc -filetype=obj %S/../Inputs/mips-dynamic.s -o %t-cheri256-lib.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %S/../Inputs/mips-dynamic.s -o %t-cheri128-lib.o
# RUN: %cheri256_llvm-mc -filetype=obj -target-abi n64 %S/../Inputs/mips-dynamic.s -o %t-cheri256-hybrid-lib.o
# RUN: %cheri128_llvm-mc -filetype=obj -target-abi n64 %S/../Inputs/mips-dynamic.s -o %t-cheri128-hybrid-lib.o
# RUN: llvm-readobj -h %t-cheri128-main.o | FileCheck -check-prefix=CHERI128-FLAGS %s
# RUN: llvm-readobj -h %t-cheri256-main.o | FileCheck -check-prefix=CHERI256-FLAGS %s
# RUN: llvm-readobj -h %t-cheri128-hybrid-main.o | FileCheck -check-prefix=CHERI128-HYBRID-FLAGS %s
# RUN: llvm-readobj -h %t-cheri256-hybrid-main.o | FileCheck -check-prefix=CHERI256-HYBRID-FLAGS %s


# Check that setting an explicit emulation doesn't infer the target ABI from the first .o:
# Check that it links fine if we don't pass a -m flag:
# RUN: ld.lld %t-cheri128-hybrid-lib.o %t-cheri128-hybrid-main.o -o /dev/null
# But with -melf64btsmip_cheri_fbsd the target ABI should be purecap (and therefore linking should fail)!
# RUN: not ld.lld -melf64btsmip_cheri_fbsd %t-cheri128-hybrid-lib.o %t-cheri128-hybrid-main.o -o %t.exe 2>&1 | FileCheck -DCHERI_TYPE=cheri128 -check-prefix HYBRID-input-with-explicit-PURECAP %s
# HYBRID-input-with-explicit-PURECAP: error: {{.*}}-cheri128-hybrid-lib.o: ABI 'n64' is incompatible with explicitly selected linker emulation 'elf64btsmip_cheri_fbsd'
# HYBRID-input-with-explicit-PURECAP: error: {{.*}}-cheri128-hybrid-lib.o: ABI 'n64' is incompatible with target ABI 'purecap'
# HYBRID-input-with-explicit-PURECAP: error: {{.*}}-cheri128-hybrid-main.o: ABI 'n64' is incompatible with target ABI 'purecap'

# Similarly purecap input should work fine without a -m flag:
# RUN: ld.lld %t-cheri128-lib.o %t-cheri128-main.o -o /dev/null
# But it should fail if we use -melf64btsmip_fbsd explicitly
# RUN: not ld.lld -melf64btsmip_fbsd %t-cheri128-lib.o %t-cheri128-main.o -o %t.exe 2>&1 | FileCheck -DCHERI_TYPE=cheri256 -check-prefix PURECAP-input-with-explicit-N64 %s
# PURECAP-input-with-explicit-N64: error: {{.*}}-cheri128-lib.o: ABI 'purecap' is incompatible with explicitly selected linker emulation 'elf64btsmip_fbsd'
# PURECAP-input-with-explicit-N64: error: {{.*}}-cheri128-lib.o: ABI 'purecap' is incompatible with target ABI 'n64'
# PURECAP-input-with-explicit-N64: error: {{.*}}-cheri128-main.o: ABI 'purecap' is incompatible with target ABI 'n64'

# Now check that lld does not allow to link incompatible CHERI files

# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd \
# RUN:         %S/../Inputs/mips-dynamic.s -o %t-mips64.o
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd \
# RUN:         %S/../Inputs/mips-dynamic.s -mcpu=mips4 -o %t-mips4.o
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd \
# RUN:         %S/../Inputs/mips-dynamic.s -mcpu=beri -o %t-beri.o
# RUN: not ld.lld %t-cheri256-main.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-vs-MIPS %s
# RUN: not ld.lld %t-cheri128-main.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-vs-MIPS %s
# RUN: not ld.lld %t-cheri256-main.o %t-cheri128-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-vs-CHERI128 %s
# RUN: not ld.lld %t-cheri128-main.o %t-cheri256-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-vs-CHERI256 %s
# RUN: not ld.lld %t-cheri256-main.o %t-cheri256-hybrid-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-vs-CHERI256-HYBRID %s
# RUN: not ld.lld %t-cheri128-main.o %t-cheri128-hybrid-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-vs-CHERI128-HYBRID %s

# linking plain mips with hybrid results in a hybrid binary:
# RUN: ld.lld %t-cheri256-hybrid-main.o %t-mips4.o -o %t.exe
# RUN: llvm-readobj -h %t.exe | FileCheck -check-prefix=CHERI256-HYBRID-FLAGS %s
# RUN: ld.lld %t-cheri128-hybrid-main.o %t-mips4.o -o %t.exe
# RUN: llvm-readobj -h %t.exe | FileCheck -check-prefix=CHERI128-HYBRID-FLAGS %s
# Same for -mcpu=beri
# RUN: ld.lld %t-cheri128-hybrid-main.o %t-beri.o -o %t.exe
# RUN: llvm-readobj -h %t.exe | FileCheck -check-prefix=CHERI128-HYBRID-FLAGS %s
# RUN: ld.lld %t-cheri256-hybrid-main.o %t-beri.o -o %t.exe
# RUN: llvm-readobj -h %t.exe | FileCheck -check-prefix=CHERI256-HYBRID-FLAGS %s


# Should be able to link beri with MIPS4
# RUN: ld.lld %t-beri-main.o %t-mips4.o -o %t.exe
# RUN: llvm-readobj -h %t.exe | FileCheck -check-prefix=BERI-FLAGS %s
# But not against mips64
# RUN: not ld.lld %t-beri-main.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -DCHERI_TYPE=cheri128 -check-prefix BERI-vs-MIPS64 %s
# BERI-vs-MIPS64: incompatible target ISA:
# BERI-vs-MIPS64-NEXT: {{.+}}-beri-main.o: mips4 (beri)
# BERI-vs-MIPS64-NEXT: {{.+}}-mips64.o: mips64


# but not if the plain MIPS arch is a superset of the cheri arch
# RUN: not ld.lld %t-cheri128-hybrid-main.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -DCHERI_TYPE=cheri128 -check-prefix CHERI-MIPS4-vs-MIPS64 %s
# RUN: not ld.lld %t-cheri256-hybrid-main.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -DCHERI_TYPE=cheri256 -check-prefix CHERI-MIPS4-vs-MIPS64 %s
# CHERI-MIPS4-vs-MIPS64: incompatible target ISA:
# CHERI-MIPS4-vs-MIPS64-NEXT: {{.+}}-[[CHERI_TYPE]]-hybrid-main.o: mips4 ([[CHERI_TYPE]])
# CHERI-MIPS4-vs-MIPS64-NEXT: {{.+}}-mips64.o: mips64


# Emit a warning when linking BERI/CHERI with plain MIPS4 since it could result in unpredictable performance on BERI
# because -mcpu=mips4 doesn't schedule loads sensibly
# RUN: not ld.lld --fatal-warnings %t-beri-main.o %t-mips4.o -o /dev/null 2>&1 | FileCheck %s -check-prefix BERI-NON-BERI -DARCH=beri
# RUN: not ld.lld --fatal-warnings %t-cheri128-hybrid-main.o %t-mips4.o -o /dev/null 2>&1 | FileCheck %s -check-prefix BERI-NON-BERI -DARCH=cheri128
# RUN: not ld.lld --fatal-warnings %t-cheri256-hybrid-main.o %t-mips4.o -o /dev/null 2>&1 | FileCheck %s -check-prefix BERI-NON-BERI -DARCH=cheri256

// BERI-NON-BERI: ld.lld: error: linking files compiled for BERI/CHERI and non-BERI/CHERI can result in surprising performance:
// BERI-NON-BERI-NEXT: >>> {{.+}}cheri-elf-flags-err.s.tmp-{{.+}}-main.o: mips4 ([[ARCH]])
// BERI-NON-BERI-NEXT: >>> {{.+}}cheri-elf-flags-err.s.tmp-mips4.o: mips4
// BERI-NON-BERI-EMPTY:

# REQUIRES: mips

  .option pic0
  .text
  .global  __start
__start:
  nop

# BERI-FLAGS:      Machine: EM_MIPS (0x8)
# BERI-FLAGS:      Flags [
# BERI-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# BERI-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# BERI-FLAGS-NEXT:    EF_MIPS_MACH_BERI (0xBE0000)
# BERI-FLAGS-NEXT:  ]
# CHERI256-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI256-FLAGS:      Flags [
# CHERI256-FLAGS-NEXT:    EF_MIPS_ABI_CHERIABI (0xC000)
# CHERI256-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI256-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI256-FLAGS-NEXT:    EF_MIPS_MACH_CHERI256 (0xC20000)
# CHERI256-FLAGS-NEXT:  ]
# CHERI128-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI128-FLAGS:      Flags [
# CHERI128-FLAGS-NEXT:    EF_MIPS_ABI_CHERIABI (0xC000)
# CHERI128-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI128-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI128-FLAGS-NEXT:    EF_MIPS_MACH_CHERI128 (0xC10000)
# CHERI128-FLAGS-NEXT:  ]

# CHERI256-HYBRID-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI256-HYBRID-FLAGS:      Flags [
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_MACH_CHERI256 (0xC20000)
# CHERI256-HYBRID-FLAGS-NEXT:  ]
# CHERI128-HYBRID-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI128-HYBRID-FLAGS:      Flags [
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_MACH_CHERI128 (0xC10000)
# CHERI128-HYBRID-FLAGS-NEXT:  ]

# CHERI256-vs-MIPS: {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o: ABI 'n64' is incompatible with target ABI 'purecap'
# CHERI128-vs-MIPS: {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o: ABI 'n64' is incompatible with target ABI 'purecap'
# CHERI256-vs-CHERI128: incompatible target ISA:
# CHERI256-vs-CHERI128-NEXT: {{.+}}-cheri256-main.o: mips4 (cheri256)
# CHERI256-vs-CHERI128-NEXT: {{.+}}-cheri128-lib.o: mips4 (cheri128)
# CHERI128-vs-CHERI256: incompatible target ISA:
# CHERI128-vs-CHERI256-NEXT: {{.+}}-cheri128-main.o: mips4 (cheri128)
# CHERI128-vs-CHERI256-NEXT: {{.+}}-cheri256-lib.o: mips4 (cheri256)
# CHERI256-vs-CHERI256-HYBRID: {{.*}}/cheri-elf-flags-err.s.tmp-cheri256-hybrid-lib.o: ABI 'n64' is incompatible with target ABI 'purecap'
# CHERI128-vs-CHERI128-HYBRID: {{.*}}/cheri-elf-flags-err.s.tmp-cheri128-hybrid-lib.o: ABI 'n64' is incompatible with target ABI 'purecap'

# TODO: should those files actually link?
# CHERI256-HYBRID-vs-MIPS: target ISA 'cheri256' is incompatible with 'mips64': {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o
# CHERI128-HYBRID-vs-MIPS: target ISA 'cheri128' is incompatible with 'mips64': {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o

