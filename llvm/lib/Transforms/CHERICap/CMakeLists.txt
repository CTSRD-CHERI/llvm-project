add_llvm_component_library(LLVMCHERICapOpts
  FoldCapIntrinsics.cpp
  PureCapABICalls.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
  )
