#include <cinttypes>
#include <iostream>
#include <ostream>
#include <string>

#define CC_FORMAT_LOWER TEST_CC_FORMAT_LOWER
#define CC_FORMAT_UPPER TEST_CC_FORMAT_UPPER

#define DO_STRINGIFY2(x) #x
#define DO_STRINGIFY1(x) DO_STRINGIFY2(x)
#define STRINGIFY(x) DO_STRINGIFY1(x)

static const char* otype_suffix(uint32_t otype) {
    // clang-format off
    switch (otype) {
#define OTYPE_CASE(Name, ...)                                                                                          \
    case _CC_N(Name): return " (" STRINGIFY(_CC_N(Name)) ")";
        _CC_N(LS_SPECIAL_OTYPES)(OTYPE_CASE, )
    default: return "";
#undef OTYPE_CASE
    }
    // clang-format on
}

#ifndef TEST_CC_IS_CHERI256
std::ostream& operator<<(std::ostream& os, const _cc_bounds_bits& value);
std::ostream& operator<<(std::ostream& os, const _cc_bounds_bits& value) {
    os << "{ B: " << (unsigned)value.B << " T: " << (unsigned)value.T << " E: " << (unsigned)value.E
       << " IE: " << (unsigned)value.IE << " }";
    return os;
}

static inline bool operator==(const _cc_bounds_bits& a, const _cc_bounds_bits& b) {
    return a.B == b.B && a.E == b.E && a.T == b.T && a.IE == b.IE;
}
#endif

static bool failed = false;

template <typename T> static inline bool check(T expected, T actual, const std::string& msg) {
    if (expected == actual)
        return true;
    std::cerr << "ERROR: " << msg << ": expected 0x" << std::hex << expected << " != 0x" << actual << "\n";
    failed = true;
    return false;
}

template <class T, std::size_t N> constexpr inline size_t array_lengthof(T (&)[N]) { return N; }

template <class Cap> static void dump_cap_fields(FILE* f, const Cap& result) {
    fprintf(f, "Base:        %#016" PRIx64 "\n", (uint64_t)result.base());
    fprintf(f, "Cursor:      %#016" PRIx64 "    Offset: %" PRIx64 "\n", (uint64_t)result.address(),
            (uint64_t)result.offset());
    unsigned __int128 top_full = result.top();
    fprintf(f, "Top:         0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(top_full >> 64), (uint64_t)top_full,
            top_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    unsigned __int128 len_full = result.length();
    fprintf(f, "Length:      0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(len_full >> 64), (uint64_t)len_full,
            len_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
#ifndef TEST_CC_IS_CHERI256
    fprintf(f, "PESBT:       %#016" PRIx64 "  Exponent: %d\n", (uint64_t)result.cr_pesbt, result.cr_exp);
#endif
    fprintf(f, "Tag:         %d  Permissions: %#" PRIx32 "  User Perms: %#" PRIx32 "\n", result.cr_tag,
            result.permissions(), result.software_permissions());
    fprintf(f, "Flags:       %d  Reserved: %d  Sealed: %d  OType: %#" PRIx32 "%s", (int)result.flags(),
            (int)result.reserved_bits(), (int)result.is_sealed(), result.type(), otype_suffix(result.type()));
}

std::ostream& operator<<(std::ostream& os, const _cc_cap_t& value);
std::ostream& operator<<(std::ostream& os, const _cc_cap_t& value) {
    char buf[1024];
    FILE* f = fmemopen(buf, sizeof(buf), "w+");
    dump_cap_fields(f, value);
    fclose(f);
    return os << buf;
}

std::ostream& operator<<(std::ostream& os, const cc128_length_t& value);
std::ostream& operator<<(std::ostream& os, const cc128_length_t& value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "0x%" PRIx64 "%016" PRIx64, (uint64_t)(value >> 64), (uint64_t)value);
    return os << buf;
}

#define CHECK_FIELD_RAW(value, expected) CHECK(value == expected)
#define CHECK_FIELD(cap, field, expected) CHECK((uint64_t)expected == cap.field())
