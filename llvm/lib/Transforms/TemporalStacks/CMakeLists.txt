
add_llvm_component_library( LLVMCHERITemporalOpts
  TemporalStackPass.cpp
  EscapeAnalysis/EscapeAnalysis.cpp

  DEPENDS
  intrinsics_gen

  COMPONENT_NAME
  CHERITemporal
)
