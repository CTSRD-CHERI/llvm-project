// RUN: %clang -target mips64-none-elf %s -o %t -### 2>&1 | FileCheck %s -check-prefixes CHECK,NOSYSROOT
// RUN: %clang --sysroot=/foo/bar -target mips64-none-elf %s -o %t -### 2>&1 | FileCheck %s -check-prefixes CHECK,SYSROOT
// CHERI triple without ABI should be the same:
// RUN: %clang -target cheri-none-elf %s -o %t -### 2>&1 | FileCheck %s -check-prefixes CHECK,NOSYSROOT

// C++ has some more libraries and include dirs:
// RUN: %clangxx --sysroot=/foo/bar -xc++ -target mips64-none-elf %s -o %t -### 2>&1 | FileCheck %s -check-prefixes CHECK,SYSROOT,CXX
// RUN: %clang -target cheri-none-elf -fPIC -mabi=purecap %s -o %t -### 2>&1 | FileCheck %s -check-prefixes CHECK,NOSYSROOT-CHERIABI
// RUN: %clangxx -xc++ -target cheri-none-elf -fPIC -mabi=purecap %s -o %t -### 2>&1 | FileCheck %s -check-prefixes CHECK,NOSYSROOT-CHERIABI


// CHECK: "-cc1" "-triple" "{{(mips64|cheri)}}-none--elf" "-emit-obj"
// CHECK-SAME: "-no-integrated-as"
// CHECK-SAME: "-target-abi" "[[ABI:(n64|purecap)]]"
// CHECK-SAME: "-resource-dir" "[[RESOURCE_DIR:[^"]+]]"
// SYSROOT-SAME: "-isysroot" "[[SYSROOT:[^"]+]]"
// CXX-SAME: "-internal-isystem" "[[SYSROOT]]/include/c++/v1"
// CHECK-SAME: "-internal-isystem" "[[RESOURCE_DIR]]/include"
// SYSROOT-SAME: "-internal-isystem" "[[SYSROOT]]/include"
// NOSYSROOT-SAME: "-internal-isystem" "include"

// CHECK: "{{.*}}ld.lld" "{{.+}}.o" "-Bstatic"
// SYSROOT-SAME:   "-L[[SYSROOT]]/lib"
// NOSYSROOT-SAME: "-L[[RESOURCE_DIR]]/lib"
// NOSYSROOT-CHERIABI-SAME: "-L[[RESOURCE_DIR]]/libcheri"
// CXX-SAME: "-lc++" "-lcxxrt" "-lunwind"
// CHECK-SAME: "-lc" "-lm" "-o" "{{.+}}/mips-baremetal.c.tmp"
