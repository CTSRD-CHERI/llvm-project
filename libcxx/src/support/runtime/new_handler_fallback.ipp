// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace std {

_LIBCPP_SAFE_STATIC static std::new_handler __new_handler;

new_handler
set_new_handler(new_handler handler) _NOEXCEPT
{
    return __atomic_exchange_n(&__new_handler, handler, __ATOMIC_SEQ_CST);
}

new_handler
get_new_handler() _NOEXCEPT
{
    return __atomic_load_n(&__new_handler, __ATOMIC_SEQ_CST);
}

} // namespace std
