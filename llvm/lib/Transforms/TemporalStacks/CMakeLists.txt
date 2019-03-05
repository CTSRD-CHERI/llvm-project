
add_llvm_loadable_module( libTemporal
  TemporalStackPass.cpp
  EscapeAnalysis/EscapeAnalysis.cpp
  CheriSetBounds.cpp
  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
