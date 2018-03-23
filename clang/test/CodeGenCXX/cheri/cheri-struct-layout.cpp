// RUN: %cheri_cc1 -fno-rtti -std=c++14 -target-abi purecap -fsyntax-only -fdump-record-layouts %s -emit-llvm -o /dev/null | %cheri_FileCheck %s
// CHECK:      *** Dumping AST Record Layout
// CHECK-NEXT:         0 | class error_category
// CHECK-NEXT:         0 |   (error_category vtable pointer)
// CHECK-NEXT:           | [sizeof=[[$CAP_SIZE]],
// CHECK-SAME:              dsize=[[$CAP_SIZE]], align=[[$CAP_SIZE]],
// CHECK-NEXT:           |  nvsize=[[$CAP_SIZE]], nvalign=[[$CAP_SIZE]]]

// CHECK:      *** Dumping AST Record Layout
// CHECK-NEXT:          0 | class __do_message
// CHECK-NEXT:          0 |   class error_category (primary base)
// CHECK-NEXT:          0 |     (error_category vtable pointer)
// CHECK-NEXT:            | [sizeof=[[$CAP_SIZE]], dsize=[[$CAP_SIZE]], align=[[$CAP_SIZE]],
// CHECK-NEXT:            |  nvsize=[[$CAP_SIZE]], nvalign=[[$CAP_SIZE]]]


// CHECK:      *** Dumping AST Record Layout
// CHECK-NEXT:          0 | class __future_error_category
// CHECK-NEXT:          0 |   class __do_message (primary base)
// CHECK-NEXT:          0 |     class error_category (primary base)
// CHECK-NEXT:          0 |       (error_category vtable pointer)
// CHECK-NEXT:            | [sizeof=[[$CAP_SIZE]], dsize=[[$CAP_SIZE]], align=[[$CAP_SIZE]],
// CHECK-NEXT:            |  nvsize=[[$CAP_SIZE]], nvalign=[[$CAP_SIZE]]]


#define _LIBCPP_ALWAYS_INLINE     __attribute__ ((__always_inline__))
#define _LIBCPP_HIDDEN            __attribute__ ((__visibility__("hidden")))
#define _LIBCPP_TYPE_VIS __attribute__ ((__type_visibility__("default")))
#define _LIBCPP_FUNC_VIS __attribute__ ((__visibility__("default")))
#define _LIBCPP_CONSTEXPR_AFTER_CXX11 constexpr
#define _NOEXCEPT noexcept
#define _NOEXCEPT_(x) noexcept(x)
#define _LIBCPP_DEFAULT = default;

class string;
class error_code;
class error_condition;


class _LIBCPP_HIDDEN __do_message;

class _LIBCPP_TYPE_VIS error_category
{
public:
    virtual ~error_category() _NOEXCEPT;

#if defined(_LIBCPP_BUILDING_SYSTEM_ERROR) && \
    defined(_LIBCPP_DEPRECATED_ABI_LEGACY_LIBRARY_DEFINITIONS_FOR_INLINE_FUNCTIONS)
    error_category() _NOEXCEPT;
#else
    _LIBCPP_ALWAYS_INLINE
    _LIBCPP_CONSTEXPR_AFTER_CXX11 error_category() _NOEXCEPT _LIBCPP_DEFAULT
#endif
private:
    error_category(const error_category&);// = delete;
    error_category& operator=(const error_category&);// = delete;

public:
    virtual const char* name() const _NOEXCEPT = 0;
    virtual error_condition default_error_condition(int __ev) const _NOEXCEPT;
    virtual bool equivalent(int __code, const error_condition& __condition) const _NOEXCEPT;
    virtual bool equivalent(const error_code& __code, int __condition) const _NOEXCEPT;
    virtual string message(int __ev) const = 0;

    _LIBCPP_ALWAYS_INLINE
    bool operator==(const error_category& __rhs) const _NOEXCEPT {return this == &__rhs;}

    _LIBCPP_ALWAYS_INLINE
    bool operator!=(const error_category& __rhs) const _NOEXCEPT {return !(*this == __rhs);}

    _LIBCPP_ALWAYS_INLINE
    bool operator< (const error_category& __rhs) const _NOEXCEPT {return this < &__rhs;}

    friend class _LIBCPP_HIDDEN __do_message;
};


class _LIBCPP_HIDDEN __do_message
    : public error_category
{
public:
    virtual string message(int ev) const;
};

class _LIBCPP_HIDDEN __future_error_category
    : public __do_message
{
public:
    virtual const char* name() const _NOEXCEPT;
    virtual string message(int ev) const;
};

_LIBCPP_FUNC_VIS
const error_category& future_category() _NOEXCEPT;

const error_category&
future_category() _NOEXCEPT
{
    static __future_error_category __f;
    return __f;
}

int main() {
  // FIXME this crashes clang?
  // return &future_category() != (void*)0x12345667;
  (void)future_category();
  return 1;
}
