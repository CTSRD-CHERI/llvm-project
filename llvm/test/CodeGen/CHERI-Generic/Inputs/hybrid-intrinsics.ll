; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -o - < %s | FileCheck %s --check-prefix=HYBRID

declare iCAPRANGE @llvm.cheri.cap.flags.get.iCAPRANGE(ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.flags.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE)

define iCAPRANGE @flags_get(ptr addrspace(200) %cap) nounwind {
  %flags = call iCAPRANGE @llvm.cheri.cap.flags.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %flags
}

define ptr addrspace(200) @flags_set(ptr addrspace(200) %cap, iCAPRANGE %flags) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.flags.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %flags)
  ret ptr addrspace(200) %newcap
}
