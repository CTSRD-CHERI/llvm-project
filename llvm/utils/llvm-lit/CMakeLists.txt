get_property(LLVM_LIT_CONFIG_MAP GLOBAL PROPERTY LLVM_LIT_CONFIG_MAP)

get_llvm_lit_path(LIT_BASE_DIR LIT_FILE_NAME)

set(LLVM_SOURCE_DIR ${LLVM_MAIN_SRC_DIR})

function(create_llvm_lit _output_file)
  set(CHERI_CAP_SIZE 16)  # TODO: allow setting a default
  configure_file(
    llvm-lit.in
    ${_output_file}
  )
  set(CHERI_CAP_SIZE 16)
  configure_file(
    llvm-lit.in
    ${_output_file}-cheri128
  )
  set(CHERI_CAP_SIZE 32)
  configure_file(
    llvm-lit.in
    ${_output_file}-cheri256
  )
endfunction()

if(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
  foreach(BUILD_MODE ${CMAKE_CONFIGURATION_TYPES})
    string(REPLACE ${CMAKE_CFG_INTDIR} ${BUILD_MODE} bi ${LIT_BASE_DIR})
    set(bi "${bi}/${LIT_FILE_NAME}")
    create_llvm_lit(${bi})
  endforeach()
else()
  set(BUILD_MODE .)
  create_llvm_lit(${LIT_BASE_DIR}/${LIT_FILE_NAME})
endif()
