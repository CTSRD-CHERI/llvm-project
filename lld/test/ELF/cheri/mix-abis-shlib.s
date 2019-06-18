# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=pcrel -filetype=obj %s -o %t-pcrel.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=plt -filetype=obj %s -o %t-plt.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=fn-desc -filetype=obj %s -o %t-fn-desc.o
# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t-mips.o -defsym=LIBRARY=1 -defsym=MIPS=1

# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=legacy -filetype=obj %s -o %t-lib-legacy.o -defsym=LIBRARY=1
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=pcrel -filetype=obj %s -o %t-lib-pcrel.o -defsym=LIBRARY=1
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=plt -filetype=obj %s -o %t-lib-plt.o -defsym=LIBRARY=1
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=fn-desc -filetype=obj %s -o %t-lib-fn-desc.o -defsym=LIBRARY=1
# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t-lib-mips.o -defsym=LIBRARY=1 -defsym=MIPS=1

# RUN: ld.lld -shared -o %t-lib-legacy.so %t-lib-legacy.o
# RUN: ld.lld -shared -o %t-lib-pcrel.so %t-lib-pcrel.o
# RUN: ld.lld -shared -o %t-lib-plt.so %t-lib-plt.o
# RUN: ld.lld -shared -o %t-lib-fn-desc.so %t-lib-fn-desc.o
# RUN: ld.lld -shared -o %t-lib-mips.so %t-lib-mips.o


## Linking against shared libraries should work for pcrel/plt/fndesc (but warn if incompatible)
## However, linking against any legacy shared library should be an error!
## Similarly linking against mips libraries is also an error!

## PCREL
# RUN: not ld.lld -pie -o /dev/null %t-pcrel.o %t-lib-legacy.so 2>&1 | FileCheck %s -check-prefix ERR-SHARED -DIN_ABI=PCREL -DBAD_ABI=LEGACY
# RUN: not ld.lld -pie -o /dev/null %t-pcrel.o %t-lib-mips.so 2>&1 | FileCheck %s -check-prefix ERR-CHERIABI-LINKING-MIPS
# RUN: ld.lld -pie -o /dev/null %t-pcrel.o %t-lib-plt.so 2>&1 | FileCheck %s -check-prefix WARN-SHARED -DIN_ABI=PCREL -DBAD_ABI=PLT
# RUN: ld.lld -pie -o /dev/null %t-pcrel.o %t-lib-fn-desc.so 2>&1 | FileCheck %s -check-prefix WARN-SHARED -DIN_ABI=PCREL -DBAD_ABI=FNDESC

# RUN: ld.lld -pie -o - %t-lib-pcrel.so %t-pcrel.o | llvm-readobj -dynamic -h - | FileCheck -check-prefix PCREL %s

## PLT
# RUN: not ld.lld -pie -o /dev/null %t-plt.o %t-lib-legacy.so 2>&1 | FileCheck %s -check-prefix ERR-SHARED -DIN_ABI=PLT -DBAD_ABI=LEGACY
# RUN: not ld.lld -pie -o /dev/null %t-plt.o %t-lib-mips.so 2>&1 | FileCheck %s -check-prefix ERR-CHERIABI-LINKING-MIPS
# RUN: ld.lld -pie -o /dev/null %t-plt.o %t-lib-fn-desc.so 2>&1 | FileCheck %s -check-prefix WARN-SHARED -DIN_ABI=PLT -DBAD_ABI=FNDESC
# RUN: ld.lld -pie -o /dev/null %t-plt.o %t-lib-pcrel.so 2>&1 | FileCheck %s -check-prefix WARN-SHARED -DIN_ABI=PLT -DBAD_ABI=PCREL

# RUN: ld.lld -pie -o - %t-lib-plt.so %t-plt.o | llvm-readobj -dynamic -h - | FileCheck -check-prefix PLT %s

## FNDESC
# RUN: not ld.lld -pie -o /dev/null %t-fn-desc.o %t-lib-legacy.so 2>&1 | FileCheck %s -check-prefix ERR-SHARED -DIN_ABI=FNDESC -DBAD_ABI=LEGACY
# RUN: not ld.lld -pie -o /dev/null %t-fn-desc.o %t-lib-mips.so 2>&1 | FileCheck %s -check-prefix ERR-CHERIABI-LINKING-MIPS
# RUN: ld.lld -pie -o /dev/null %t-fn-desc.o %t-lib-pcrel.so 2>&1 | FileCheck %s -check-prefix WARN-SHARED -DIN_ABI=FNDESC -DBAD_ABI=PCREL
# RUN: ld.lld -pie -o /dev/null %t-fn-desc.o %t-lib-plt.so 2>&1 | FileCheck %s -check-prefix WARN-SHARED -DIN_ABI=FNDESC -DBAD_ABI=PLT

# RUN: ld.lld -pie -o - %t-lib-fn-desc.so %t-fn-desc.o | llvm-readobj -dynamic -h - | FileCheck -check-prefix FNDESC %s

## LEGACY
# RUN: not ld.lld -pie -o /dev/null %t-legacy.o %t-lib-pcrel.so 2>&1 | FileCheck %s -check-prefix ERR-SHARED -DIN_ABI=LEGACY -DBAD_ABI=PCREL
# RUN: not ld.lld -pie -o /dev/null %t-legacy.o %t-lib-plt.so 2>&1 | FileCheck %s -check-prefix ERR-SHARED -DIN_ABI=LEGACY -DBAD_ABI=PLT
# RUN: not ld.lld -pie -o /dev/null %t-legacy.o %t-lib-fn-desc.so 2>&1 | FileCheck %s -check-prefix ERR-SHARED -DIN_ABI=LEGACY -DBAD_ABI=FNDESC
# RUN: not ld.lld -pie -o /dev/null %t-legacy.o %t-lib-mips.so 2>&1 | FileCheck %s -check-prefix ERR-CHERIABI-LINKING-MIPS

# RUN: ld.lld -pie -o - %t-lib-legacy.so %t-legacy.o | llvm-readobj -dynamic -h - | FileCheck -check-prefix LEGACY %s

## MIPS against cheriabi
# RUN: not ld.lld -pie -o /dev/null %t-mips.o %t-lib-legacy.so 2>&1 | FileCheck %s -check-prefix ERR-MIPS-LINKING-CHERIABI
# RUN: not ld.lld -pie -o /dev/null %t-mips.o %t-lib-pcrel.so 2>&1 | FileCheck %s -check-prefix ERR-MIPS-LINKING-CHERIABI
# RUN: not ld.lld -pie -o /dev/null %t-mips.o %t-lib-plt.so 2>&1 | FileCheck %s -check-prefix ERR-MIPS-LINKING-CHERIABI
# RUN: not ld.lld -pie -o /dev/null %t-mips.o %t-lib-fn-desc.so 2>&1 | FileCheck %s -check-prefix ERR-MIPS-LINKING-CHERIABI
# RUN: ld.lld -pie -o - %t-lib-mips.so %t-mips.o | llvm-readobj -dynamic -h - | FileCheck -check-prefix MIPS %s


# LEGACY: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_LEGACY RELATIVE_CAPRELOCS
# PCREL: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL RELATIVE_CAPRELOCS
# PLT: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PLT RELATIVE_CAPRELOCS
# FNDESC: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_FNDESC RELATIVE_CAPRELOCS
# MIPS-NOT: MIPS_CHERI_FLAGS

# ERR-CHERIABI-LINKING-MIPS: ld.lld: error: {{.+}}mix-abis-shlib.s.tmp-lib-mips.so: ABI 'n64' is incompatible with target ABI: purecap
# ERR-CHERIABI-LINKING-MIPS-EMPTY:
# ERR-MIPS-LINKING-CHERIABI: ld.lld: error: {{.+}}mix-abis-shlib.s.tmp-lib-mips.so: ABI 'purecap' is incompatible with target ABI: n64
# ERR-MIPS-LINKING-CHERIABI-EMPTY:

# ERR-SHARED: ld.lld: error: target pure-capability ABI EXT_CHERI_ABI_[[IN_ABI]] is incompatible with linked shared library
# ERR-SHARED-NEXT: >>> {{.+}}mix-abis-shlib.s.tmp-lib-{{.+}}.so uses EXT_CHERI_ABI_[[BAD_ABI]]
# ERR-SHARED-EMPTY:
# WARN-SHARED: ld.lld: error: target pure-capability ABI EXT_CHERI_ABI_[[IN_ABI]] is incompatible with linked shared library
# WARN-SHARED-NEXT: >>> {{.+}}mix-abis-shlib.s.tmp-lib-{{.+}}.so uses EXT_CHERI_ABI_[[BAD_ABI]]
# WARN-SHARED-EMPTY:

.global libfunc
.type libfunc,@function


.ifdef LIBRARY
.text
.ent libfunc
libfunc:
    nop
.end libfunc
.else


.text
.global __start
.ent __start
__start:
.ifdef MIPS
    ld $2 %call16(libfunc)($gp)
.else
    clcbi $c1, %capcall20(libfunc)($cgp)
.endif
    nop
.end __start
.endif
