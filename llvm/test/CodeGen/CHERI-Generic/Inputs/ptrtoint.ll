;; Check that we can correctly generate code for ptrtoint and perform simple folds
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s | FileCheck %s
; RUN: llc @HYBRID_HARDFLOAT_ARGS@ < %s | FileCheck %s --check-prefix=HYBRID

define internal iCAPRANGE @ptrtoint(i8 addrspace(200)* %cap) addrspace(200) nounwind {
  %ret = ptrtoint i8 addrspace(200)* %cap to iCAPRANGE
  ret iCAPRANGE %ret
}

define internal iCAPRANGE @ptrtoint_plus_const(i8 addrspace(200)* %cap) addrspace(200) nounwind {
  %zero = ptrtoint i8 addrspace(200)* %cap to iCAPRANGE
  %ret = add iCAPRANGE %zero, 2
  ret iCAPRANGE %ret
}

define internal iCAPRANGE @ptrtoint_plus_var(i8 addrspace(200)* %cap, iCAPRANGE %add) addrspace(200) nounwind {
  %zero = ptrtoint i8 addrspace(200)* %cap to iCAPRANGE
  %ret = add iCAPRANGE %zero, %add
  ret iCAPRANGE %ret
}

define internal iCAPRANGE @ptrtoint_null() addrspace(200) nounwind {
  %ret = ptrtoint i8 addrspace(200)* null to iCAPRANGE
  ret iCAPRANGE %ret
}

define internal iCAPRANGE @ptrtoint_null_plus_const() addrspace(200) nounwind {
  %zero = ptrtoint i8 addrspace(200)* null to iCAPRANGE
  %ret = add iCAPRANGE %zero, 2
  ret iCAPRANGE %ret
}

define internal iCAPRANGE @ptrtoint_null_plus_var(iCAPRANGE %add) addrspace(200) nounwind {
  %zero = ptrtoint i8 addrspace(200)* null to iCAPRANGE
  %ret = add iCAPRANGE %zero, %add
  ret iCAPRANGE %ret
}
