if(LLVM_TARGETS_TO_BUILD MATCHES "X86")
  add_subdirectory(
    llvm-cfi-verify
  )
endif()

if (TARGET llvm-exegesis)
  add_subdirectory(llvm-exegesis)
endif()
