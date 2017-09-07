add_llvm_library(LLVMCHERICapOpts
  FoldCapIntrinsics.cpp
  PureCapABICalls.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  )

add_dependencies(LLVMCHERICapOpts intrinsics_gen)
