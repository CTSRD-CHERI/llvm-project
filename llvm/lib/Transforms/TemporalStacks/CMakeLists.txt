
add_llvm_library( LLVMCHERITemporalOpts
  TemporalStackPass.cpp
  EscapeAnalysis/EscapeAnalysis.cpp
  CheriSetBounds.cpp
)

add_dependencies(LLVMCHERITemporalOpts intrinsics_gen)
