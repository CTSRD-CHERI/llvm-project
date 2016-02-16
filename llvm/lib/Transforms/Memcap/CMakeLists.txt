add_llvm_library(LLVMMemCapOpts
  FoldCapIntrinsics.cpp
  PureCapABICalls.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  )

add_dependencies(LLVMMemCapOpts intrinsics_gen)
