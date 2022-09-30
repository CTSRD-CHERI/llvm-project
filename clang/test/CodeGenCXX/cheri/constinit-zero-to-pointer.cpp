// RUN: %clang_cc1 -triple x86_64-unknown-unknown %s -emit-llvm -o - | FileCheck %s --check-prefix=X86
// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -o - -verify=purecap | FileCheck %s --check-prefix=PURECAP
/// The CHERI logic added to check whether constant-evaluated pointer
/// expressions are NULL-derived or not, was assuming any non-__intcap
/// source expression had to be an integer and did not consider enumerations.
/// This caused build failures (even for non-CHERI targets) in Chromium.
/// Check that we accept a cast from enum to void* in variable initialization.

enum class XFA_AttributeValue { None };
struct AttributeData {
  void *default_value;
};

// NB: this must be const, no constexpr since the enum to void*
// cast is not permitted for constexpr/constinit intializers.
const AttributeData kBarcodeAttributeData{
    (void *)XFA_AttributeValue::None
    // purecap-warning@-1{{cast from provenance-free integer type to pointer type}}
};
// X86: @_ZL21kBarcodeAttributeData = internal constant %struct.AttributeData zeroinitializer, align 8
// PURECAP: @_ZL21kBarcodeAttributeData = internal addrspace(200) constant %struct.AttributeData zeroinitializer, align 16

// Use the constant to ensure initializer is emitted.
AttributeData test() { return kBarcodeAttributeData; }

/// Also check enums with underlying type __intcap for CHERI targets:
#if __has_feature(capabilities)
enum class IntCapEnum : __intcap { Zero };

void *intcapEnumToPtrConstant = (void *)IntCapEnum::Zero;
void *test2() { return intcapEnumToPtrConstant; }
// PURECAP: @intcapEnumToPtrConstant = addrspace(200) global i8 addrspace(200)* null, align 16

extern int x;
void *ptrViaEnumCast = (void *)(IntCapEnum)(__intcap)&x;
void *test3() { return ptrViaEnumCast; }
// PURECAP: @ptrViaEnumCast = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @x to i8 addrspace(200)*), align 16
#endif
