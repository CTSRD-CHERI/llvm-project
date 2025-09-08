//===- compressed_cap_utils.h -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_COMPRESSED_CAP_UTILS
#define LLVM_COMPRESSED_CAP_UTILS
#include <llvm/CHERI/cheri-compressed-cap/cheri_compressed_cap.h>
#include <type_traits>

namespace llvm::cc {
template <bool Is64Bit>
using CapTy = std::conditional_t<Is64Bit, cc128r_cap_t, cc64r_cap_t>;
template <bool Is64Bit>
using AddrTy = std::conditional_t<Is64Bit, cc128r_addr_t, cc64r_addr_t>;
template <bool Is64Bit>
using LengthTy = std::conditional_t<Is64Bit, cc128r_length_t, cc64r_length_t>;

template <bool Is64Bit>
constexpr AddrTy<Is64Bit> getRepresentableLength(AddrTy<Is64Bit> Addr) {
  if constexpr (Is64Bit) {
    return cc128r_get_representable_length(Addr);
  } else {
    return cc64r_get_representable_length(Addr);
  }
}

template <bool Is64Bit>
constexpr CapTy<Is64Bit> makeMaxPermCap(AddrTy<Is64Bit> Addr,
                                        AddrTy<Is64Bit> Cursor,
                                        LengthTy<Is64Bit> Top) {
  if constexpr (Is64Bit) {
    return cc128r_make_max_perms_cap(Addr, Cursor, Top);
  } else {
    return cc64r_make_max_perms_cap(Addr, Cursor, Top);
  }
}

template <bool Is64Bit>
constexpr CapTy<Is64Bit>
makeMaxPermCapMLV(AddrTy<Is64Bit> Addr, AddrTy<Is64Bit> Cursor,
                  LengthTy<Is64Bit> Top, bool Mode, uint8_t LVBits) {
  if constexpr (Is64Bit) {
    return cc128r_make_max_perms_cap_m_lv(Addr, Cursor, Top, Mode, LVBits);
  } else {
    return cc64r_make_max_perms_cap_m_lv(Addr, Cursor, Top, Mode, LVBits);
  }
}

template <bool Is64Bit> constexpr void compressMem(CapTy<Is64Bit> *Csp) {
  if constexpr (Is64Bit) {
    cc128r_compress_mem(Csp);
  } else {
    cc64r_compress_mem(Csp);
  }
}

template <bool Is64Bit> constexpr void modeApCompress(CapTy<Is64Bit> *Csp) {
  if constexpr (Is64Bit) {
    cc128r_m_ap_compress(Csp);
  } else {
    cc64r_m_ap_compress(Csp);
  }
}

template <bool Is64Bit>
constexpr void updateCT(CapTy<Is64Bit> *Csp, uint8_t Val) {
  if constexpr (Is64Bit) {
    cc128r_update_ct(Csp, Val);
  } else {
    cc64r_update_ct(Csp, Val);
  }
}

template <bool Is64Bit>
constexpr void decompressMem(uint64_t Pesbt, uint64_t Cursor, bool Tag,
                             uint8_t LVBits, CapTy<Is64Bit> *Csp) {
  if constexpr (Is64Bit) {
    cc128r_decompress_raw__(Pesbt, Cursor, Tag, LVBits, Csp);
  } else {
    cc64r_decompress_raw__(Pesbt, Cursor, Tag, LVBits, Csp);
  }
}
} // namespace llvm::cc
#endif // LLVM_COMPRESSED_CAP_UTILS
