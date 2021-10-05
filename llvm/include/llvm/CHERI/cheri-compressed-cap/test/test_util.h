#include <cinttypes>
#include <iostream>
#include <ostream>
#include <string>

std::ostream& operator<<(std::ostream& os, cc128_length_t value);

std::ostream& operator<<(std::ostream& os, cc128_length_t value) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "{0x%" PRIx64 " %016" PRIx64 "}", (uint64_t)(value >> 64), (uint64_t)(value));
    return os << buffer;
}

static const char* otype_suffix(uint32_t otype) {
    switch (otype) {
#define OTYPE_CASE(Name, ...)                                                                                          \
    case CC128_##Name: return " (CC128_" #Name ")";
        LS_SPECIAL_OTYPES(OTYPE_CASE, )
    default: return "";
    }
}

std::ostream& operator<<(std::ostream& os, const cc128_bounds_bits& value);
std::ostream& operator<<(std::ostream& os, const cc128_bounds_bits& value) {
    os << "{ B: " << (unsigned)value.B << " T: " << (unsigned)value.T << " E: " << (unsigned)value.E
       << " IE: " << (unsigned)value.IE << " }";
    return os;
}
std::ostream& operator<<(std::ostream& os, const cc64_bounds_bits& value);
std::ostream& operator<<(std::ostream& os, const cc64_bounds_bits& value) {
    os << "{ B: " << (unsigned)value.B << " T: " << (unsigned)value.T << " E: " << (unsigned)value.E
       << " IE: " << (unsigned)value.IE << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const cc128_cap_t& value);
std::ostream& operator<<(std::ostream& os, const cc128_cap_t& value) {
    char buffer[4096];
    cc128_length_t top_full = value._cr_top;
    snprintf(buffer, sizeof(buffer),
             "\tPermissions: 0x%" PRIx32 "\n"
             "\tUser Perms:  0x%" PRIx32 "\n"
             "\tBase:        0x%016" PRIx64 "\n"
             "\tOffset:      0x%016" PRIx64 "\n"
             "\tLength:      0x%" PRIx64 "%016" PRIx64 " %s\n"
             "\tTop:         0x%" PRIx64 "%016" PRIx64 " %s\n"
             "\tFlags:       %d\n"
             "\tReserved:    %d\n"
             "\tSealed:      %d\n"
             "\tOType:       0x%" PRIx32 "%s\n",
             value.permissions(), value.software_permissions(), value.base(), (uint64_t)value.offset(),
             (uint64_t)(value.length() >> 64), (uint64_t)value.length(),
             value.length() > UINT64_MAX ? " (greater than UINT64_MAX)" : "", (uint64_t)(top_full >> 64),
             (uint64_t)top_full, top_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "", (int)value.flags(),
             (int)value.reserved_bits(), (int)(value.is_sealed()), value.type(), otype_suffix(value.type()));
    os << "{\n" << buffer << "}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const cc64_cap_t& value);
std::ostream& operator<<(std::ostream& os, const cc64_cap_t& value) {
    char buffer[4096];
    snprintf(buffer, sizeof(buffer),
             "\tPermissions: 0x%" PRIx32 "\n"
             "\tUser Perms:  0x%" PRIx32 "\n"
             "\tBase:        0x%08" PRIx32 "\n"
             "\tOffset:      0x%08" PRIx32 "\n"
             "\tLength:      0x%08" PRIx64 " %s\n"
             "\tTop:         0x%08" PRIx64 " %s\n"
             "\tFlags:       %d\n"
             "\tReserved:    %d\n"
             "\tSealed:      %d\n"
             "\tOType:       0x%" PRIx32 "%s\n",
             value.permissions(), value.software_permissions(), value.base(), (uint32_t)value.offset(),
             (uint64_t)value.length(), value.length() > UINT32_MAX ? " (greater than UINT32_MAX)" : "",
             (uint64_t)value.top(), value.top() > UINT32_MAX ? " (greater than UINT32_MAX)" : "", (int)value.flags(),
             (int)value.reserved_bits(), (int)(value.is_sealed()), value.type(), otype_suffix(value.type()));
    os << "{\n" << buffer << "}";
    return os;
}

static inline bool operator==(const cc128_bounds_bits& a, const cc128_bounds_bits& b) {
    return a.B == b.B && a.E == b.E && a.T == b.T && a.IE == b.IE;
}
static inline bool operator==(const cc64_bounds_bits& a, const cc64_bounds_bits& b) {
    return a.B == b.B && a.E == b.E && a.T == b.T && a.IE == b.IE;
}

#include "catch.hpp"

static bool failed = false;

template <typename T> static inline bool check(T expected, T actual, const std::string& msg) {
    if (expected == actual)
        return true;
    std::cerr << "ERROR: " << msg << ": expected 0x" << std::hex << expected << " != 0x" << actual << "\n";
    failed = true;
    return false;
}

template <class T, std::size_t N> constexpr inline size_t array_lengthof(T (&)[N]) { return N; }

template <class Cap> static void dump_cap_fields(const Cap& result) {
    fprintf(stderr, "Permissions: 0x%" PRIx32 "\n", result.permissions()); // TODO: decode perms
    fprintf(stderr, "User Perms:  0x%" PRIx32 "\n", result.software_permissions());
    fprintf(stderr, "Base:        0x%016" PRIx64 "\n", (uint64_t)result.base());
    fprintf(stderr, "Offset:      0x%016" PRIx64 "\n", (uint64_t)result.offset());
    fprintf(stderr, "Cursor:      0x%016" PRIx64 "\n", (uint64_t)result.address());
    cc128_length_t len_full = result.length();
    fprintf(stderr, "Length:      0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(len_full >> 64), (uint64_t)len_full,
            len_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    cc128_length_t top_full = result.top();
    fprintf(stderr, "Top:         0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(top_full >> 64), (uint64_t)top_full,
            top_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    fprintf(stderr, "Flags:       %d\n", (int)result.flags());
    fprintf(stderr, "Reserved:    %d\n", (int)result.reserved_bits());
    fprintf(stderr, "Sealed:      %d\n", (int)result.is_sealed());
    fprintf(stderr, "OType:       0x%" PRIx32 "%s\n", result.type(), otype_suffix(result.type()));
    fprintf(stderr, "\n");
}

__attribute__((used)) static cc128_cap_t decompress_representable(uint64_t pesbt_already_xored, uint64_t cursor) {
    cc128_cap_t result;
    printf("Decompressing pesbt = %016" PRIx64 ", cursor = %016" PRIx64 "\n", pesbt_already_xored, cursor);
    cc128_decompress_raw(pesbt_already_xored, cursor, false, &result);
    dump_cap_fields(result);
    // Check that the result is the same again when compressed
    uint64_t new_pesbt_already_xored = cc128_compress_raw(&result);
    CHECK(pesbt_already_xored == new_pesbt_already_xored);
    CHECK(cursor == result.address());
    return result;
}

inline cc128_cap_t make_max_perms_cap(uint64_t base, uint64_t offset, cc128_length_t length) {
    return cc128_make_max_perms_cap(base, offset, length);
}

#define DO_STRINGIFY2(x) #x
#define DO_STRINGIFY1(x) DO_STRINGIFY2(x)
#define STRINGIFY(x) DO_STRINGIFY1(x)

#define CHECK_FIELD_RAW(value, expected) CHECK(value == expected)
#define CHECK_FIELD(cap, field, expected) CHECK((uint64_t)expected == cap.field())

enum {
#ifdef CC128_OLD_FORMAT
    TESTING_OLD_FORMAT = 1
#else
    TESTING_OLD_FORMAT = 0
#endif
};
