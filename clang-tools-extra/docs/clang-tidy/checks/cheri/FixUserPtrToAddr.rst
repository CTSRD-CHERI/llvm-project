.. title:: clang-tidy - cheri-FixUserPtrToAddr

cheri-FixUserPtrToAddr
======================

In the CHERI Linux Kernel __c_ua() must be used to convert
a kernel pointer to an address while __c_ua_u() must be used
for user pointers. Check that this is consistently done even
if the current configuration does the same thing for both.

