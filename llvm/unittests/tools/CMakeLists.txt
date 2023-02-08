if(LLVM_TARGETS_TO_BUILD MATCHES "X86")
  add_subdirectory(
    llvm-cfi-verify
  )
endif()

if (TARGET llvm-exegesis)
  add_subdirectory(llvm-exegesis)
endif()

add_subdirectory(llvm-profdata)

add_subdirectory(llvm-profgen)
if (TARGET llvm-mca)
  add_subdirectory(llvm-mca)
endif()
