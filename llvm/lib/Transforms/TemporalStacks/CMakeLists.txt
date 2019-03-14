
add_llvm_library( LLVMCHERITemporalOpts
  TemporalStackPass.cpp
  EscapeAnalysis/EscapeAnalysis.cpp
)

add_dependencies(LLVMCHERITemporalOpts intrinsics_gen)
