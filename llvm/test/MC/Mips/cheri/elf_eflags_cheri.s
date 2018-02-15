# RUN: %cheri256_llvm-mc -filetype=obj %s -o -| llvm-readobj -h | FileCheck --check-prefix=CHERI256-NO-ABI %s
# CHERI256-NO-ABI: Flags [ (0x30C20004)
# RUN: %cheri128_llvm-mc -filetype=obj %s -o -| llvm-readobj -h | FileCheck --check-prefix=CHERI128-NO-ABI %s
# CHERI128-NO-ABI: Flags [ (0x30C10004)
# RUN: %cheri256_llvm-mc -filetype=obj -target-abi n64 %s -o -| llvm-readobj -h | FileCheck --check-prefix=CHERI256-N64 %s
# CHERI256-N64: Flags [ (0x30C20004)
# RUN: %cheri128_llvm-mc -filetype=obj -target-abi n64 %s -o -| llvm-readobj -h | FileCheck --check-prefix=CHERI128-N64 %s
# CHERI128-N64: Flags [ (0x30C10004)
# RUN: %cheri256_purecap_llvm-mc -filetype=obj %s -o -| llvm-readobj -h | FileCheck --check-prefix=CHERI256-SANDBOX %s
# CHERI256-SANDBOX: Flags [ (0x30C2C006)
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o -| llvm-readobj -h | FileCheck --check-prefix=CHERI128-SANDBOX %s
# CHERI128-SANDBOX: Flags [ (0x30C1C006)
