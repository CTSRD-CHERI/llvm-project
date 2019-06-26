#include <iostream>
#include <ostream>
#include <string>

std::ostream& operator<<(std::ostream& os, unsigned __int128 value);

std::ostream& operator<<(std::ostream& os, unsigned __int128 value) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "{0x%" PRIx64 " %016" PRIx64 "}", (uint64_t)(value >> 64), (uint64_t)(value));
    return os << buffer;
}

static const char* otype_suffix(uint32_t otype) {
    // Two separate switches since if the number of otype bits is the same
    // we cannot have both of the case statements in one switch
    switch (otype) {
    case CC128_OTYPE_UNSEALED:
        return " (CC128_OTYPE_UNSEALED)";
    case CC128_OTYPE_SENTRY:
        return " (CC128_OTYPE_SENTRY)";
    case CC128_OTYPE_RESERVED2:
        return " (CC128_OTYPE_RESERVED2)";
    case CC128_OTYPE_RESERVED3:
        return " (CC128_OTYPE_RESERVED3)";
    default:
        break;
    }
    switch (otype) {
    case CC256_OTYPE_UNSEALED:
        return " (CC256_OTYPE_UNSEALED)";
    case CC256_OTYPE_SENTRY:
        return " (CC256_OTYPE_SENTRY)";
    case CC256_OTYPE_RESERVED2:
        return " (CC256_OTYPE_RESERVED2)";
    case CC256_OTYPE_RESERVED3:
        return " (CC256_OTYPE_RESERVED3)";
    default:
        return "";
    }
}

std::ostream& operator<<(std::ostream& os, const cap_register_t& value);

std::ostream& operator<<(std::ostream& os, const cap_register_t& value) {
    char buffer[4096];
    unsigned __int128 top_full = value.cr_base + value._cr_length;
    snprintf(buffer, sizeof(buffer),
             "\tPermissions: 0x%" PRIx32 "\n"
             "\tUser Perms:  0x%" PRIx32 "\n"
             "\tBase:        0x%016" PRIx64 "\n"
             "\tOffset:      0x%016" PRIx64 "\n"
             "\tLength:      0x%" PRIx64 "%016" PRIx64 " %s\n"
             "\tTop:         0x%" PRIx64 "%016" PRIx64 " %s\n"
             "\tSealed:      %d\n"
             "\tOType:       0x%" PRIx32 "%s\n",
             value.cr_perms, value.cr_uperms, value.cr_base, value.cr_offset, (uint64_t)(value._cr_length >> 64),
             (uint64_t)value._cr_length, value._cr_length > UINT64_MAX ? " (greater than UINT64_MAX)" : "",
             (uint64_t)(top_full >> 64), (uint64_t)top_full, top_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "",
             (int)cc128_is_cap_sealed(&value), value.cr_otype, otype_suffix(value.cr_otype));
    os << "{\n" << buffer << "}";
    return os;
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

static void dump_cap_fields(const cap_register_t& result) {
    fprintf(stderr, "Permissions: 0x%" PRIx32 "\n", result.cr_perms); // TODO: decode perms
    fprintf(stderr, "User Perms:  0x%" PRIx32 "\n", result.cr_uperms);
    fprintf(stderr, "Base:        0x%016" PRIx64 "\n", result.cr_base);
    fprintf(stderr, "Offset:      0x%016" PRIx64 "\n", result.cr_offset);
    fprintf(stderr, "Length:      0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(result._cr_length >> 64),
            (uint64_t)result._cr_length, result._cr_length > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    unsigned __int128 top_full = result.cr_base + result._cr_length;
    fprintf(stderr, "Top:         0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(top_full >> 64), (uint64_t)top_full,
            top_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    fprintf(stderr, "Sealed:      %d\n", (int)cc128_is_cap_sealed(&result));
    fprintf(stderr, "OType:       0x%" PRIx32 "%s\n", result.cr_otype, otype_suffix(result.cr_otype));
    fprintf(stderr, "\n");
}

__attribute__((used)) static cap_register_t decompress_representable(uint64_t pesbt_already_xored, uint64_t cursor) {
    cap_register_t result;
    printf("Decompressing pesbt = %016" PRIx64 ", cursor = %016" PRIx64 "\n", pesbt_already_xored, cursor);
    decompress_128cap_already_xored(pesbt_already_xored, cursor, &result);
    dump_cap_fields(result);
    // Check that the result is the same again when compressed
    uint64_t new_pesbt_already_xored = compress_128cap_without_xor(&result);
    CHECK(pesbt_already_xored == new_pesbt_already_xored);
    CHECK(cursor  == result.cr_base + result.cr_offset);
    return result;
}

inline cap_register_t make_max_perms_cap(uint64_t base, uint64_t offset, unsigned __int128 length) {
    cap_register_t creg;
    memset(&creg, 0, sizeof(creg));
    creg.cr_base = base;
    creg.cr_offset = offset;
    creg._cr_length = length;
    creg.cr_perms = CC128_PERMS_ALL;
    creg.cr_uperms = CC128_UPERMS_ALL;
    creg.cr_otype = CC128_OTYPE_UNSEALED;
    creg.cr_tag = true;
    CHECK(cc128_is_representable(false, base, length, offset, offset));
    return creg;
}

#define DO_STRINGIFY2(x) #x
#define DO_STRINGIFY1(x) DO_STRINGIFY2(x)
#define STRINGIFY(x) DO_STRINGIFY1(x)

#define CHECK_FIELD_RAW(value, expected) CHECK(value == expected)
#define CHECK_FIELD(cap, field, expected) CHECK((uint64_t)expected == (uint64_t)cap.cr_##field)

enum {
#ifdef CC128_OLD_FORMAT
    TESTING_OLD_FORMAT = 1
#else
    TESTING_OLD_FORMAT = 0
#endif
};
