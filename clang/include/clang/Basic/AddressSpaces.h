//===--- AddressSpaces.h - Language-specific address spaces -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Provides definitions for the various language-specific address
/// spaces.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_ADDRESSSPACES_H
#define LLVM_CLANG_BASIC_ADDRESSSPACES_H

namespace clang {

namespace LangAS {

/// \brief Defines the address space values used by the address space qualifier
/// of QualType.
///
enum class ID {
  // The default value 0 is the value used in QualType for the the situation
  // where there is no address space qualifier. For most languages, this also
  // corresponds to the situation where there is no address space qualifier in
  // the source code, except for OpenCL, where the address space value 0 in
  // QualType represents private address space in OpenCL source code.
  Default = 0,

  // OpenCL specific address spaces.
  opencl_global,
  opencl_local,
  opencl_constant,
  opencl_generic,

  // CUDA specific address spaces.
  cuda_device,
  cuda_constant,
  cuda_shared,

  // This denotes the count of language-specific address spaces and also
  // the offset added to the target-specific address spaces, which are usually
  // specified by address space attributes __attribute__(address_space(n))).
  FirstTargetAddressSpace,

  // Currently CHERI TLS variables need to be in AS0 otherwise we get Cannot
  // select errors
  // This is hack to force CodeGen to use AS0 even if default == AS200
  cheri_tls = FirstTargetAddressSpace + 0xdead,
  // TODO: should we add this:
  // cheri_capability = FirstTargetAddressSpace + 200,
};

constexpr ID Default = ID::Default;
constexpr ID opencl_global = ID::opencl_global;
constexpr ID opencl_local = ID::opencl_local;
constexpr ID opencl_constant = ID::opencl_constant;
constexpr ID opencl_generic = ID::opencl_generic;
constexpr ID cuda_device = ID::cuda_device;
constexpr ID cuda_constant = ID::cuda_constant;
constexpr ID cuda_shared = ID::cuda_shared;
constexpr ID FirstTargetAddressSpace = ID::FirstTargetAddressSpace;
constexpr ID cheri_tls = ID::cheri_tls;

/// The type of a lookup table which maps from language-specific address spaces
/// to target-specific ones.
typedef unsigned Map[(int)ID::FirstTargetAddressSpace];
} // namespace LangAS
}

inline bool operator>=(clang::LangAS::ID LHS, clang::LangAS::ID RHS) {
  return static_cast<unsigned>(LHS) >= static_cast<unsigned>(RHS);
}
inline bool operator==(clang::LangAS::ID LHS, clang::LangAS::ID RHS) {
  return static_cast<unsigned>(LHS) == static_cast<unsigned>(RHS);
}
inline bool operator!=(clang::LangAS::ID LHS, clang::LangAS::ID RHS) {
  return static_cast<unsigned>(LHS) != static_cast<unsigned>(RHS);
}
inline bool operator<=(clang::LangAS::ID LHS, clang::LangAS::ID RHS) {
  return static_cast<unsigned>(LHS) <= static_cast<unsigned>(RHS);
}
inline clang::LangAS::ID operator+(clang::LangAS::ID LHS, unsigned RHS) {
  return static_cast<clang::LangAS::ID>(static_cast<unsigned>(LHS) + RHS);
}
inline clang::LangAS::ID operator+(unsigned LHS, clang::LangAS::ID RHS) {
  return static_cast<clang::LangAS::ID>(LHS + static_cast<unsigned>(RHS));
}

#endif
