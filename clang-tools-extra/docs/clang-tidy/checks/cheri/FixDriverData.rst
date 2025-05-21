.. title:: clang-tidy - cheri-FixDriverData

cheri-FixDriverData
===================

Static initializations of driver data fields (fields name "driver_data"
or "driver_info") should use a cast to uintptr_t instead of some
variant of unsigned long.
