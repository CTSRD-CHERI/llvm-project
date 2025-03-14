// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - -verify | %cheri_FileCheck %s
struct cap_struct {
  void* __capability cap;
  int x;
};
struct cap_struct cap_struct_global;
// CHECK: @cap_struct_global = addrspace(200) global %struct.cap_struct zeroinitializer, align [[#CAP_SIZE]]



#pragma pack(push, 1)
// expected-warning@+2 {{alignment (1) of 'struct packed_cap_pragma' is less than the required capability alignment}}
// expected-note@+1 {{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
struct packed_cap_pragma {
  int i;
  // expected-warning@+1 {{under aligned capability field at offset 4 in packed structure will trap if accessed}}
  void* __capability cap; 
};
#pragma pack(pop)
struct packed_cap_pragma packed_cap_global;
// CHECK: @packed_cap_global = addrspace(200) global %struct.packed_cap_pragma zeroinitializer, align 1


#pragma pack(push, 4)
// expected-warning@+2 {{alignment (4) of 'struct packed_cap_pragma_4' is less than the required capability alignment}}
// expected-note@+1 {{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
struct packed_cap_pragma_4 {
  int i;
  // expected-warning@+1 {{under aligned capability field at offset 4 in packed structure will trap if accessed}}
  void* __capability cap; 
};
#pragma pack(pop)
struct packed_cap_pragma_4 packed_cap_global_4;
// CHECK: @packed_cap_global_4 = addrspace(200) global %struct.packed_cap_pragma_4 zeroinitializer, align 4


#pragma pack(push, 4)
struct packed_but_force_align {
  int i,j,k,l;
  void *__capability cap;
} __attribute__((packed)) __attribute__((aligned(32)));
#pragma pack(pop)

struct packed_but_force_align forced;
// CHECK: @forced = addrspace(200) global %struct.packed_but_force_align zeroinitializer, align 32




#pragma align=packed
// expected-warning@+2 {{alignment (1) of 'struct align_pack_cap_pragma' is less than the required capability alignment}}
// expected-note@+1 {{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
struct align_pack_cap_pragma {
  int i;
  // expected-warning@+1 {{under aligned capability field at offset 4 in packed structure will trap if accessed}}
  void * __capability cap;
};
#pragma align=reset

struct align_pack_cap_pragma aligned_packed;
// CHECK: @aligned_packed = addrspace(200) global %struct.align_pack_cap_pragma zeroinitializer, align 1



#pragma pack(push, 1)
// expected-warning@+2 {{alignment (1) of 'struct packed_cap_struct' is less than the required capability alignment}}
// expected-note@+1 {{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
struct packed_cap_struct { 
  int i;
  // expected-warning@+1 {{under aligned structure field at offset 4 in packed structure containing capabilities will trap if capability field is accessed}}
  struct cap_struct cap;  
};
#pragma pack(pop)
struct packed_cap_struct packed_cap_struct_global;
// CHECK: @packed_cap_struct_global = addrspace(200) global %struct.packed_cap_struct zeroinitializer, align 1



// check that we warn for structs that are wrong when used in arrays
#pragma pack(push, 8)
// expected-note@+1 {{Add __attribute__((aligned(16))) to ensure sufficient alignment}}
struct correctly_aligned_but_size_not_multiple {
  char before[sizeof(void*)];
// expected-warning@+1 {{capability field at offset 16 in packed structure will trap if structure is used in an array}}
  __UINTPTR_TYPE__ cap;
  char after[1];
} __attribute__((annotate("underaligned_capability")));
#pragma pack(pop)
struct correctly_aligned_but_size_not_multiple correctly_aligned_but_size_not_multiple_global;
// CHECK: @correctly_aligned_but_size_not_multiple_global = addrspace(200) global %struct.correctly_aligned_but_size_not_multiple zeroinitializer, align 8
