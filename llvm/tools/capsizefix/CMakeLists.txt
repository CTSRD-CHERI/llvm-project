set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  CodeGen
  MC
  Object
  Support
  )

add_llvm_tool(capsizefix
  sizefix.cpp
  )
