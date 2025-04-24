set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_clang_library(clangTidyCheriModule
  CheriTidyModule.cpp
  PtrtointcastCheck.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils

  DEPENDS
  omp_gen
  )

clang_target_link_libraries(clangTidyCheriModule
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  )
