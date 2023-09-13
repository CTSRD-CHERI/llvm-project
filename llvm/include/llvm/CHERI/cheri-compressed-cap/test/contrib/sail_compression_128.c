#include "sail.h"
#include "rts.h"
#include "elf.h"

// enum write_kind
enum zwrite_kind { zWrite_plain, zWrite_conditional, zWrite_release, zWrite_exclusive, zWrite_exclusive_release, zWrite_RISCV_release, zWrite_RISCV_strong_release, zWrite_RISCV_conditional, zWrite_RISCV_conditional_release, zWrite_RISCV_conditional_strong_release, zWrite_X86_locked };

static bool eq_zwrite_kind(enum zwrite_kind op1, enum zwrite_kind op2) { return op1 == op2; }

static enum zwrite_kind UNDEFINED(zwrite_kind)(unit u) { return zWrite_plain; }

// enum trans_kind
enum ztrans_kind { zTransaction_start, zTransaction_commit, zTransaction_abort };

static bool eq_ztrans_kind(enum ztrans_kind op1, enum ztrans_kind op2) { return op1 == op2; }

static enum ztrans_kind UNDEFINED(ztrans_kind)(unit u) { return zTransaction_start; }

// enum read_kind
enum zread_kind { zRead_plain, zRead_reserve, zRead_acquire, zRead_exclusive, zRead_exclusive_acquire, zRead_stream, zRead_ifetch, zRead_RISCV_acquire, zRead_RISCV_strong_acquire, zRead_RISCV_reserved, zRead_RISCV_reserved_acquire, zRead_RISCV_reserved_strong_acquire, zRead_X86_locked };

static bool eq_zread_kind(enum zread_kind op1, enum zread_kind op2) { return op1 == op2; }

static enum zread_kind UNDEFINED(zread_kind)(unit u) { return zRead_plain; }

// union exception
enum kind_zexception { Kind_z__dummy_exnz3 };

struct zexception {
  enum kind_zexception kind;
  union {struct { unit z__dummy_exnz3; };};
};

static void CREATE(zexception)(struct zexception *op)
{
  op->kind = Kind_z__dummy_exnz3;

}

static void RECREATE(zexception)(struct zexception *op) {}

static void KILL(zexception)(struct zexception *op)
{
  if (op->kind == Kind_z__dummy_exnz3){
    /* do nothing */
  };
}

static void COPY(zexception)(struct zexception *rop, struct zexception op)
{
  if (rop->kind == Kind_z__dummy_exnz3){
    /* do nothing */
  };
  rop->kind = op.kind;
  if (op.kind == Kind_z__dummy_exnz3){
    rop->z__dummy_exnz3 = op.z__dummy_exnz3;
  }
}

static bool EQUAL(zexception)(struct zexception op1, struct zexception op2) {
  if (op1.kind == Kind_z__dummy_exnz3 && op2.kind == Kind_z__dummy_exnz3) {
    return EQUAL(unit)(op1.z__dummy_exnz3, op2.z__dummy_exnz3);
  } else return false;
}

static void sailgen___dummy_exnz3(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_z__dummy_exnz3){
    /* do nothing */
  }
  rop->kind = Kind_z__dummy_exnz3;
  rop->z__dummy_exnz3 = op;
}

struct zexception *current_exception = NULL;
bool have_exception = false;
sail_string *throw_location = NULL;

// enum cache_op_kind
enum zcache_op_kind { zCache_op_D_IVAC, zCache_op_D_ISW, zCache_op_D_CSW, zCache_op_D_CISW, zCache_op_D_ZVA, zCache_op_D_CVAC, zCache_op_D_CVAU, zCache_op_D_CIVAC, zCache_op_I_IALLUIS, zCache_op_I_IALLU, zCache_op_I_IVAU };

static bool eq_zcache_op_kind(enum zcache_op_kind op1, enum zcache_op_kind op2) { return op1 == op2; }

static enum zcache_op_kind UNDEFINED(zcache_op_kind)(unit u) { return zCache_op_D_IVAC; }

// enum a64_barrier_type
enum za64_barrier_type { zA64_barrier_all, zA64_barrier_LD, zA64_barrier_ST };

static bool eq_za64_barrier_type(enum za64_barrier_type op1, enum za64_barrier_type op2) { return op1 == op2; }

static enum za64_barrier_type UNDEFINED(za64_barrier_type)(unit u) { return zA64_barrier_all; }

// enum a64_barrier_domain
enum za64_barrier_domain { zA64_FullShare, zA64_InnerShare, zA64_OuterShare, zA64_NonShare };

static bool eq_za64_barrier_domain(enum za64_barrier_domain op1, enum za64_barrier_domain op2) { return op1 == op2; }

static enum za64_barrier_domain UNDEFINED(za64_barrier_domain)(unit u) { return zA64_FullShare; }

// struct EncCapability
struct zEncCapability {
  uint64_t zB;
  uint64_t zT;
  uint64_t zaddress;
  uint64_t zflags;
  fbits zinternal_E;
  uint64_t zotype;
  uint64_t zperms;
  uint64_t zreserved;
};

static void COPY(zEncCapability)(struct zEncCapability *rop, const struct zEncCapability op) {
  rop->zB = op.zB;
  rop->zT = op.zT;
  rop->zaddress = op.zaddress;
  rop->zflags = op.zflags;
  rop->zinternal_E = op.zinternal_E;
  rop->zotype = op.zotype;
  rop->zperms = op.zperms;
  rop->zreserved = op.zreserved;
}

static bool EQUAL(zEncCapability)(struct zEncCapability op1, struct zEncCapability op2) {
  return EQUAL(fbits)(op1.zB, op2.zB) && EQUAL(fbits)(op1.zT, op2.zT) && EQUAL(fbits)(op1.zaddress, op2.zaddress) && EQUAL(fbits)(op1.zflags, op2.zflags) && EQUAL(fbits)(op1.zinternal_E, op2.zinternal_E) && EQUAL(fbits)(op1.zotype, op2.zotype) && EQUAL(fbits)(op1.zperms, op2.zperms) && EQUAL(fbits)(op1.zreserved, op2.zreserved);
}

// enum ClearRegSet
enum zClearRegSet { zGPRegs, zFPRegs };

static bool eq_zClearRegSet(enum zClearRegSet op1, enum zClearRegSet op2) { return op1 == op2; }

static enum zClearRegSet UNDEFINED(zClearRegSet)(unit u) { return zGPRegs; }

// struct Capability
struct zCapability {
  uint64_t zB;
  uint64_t zE;
  uint64_t zT;
  bool zaccess_system_regs;
  uint64_t zaddress;
  bool zflag_cap_mode;
  bool zglobal;
  bool zinternal_E;
  uint64_t zotype;
  bool zpermit_cinvoke;
  bool zpermit_execute;
  bool zpermit_load;
  bool zpermit_load_cap;
  bool zpermit_seal;
  bool zpermit_set_CID;
  bool zpermit_store;
  bool zpermit_store_cap;
  bool zpermit_store_local_cap;
  bool zpermit_unseal;
  uint64_t zreserved;
  bool ztag;
  uint64_t zuperms;
};

static void COPY(zCapability)(struct zCapability *rop, const struct zCapability op) {
  rop->zB = op.zB;
  rop->zE = op.zE;
  rop->zT = op.zT;
  rop->zaccess_system_regs = op.zaccess_system_regs;
  rop->zaddress = op.zaddress;
  rop->zflag_cap_mode = op.zflag_cap_mode;
  rop->zglobal = op.zglobal;
  rop->zinternal_E = op.zinternal_E;
  rop->zotype = op.zotype;
  rop->zpermit_cinvoke = op.zpermit_cinvoke;
  rop->zpermit_execute = op.zpermit_execute;
  rop->zpermit_load = op.zpermit_load;
  rop->zpermit_load_cap = op.zpermit_load_cap;
  rop->zpermit_seal = op.zpermit_seal;
  rop->zpermit_set_CID = op.zpermit_set_CID;
  rop->zpermit_store = op.zpermit_store;
  rop->zpermit_store_cap = op.zpermit_store_cap;
  rop->zpermit_store_local_cap = op.zpermit_store_local_cap;
  rop->zpermit_unseal = op.zpermit_unseal;
  rop->zreserved = op.zreserved;
  rop->ztag = op.ztag;
  rop->zuperms = op.zuperms;
}

static bool EQUAL(zCapability)(struct zCapability op1, struct zCapability op2) {
  return EQUAL(fbits)(op1.zB, op2.zB) && EQUAL(fbits)(op1.zE, op2.zE) && EQUAL(fbits)(op1.zT, op2.zT) && EQUAL(bool)(op1.zaccess_system_regs, op2.zaccess_system_regs) && EQUAL(fbits)(op1.zaddress, op2.zaddress) && EQUAL(bool)(op1.zflag_cap_mode, op2.zflag_cap_mode) && EQUAL(bool)(op1.zglobal, op2.zglobal) && EQUAL(bool)(op1.zinternal_E, op2.zinternal_E) && EQUAL(fbits)(op1.zotype, op2.zotype) && EQUAL(bool)(op1.zpermit_cinvoke, op2.zpermit_cinvoke) && EQUAL(bool)(op1.zpermit_execute, op2.zpermit_execute) && EQUAL(bool)(op1.zpermit_load, op2.zpermit_load) && EQUAL(bool)(op1.zpermit_load_cap, op2.zpermit_load_cap) && EQUAL(bool)(op1.zpermit_seal, op2.zpermit_seal) && EQUAL(bool)(op1.zpermit_set_CID, op2.zpermit_set_CID) && EQUAL(bool)(op1.zpermit_store, op2.zpermit_store) && EQUAL(bool)(op1.zpermit_store_cap, op2.zpermit_store_cap) && EQUAL(bool)(op1.zpermit_store_local_cap, op2.zpermit_store_local_cap) && EQUAL(bool)(op1.zpermit_unseal, op2.zpermit_unseal) && EQUAL(fbits)(op1.zreserved, op2.zreserved) && EQUAL(bool)(op1.ztag, op2.ztag) && EQUAL(fbits)(op1.zuperms, op2.zuperms);
}

// enum CapEx
enum zCapEx { zCapEx_None, zCapEx_LengthViolation, zCapEx_TagViolation, zCapEx_SealViolation, zCapEx_TypeViolation, zCapEx_UserDefViolation, zCapEx_UnalignedBase, zCapEx_GlobalViolation, zCapEx_PermitExecuteViolation, zCapEx_PermitLoadViolation, zCapEx_PermitStoreViolation, zCapEx_PermitLoadCapViolation, zCapEx_PermitStoreCapViolation, zCapEx_PermitStoreLocalCapViolation, zCapEx_AccessSystemRegsViolation, zCapEx_PermitCInvokeViolation, zCapEx_PermitSetCIDViolation };

static bool eq_zCapEx(enum zCapEx op1, enum zCapEx op2) { return op1 == op2; }

static enum zCapEx UNDEFINED(zCapEx)(unit u) { return zCapEx_None; }

// enum CPtrCmpOp
enum zCPtrCmpOp { zCEQ, zCNE, zCLT, zCLE, zCLTU, zCLEU, zCEXEQ, zCNEXEQ };

static bool eq_zCPtrCmpOp(enum zCPtrCmpOp op1, enum zCPtrCmpOp op2) { return op1 == op2; }

static enum zCPtrCmpOp UNDEFINED(zCPtrCmpOp)(unit u) { return zCEQ; }

















static bool sailgen_neq_int(sail_int, sail_int);

static bool sailgen_neq_int(sail_int zx, sail_int zy)
{
  __label__ end_function_1, end_block_exception_2;

  bool zcbz30;
  bool zgaz30;
  zgaz30 = eq_int(zx, zy);
  zcbz30 = not(zgaz30);

end_function_1: ;
  return zcbz30;
end_block_exception_2: ;

  return false;
}





















static bool sailgen_neq_bits(lbits zx, lbits zy)
{
  __label__ end_function_4, end_block_exception_5;

  bool zcbz31;
  bool zgaz31;
  zgaz31 = eq_bits(zx, zy);
  zcbz31 = not(zgaz31);

end_function_4: ;
  return zcbz31;
end_block_exception_5: ;

  return false;
}



































static void sailgen_sail_ones(lbits *rop, sail_int);

static void sailgen_sail_ones(lbits *zcbz32, sail_int zn)
{
  __label__ end_function_7, end_block_exception_8, end_function_208;

  lbits zgaz32;
  CREATE(lbits)(&zgaz32);
  zeros(&zgaz32, zn);
  not_bits((*(&zcbz32)), zgaz32);
  KILL(lbits)(&zgaz32);
end_function_7: ;
  goto end_function_208;
end_block_exception_8: ;
  goto end_function_208;
end_function_208: ;
}







static fbits sailgen_not_bit(fbits);

static fbits sailgen_not_bit(fbits zb)
{
  __label__ end_function_10, end_block_exception_11;

  fbits zcbz33;
  bool zgaz33;
  zgaz33 = eq_bit(zb, UINT64_C(1));
  if (zgaz33) {  zcbz33 = UINT64_C(0);  } else {  zcbz33 = UINT64_C(1);  }

end_function_10: ;
  return zcbz33;
end_block_exception_11: ;

  return UINT64_C(0);
}















static void sailgen_EXTS(lbits *rop, sail_int, lbits);

static void sailgen_EXTZ(lbits *rop, sail_int, lbits);

static void sailgen_EXTS(lbits *zcbz34, sail_int zm, lbits zv)
{
  __label__ end_function_13, end_block_exception_14, end_function_207;

  sign_extend((*(&zcbz34)), zv, zm);
end_function_13: ;
  goto end_function_207;
end_block_exception_14: ;
  goto end_function_207;
end_function_207: ;
}

static void sailgen_EXTZ(lbits *zcbz35, sail_int zm, lbits zv)
{
  __label__ end_function_16, end_block_exception_17, end_function_206;

  zero_extend((*(&zcbz35)), zv, zm);
end_function_16: ;
  goto end_function_206;
end_block_exception_17: ;
  goto end_function_206;
end_function_206: ;
}

static void sailgen_zzeros_implicit(lbits *rop, sail_int);

static void sailgen_zzeros_implicit(lbits *zcbz36, sail_int zn)
{
  __label__ end_function_19, end_block_exception_20, end_function_205;

  zeros((*(&zcbz36)), zn);
end_function_19: ;
  goto end_function_205;
end_block_exception_20: ;
  goto end_function_205;
end_function_205: ;
}

static void sailgen_ones(lbits *rop, sail_int);

static void sailgen_ones(lbits *zcbz37, sail_int zn)
{
  __label__ end_function_22, end_block_exception_23, end_function_204;

  sailgen_sail_ones((*(&zcbz37)), zn);
end_function_22: ;
  goto end_function_204;
end_block_exception_23: ;
  goto end_function_204;
end_function_204: ;
}

static uint64_t sailgen_bool_to_bits(bool);

static uint64_t sailgen_bool_to_bits(bool zx)
{
  __label__ end_function_25, end_block_exception_26;

  uint64_t zcbz38;
  if (zx) {  zcbz38 = UINT64_C(0b1);  } else {  zcbz38 = UINT64_C(0b0);  }
end_function_25: ;
  return zcbz38;
end_block_exception_26: ;

  return UINT64_C(0xdeadc0de);
}

static bool sailgen_bit_to_bool(fbits);

static bool sailgen_bit_to_bool(fbits zb)
{
  __label__ case_29, case_30, finish_match_28, end_function_31, end_block_exception_32;

  bool zcbz39;
  bool zgsz30;
  {
    fbits zp0z3;
    zp0z3 = zb;
    bool zgsz31;
    zgsz31 = eq_bit(zp0z3, UINT64_C(1));
    if (!(zgsz31)) {

      goto case_29;
    }
    zgsz30 = true;
    goto finish_match_28;
  }
case_29: ;
  {
    fbits zuz30;
    zuz30 = zb;
    bool zgsz32;
    zgsz32 = eq_bit(zuz30, UINT64_C(0));
    if (!(zgsz32)) {

      goto case_30;
    }
    zgsz30 = false;
    goto finish_match_28;
  }
case_30: ;
  sail_match_failure("bit_to_bool");
finish_match_28: ;
  zcbz39 = zgsz30;

end_function_31: ;
  return zcbz39;
end_block_exception_32: ;

  return false;
}

static void sailgen_to_bits(lbits *rop, sail_int, sail_int);

static void sailgen_to_bits(lbits *zcbz310, sail_int zl, sail_int zn)
{
  __label__ end_function_34, end_block_exception_35, end_function_203;

  {
    sail_int zgsz3106;
    CREATE(sail_int)(&zgsz3106);
    CONVERT_OF(sail_int, mach_int)(&zgsz3106, INT64_C(0));
    get_slice_int((*(&zcbz310)), zl, zn, zgsz3106);
    KILL(sail_int)(&zgsz3106);
  }
end_function_34: ;
  goto end_function_203;
end_block_exception_35: ;
  goto end_function_203;
end_function_203: ;
}

static bool sailgen_z8operatorz0zI_uz9(lbits, lbits);

static bool sailgen_z8operatorz0zKzJ_uz9(lbits, lbits);

static bool sailgen_z8operatorz0zI_uz9(lbits zx, lbits zy)
{
  __label__ end_function_37, end_block_exception_38;

  bool zcbz311;
  sail_int zgaz34;
  CREATE(sail_int)(&zgaz34);
  sail_unsigned(&zgaz34, zx);
  sail_int zgaz35;
  CREATE(sail_int)(&zgaz35);
  sail_unsigned(&zgaz35, zy);
  zcbz311 = lt(zgaz34, zgaz35);
  KILL(sail_int)(&zgaz35);
  KILL(sail_int)(&zgaz34);
end_function_37: ;
  return zcbz311;
end_block_exception_38: ;

  return false;
}

static bool sailgen_z8operatorz0zKzJ_uz9(lbits zx, lbits zy)
{
  __label__ end_function_40, end_block_exception_41;

  bool zcbz312;
  sail_int zgaz36;
  CREATE(sail_int)(&zgaz36);
  sail_unsigned(&zgaz36, zx);
  sail_int zgaz37;
  CREATE(sail_int)(&zgaz37);
  sail_unsigned(&zgaz37, zy);
  zcbz312 = gteq(zgaz36, zgaz37);
  KILL(sail_int)(&zgaz37);
  KILL(sail_int)(&zgaz36);
end_function_40: ;
  return zcbz312;
end_block_exception_41: ;

  return false;
}



static void sailgen_MAX(sail_int *rop, sail_int);

static void sailgen_MAX(sail_int *zcbz313, sail_int zn)
{
  __label__ end_function_43, end_block_exception_44, end_function_202;

  sail_int zgaz38;
  CREATE(sail_int)(&zgaz38);
  pow2(&zgaz38, zn);
  {
    sail_int zgsz3107;
    CREATE(sail_int)(&zgsz3107);
    CONVERT_OF(sail_int, mach_int)(&zgsz3107, INT64_C(1));
    sub_int((*(&zcbz313)), zgaz38, zgsz3107);
    KILL(sail_int)(&zgsz3107);
  }
  KILL(sail_int)(&zgaz38);
end_function_43: ;
  goto end_function_202;
end_block_exception_44: ;
  goto end_function_202;
end_function_202: ;
}



static int64_t zreserved_otypes;
static void create_letbind_0(void) {


  int64_t zgsz33;
  zgsz33 = INT64_C(4);
  zreserved_otypes = zgsz33;

let_end_45: ;
}
static void kill_letbind_0(void) {
}

static int64_t zotype_unsealed;
static void create_letbind_1(void) {


  int64_t zgsz34;
  zgsz34 = INT64_C(-1);
  zotype_unsealed = zgsz34;

let_end_46: ;
}
static void kill_letbind_1(void) {
}

static int64_t zotype_sentry;
static void create_letbind_2(void) {


  int64_t zgsz35;
  zgsz35 = INT64_C(-2);
  zotype_sentry = zgsz35;

let_end_47: ;
}
static void kill_letbind_2(void) {
}

static uint64_t zPCC_IDX;
static void create_letbind_3(void) {


  uint64_t zgsz36;
  zgsz36 = UINT64_C(0b100000);
  zPCC_IDX = zgsz36;

let_end_48: ;
}
static void kill_letbind_3(void) {
}

static uint64_t zDDC_IDX;
static void create_letbind_4(void) {


  uint64_t zgsz37;
  zgsz37 = UINT64_C(0b100001);
  zDDC_IDX = zgsz37;

let_end_49: ;
}
static void kill_letbind_4(void) {
}

static int64_t zcap_sizze;
static void create_letbind_5(void) {


  int64_t zgsz38;
  zgsz38 = INT64_C(16);
  zcap_sizze = zgsz38;

let_end_50: ;
}
static void kill_letbind_5(void) {
}

static int64_t zlog2_cap_sizze;
static void create_letbind_6(void) {


  int64_t zgsz39;
  zgsz39 = INT64_C(4);
  zlog2_cap_sizze = zgsz39;

let_end_51: ;
}
static void kill_letbind_6(void) {
}

static int64_t zcap_hperms_width;
static void create_letbind_7(void) {


  int64_t zgsz310;
  zgsz310 = INT64_C(12);
  zcap_hperms_width = zgsz310;

let_end_52: ;
}
static void kill_letbind_7(void) {
}

static int64_t zcap_uperms_width;
static void create_letbind_8(void) {


  int64_t zgsz311;
  zgsz311 = INT64_C(4);
  zcap_uperms_width = zgsz311;

let_end_53: ;
}
static void kill_letbind_8(void) {
}

static int64_t zcap_otype_width;
static void create_letbind_9(void) {


  int64_t zgsz312;
  zgsz312 = INT64_C(18);
  zcap_otype_width = zgsz312;

let_end_54: ;
}
static void kill_letbind_9(void) {
}

static int64_t zcap_reserved_width;
static void create_letbind_10(void) {


  int64_t zgsz313;
  zgsz313 = INT64_C(2);
  zcap_reserved_width = zgsz313;

let_end_55: ;
}
static void kill_letbind_10(void) {
}

static int64_t zcap_flags_width;
static void create_letbind_11(void) {


  int64_t zgsz314;
  zgsz314 = INT64_C(1);
  zcap_flags_width = zgsz314;

let_end_56: ;
}
static void kill_letbind_11(void) {
}

static int64_t zcap_mantissa_width;
static void create_letbind_12(void) {


  int64_t zgsz315;
  zgsz315 = INT64_C(14);
  zcap_mantissa_width = zgsz315;

let_end_57: ;
}
static void kill_letbind_12(void) {
}

static int64_t zcap_E_width;
static void create_letbind_13(void) {


  int64_t zgsz316;
  zgsz316 = INT64_C(6);
  zcap_E_width = zgsz316;

let_end_58: ;
}
static void kill_letbind_13(void) {
}

static int64_t zcap_addr_width;
static void create_letbind_14(void) {


  int64_t zgsz317;
  zgsz317 = INT64_C(64);
  zcap_addr_width = zgsz317;

let_end_59: ;
}
static void kill_letbind_14(void) {
}

static int64_t zcap_len_width;
static void create_letbind_15(void) {


  int64_t zgsz318;
  {
    sail_int zgsz3108;
    CREATE(sail_int)(&zgsz3108);
    CONVERT_OF(sail_int, mach_int)(&zgsz3108, INT64_C(64));
    sail_int zgsz3109;
    CREATE(sail_int)(&zgsz3109);
    CONVERT_OF(sail_int, mach_int)(&zgsz3109, INT64_C(1));
    sail_int zgsz3110;
    CREATE(sail_int)(&zgsz3110);
    add_int(&zgsz3110, zgsz3108, zgsz3109);
    zgsz318 = CONVERT_OF(mach_int, sail_int)(zgsz3110);
    KILL(sail_int)(&zgsz3110);
    KILL(sail_int)(&zgsz3109);
    KILL(sail_int)(&zgsz3108);
  }
  zcap_len_width = zgsz318;

let_end_60: ;
}
static void kill_letbind_15(void) {
}

static int64_t zcaps_per_cache_line;
static void create_letbind_16(void) {


  int64_t zgsz319;
  zgsz319 = INT64_C(4);
  zcaps_per_cache_line = zgsz319;

let_end_61: ;
}
static void kill_letbind_16(void) {
}

static int64_t zinternal_E_take_bits;
static void create_letbind_17(void) {


  int64_t zgsz320;
  zgsz320 = INT64_C(3);
  zinternal_E_take_bits = zgsz320;

let_end_62: ;
}
static void kill_letbind_17(void) {
}

static struct zEncCapability sailgen_capBitsToEncCapability(lbits);

static struct zEncCapability sailgen_capBitsToEncCapability(lbits zc)
{
  __label__ end_function_64, end_block_exception_65;

  struct zEncCapability zcbz314;
  uint64_t zgaz39;
  {
    sail_int zgsz3130;
    CREATE(sail_int)(&zgsz3130);
    CONVERT_OF(sail_int, mach_int)(&zgsz3130, INT64_C(127));
    sail_int zgsz3131;
    CREATE(sail_int)(&zgsz3131);
    CONVERT_OF(sail_int, mach_int)(&zgsz3131, INT64_C(112));
    lbits zgsz3132;
    CREATE(lbits)(&zgsz3132);
    vector_subrange_lbits(&zgsz3132, zc, zgsz3130, zgsz3131);
    zgaz39 = CONVERT_OF(fbits, lbits)(zgsz3132, true);
    KILL(lbits)(&zgsz3132);
    KILL(sail_int)(&zgsz3131);
    KILL(sail_int)(&zgsz3130);
  }
  uint64_t zgaz310;
  {
    sail_int zgsz3127;
    CREATE(sail_int)(&zgsz3127);
    CONVERT_OF(sail_int, mach_int)(&zgsz3127, INT64_C(111));
    sail_int zgsz3128;
    CREATE(sail_int)(&zgsz3128);
    CONVERT_OF(sail_int, mach_int)(&zgsz3128, INT64_C(110));
    lbits zgsz3129;
    CREATE(lbits)(&zgsz3129);
    vector_subrange_lbits(&zgsz3129, zc, zgsz3127, zgsz3128);
    zgaz310 = CONVERT_OF(fbits, lbits)(zgsz3129, true);
    KILL(lbits)(&zgsz3129);
    KILL(sail_int)(&zgsz3128);
    KILL(sail_int)(&zgsz3127);
  }
  uint64_t zgaz311;
  {
    sail_int zgsz3124;
    CREATE(sail_int)(&zgsz3124);
    CONVERT_OF(sail_int, mach_int)(&zgsz3124, INT64_C(109));
    sail_int zgsz3125;
    CREATE(sail_int)(&zgsz3125);
    CONVERT_OF(sail_int, mach_int)(&zgsz3125, INT64_C(109));
    lbits zgsz3126;
    CREATE(lbits)(&zgsz3126);
    vector_subrange_lbits(&zgsz3126, zc, zgsz3124, zgsz3125);
    zgaz311 = CONVERT_OF(fbits, lbits)(zgsz3126, true);
    KILL(lbits)(&zgsz3126);
    KILL(sail_int)(&zgsz3125);
    KILL(sail_int)(&zgsz3124);
  }
  uint64_t zgaz312;
  {
    sail_int zgsz3121;
    CREATE(sail_int)(&zgsz3121);
    CONVERT_OF(sail_int, mach_int)(&zgsz3121, INT64_C(108));
    sail_int zgsz3122;
    CREATE(sail_int)(&zgsz3122);
    CONVERT_OF(sail_int, mach_int)(&zgsz3122, INT64_C(91));
    lbits zgsz3123;
    CREATE(lbits)(&zgsz3123);
    vector_subrange_lbits(&zgsz3123, zc, zgsz3121, zgsz3122);
    zgaz312 = CONVERT_OF(fbits, lbits)(zgsz3123, true);
    KILL(lbits)(&zgsz3123);
    KILL(sail_int)(&zgsz3122);
    KILL(sail_int)(&zgsz3121);
  }
  fbits zgaz313;
  {
    sail_int zgsz3120;
    CREATE(sail_int)(&zgsz3120);
    CONVERT_OF(sail_int, mach_int)(&zgsz3120, INT64_C(90));
    zgaz313 = bitvector_access(zc, zgsz3120);
    KILL(sail_int)(&zgsz3120);
  }
  uint64_t zgaz314;
  {
    sail_int zgsz3117;
    CREATE(sail_int)(&zgsz3117);
    CONVERT_OF(sail_int, mach_int)(&zgsz3117, INT64_C(89));
    sail_int zgsz3118;
    CREATE(sail_int)(&zgsz3118);
    CONVERT_OF(sail_int, mach_int)(&zgsz3118, INT64_C(78));
    lbits zgsz3119;
    CREATE(lbits)(&zgsz3119);
    vector_subrange_lbits(&zgsz3119, zc, zgsz3117, zgsz3118);
    zgaz314 = CONVERT_OF(fbits, lbits)(zgsz3119, true);
    KILL(lbits)(&zgsz3119);
    KILL(sail_int)(&zgsz3118);
    KILL(sail_int)(&zgsz3117);
  }
  uint64_t zgaz315;
  {
    sail_int zgsz3114;
    CREATE(sail_int)(&zgsz3114);
    CONVERT_OF(sail_int, mach_int)(&zgsz3114, INT64_C(77));
    sail_int zgsz3115;
    CREATE(sail_int)(&zgsz3115);
    CONVERT_OF(sail_int, mach_int)(&zgsz3115, INT64_C(64));
    lbits zgsz3116;
    CREATE(lbits)(&zgsz3116);
    vector_subrange_lbits(&zgsz3116, zc, zgsz3114, zgsz3115);
    zgaz315 = CONVERT_OF(fbits, lbits)(zgsz3116, true);
    KILL(lbits)(&zgsz3116);
    KILL(sail_int)(&zgsz3115);
    KILL(sail_int)(&zgsz3114);
  }
  uint64_t zgaz316;
  {
    sail_int zgsz3111;
    CREATE(sail_int)(&zgsz3111);
    CONVERT_OF(sail_int, mach_int)(&zgsz3111, INT64_C(63));
    sail_int zgsz3112;
    CREATE(sail_int)(&zgsz3112);
    CONVERT_OF(sail_int, mach_int)(&zgsz3112, INT64_C(0));
    lbits zgsz3113;
    CREATE(lbits)(&zgsz3113);
    vector_subrange_lbits(&zgsz3113, zc, zgsz3111, zgsz3112);
    zgaz316 = CONVERT_OF(fbits, lbits)(zgsz3113, true);
    KILL(lbits)(&zgsz3113);
    KILL(sail_int)(&zgsz3112);
    KILL(sail_int)(&zgsz3111);
  }
  struct zEncCapability zgsz321;
  zgsz321.zB = zgaz315;
  zgsz321.zT = zgaz314;
  zgsz321.zaddress = zgaz316;
  zgsz321.zflags = zgaz311;
  zgsz321.zinternal_E = zgaz313;
  zgsz321.zotype = zgaz312;
  zgsz321.zperms = zgaz39;
  zgsz321.zreserved = zgaz310;
  zcbz314 = zgsz321;









end_function_64: ;
  return zcbz314;
end_block_exception_65: ;
  struct zEncCapability zcbz346 = { .zreserved = UINT64_C(0xdeadc0de), .zperms = UINT64_C(0xdeadc0de), .zotype = UINT64_C(0xdeadc0de), .zinternal_E = UINT64_C(0), .zflags = UINT64_C(0xdeadc0de), .zaddress = UINT64_C(0xdeadc0de), .zT = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz346;
}

static void sailgen_encCapToBits(lbits *rop, struct zEncCapability);

static void sailgen_encCapToBits(lbits *zcbz315, struct zEncCapability zcap)
{
  __label__ end_function_67, end_block_exception_68, end_function_201;

  uint64_t zgaz330;
  zgaz330 = zcap.zperms;
  lbits zgaz331;
  CREATE(lbits)(&zgaz331);
  {
    uint64_t zgaz328;
    zgaz328 = zcap.zreserved;
    lbits zgaz329;
    CREATE(lbits)(&zgaz329);
    {
      uint64_t zgaz326;
      zgaz326 = zcap.zflags;
      lbits zgaz327;
      CREATE(lbits)(&zgaz327);
      {
        uint64_t zgaz324;
        zgaz324 = zcap.zotype;
        lbits zgaz325;
        CREATE(lbits)(&zgaz325);
        {
          uint64_t zgaz322;
          {
            fbits zgaz317;
            zgaz317 = zcap.zinternal_E;
            uint64_t zgsz322;
            zgsz322 = UINT64_C(0b0);
            zgsz322 = update_fbits(zgsz322, INT64_C(0), zgaz317);
            zgaz322 = zgsz322;
          }
          lbits zgaz323;
          CREATE(lbits)(&zgaz323);
          {
            uint64_t zgaz320;
            zgaz320 = zcap.zT;
            lbits zgaz321;
            CREATE(lbits)(&zgaz321);
            {
              uint64_t zgaz318;
              zgaz318 = zcap.zB;
              uint64_t zgaz319;
              zgaz319 = zcap.zaddress;
              {
                lbits zgsz3133;
                CREATE(lbits)(&zgsz3133);
                CONVERT_OF(lbits, fbits)(&zgsz3133, zgaz318, UINT64_C(14) , true);
                lbits zgsz3134;
                CREATE(lbits)(&zgsz3134);
                CONVERT_OF(lbits, fbits)(&zgsz3134, zgaz319, UINT64_C(64) , true);
                append(&zgaz321, zgsz3133, zgsz3134);
                KILL(lbits)(&zgsz3134);
                KILL(lbits)(&zgsz3133);
              }
            }
            {
              lbits zgsz3135;
              CREATE(lbits)(&zgsz3135);
              CONVERT_OF(lbits, fbits)(&zgsz3135, zgaz320, UINT64_C(12) , true);
              append(&zgaz323, zgsz3135, zgaz321);
              KILL(lbits)(&zgsz3135);
            }
            KILL(lbits)(&zgaz321);
          }
          {
            lbits zgsz3136;
            CREATE(lbits)(&zgsz3136);
            CONVERT_OF(lbits, fbits)(&zgsz3136, zgaz322, UINT64_C(1) , true);
            append(&zgaz325, zgsz3136, zgaz323);
            KILL(lbits)(&zgsz3136);
          }
          KILL(lbits)(&zgaz323);
        }
        {
          lbits zgsz3137;
          CREATE(lbits)(&zgsz3137);
          CONVERT_OF(lbits, fbits)(&zgsz3137, zgaz324, UINT64_C(18) , true);
          append(&zgaz327, zgsz3137, zgaz325);
          KILL(lbits)(&zgsz3137);
        }
        KILL(lbits)(&zgaz325);
      }
      {
        lbits zgsz3138;
        CREATE(lbits)(&zgsz3138);
        CONVERT_OF(lbits, fbits)(&zgsz3138, zgaz326, UINT64_C(1) , true);
        append(&zgaz329, zgsz3138, zgaz327);
        KILL(lbits)(&zgsz3138);
      }
      KILL(lbits)(&zgaz327);
    }
    {
      lbits zgsz3139;
      CREATE(lbits)(&zgsz3139);
      CONVERT_OF(lbits, fbits)(&zgsz3139, zgaz328, UINT64_C(2) , true);
      append(&zgaz331, zgsz3139, zgaz329);
      KILL(lbits)(&zgsz3139);
    }
    KILL(lbits)(&zgaz329);
  }
  {
    lbits zgsz3140;
    CREATE(lbits)(&zgsz3140);
    CONVERT_OF(lbits, fbits)(&zgsz3140, zgaz330, UINT64_C(16) , true);
    append((*(&zcbz315)), zgsz3140, zgaz331);
    KILL(lbits)(&zgsz3140);
  }
  KILL(lbits)(&zgaz331);

end_function_67: ;
  goto end_function_201;
end_block_exception_68: ;
  goto end_function_201;
end_function_201: ;
}

static sail_int zcap_max_addr;
static void create_letbind_18(void) {    CREATE(sail_int)(&zcap_max_addr);


  sail_int zgsz323;
  CREATE(sail_int)(&zgsz323);
  {
    sail_int zgsz3141;
    CREATE(sail_int)(&zgsz3141);
    CONVERT_OF(sail_int, mach_int)(&zgsz3141, zcap_addr_width);
    sailgen_MAX(&zgsz323, zgsz3141);
    KILL(sail_int)(&zgsz3141);
  }
  COPY(sail_int)(&zcap_max_addr, zgsz323);
  KILL(sail_int)(&zgsz323);
let_end_69: ;
}
static void kill_letbind_18(void) {    KILL(sail_int)(&zcap_max_addr);
}

static int64_t zcap_max_otype;
static void create_letbind_19(void) {


  int64_t zgsz324;
  int64_t zgaz332;
  {
    sail_int zgsz3145;
    CREATE(sail_int)(&zgsz3145);
    CONVERT_OF(sail_int, mach_int)(&zgsz3145, zcap_otype_width);
    sail_int zgsz3146;
    CREATE(sail_int)(&zgsz3146);
    sailgen_MAX(&zgsz3146, zgsz3145);
    zgaz332 = CONVERT_OF(mach_int, sail_int)(zgsz3146);
    KILL(sail_int)(&zgsz3146);
    KILL(sail_int)(&zgsz3145);
  }
  {
    sail_int zgsz3142;
    CREATE(sail_int)(&zgsz3142);
    CONVERT_OF(sail_int, mach_int)(&zgsz3142, zgaz332);
    sail_int zgsz3143;
    CREATE(sail_int)(&zgsz3143);
    CONVERT_OF(sail_int, mach_int)(&zgsz3143, zreserved_otypes);
    sail_int zgsz3144;
    CREATE(sail_int)(&zgsz3144);
    sub_int(&zgsz3144, zgsz3142, zgsz3143);
    zgsz324 = CONVERT_OF(mach_int, sail_int)(zgsz3144);
    KILL(sail_int)(&zgsz3144);
    KILL(sail_int)(&zgsz3143);
    KILL(sail_int)(&zgsz3142);
  }

  zcap_max_otype = zgsz324;

let_end_70: ;
}
static void kill_letbind_19(void) {
}

static int64_t zcap_uperms_shift;
static void create_letbind_20(void) {


  int64_t zgsz325;
  zgsz325 = INT64_C(15);
  zcap_uperms_shift = zgsz325;

let_end_71: ;
}
static void kill_letbind_20(void) {
}

static int64_t zcap_perms_width;
static void create_letbind_21(void) {


  int64_t zgsz326;
  {
    sail_int zgsz3147;
    CREATE(sail_int)(&zgsz3147);
    CONVERT_OF(sail_int, mach_int)(&zgsz3147, INT64_C(15));
    sail_int zgsz3148;
    CREATE(sail_int)(&zgsz3148);
    CONVERT_OF(sail_int, mach_int)(&zgsz3148, INT64_C(4));
    sail_int zgsz3149;
    CREATE(sail_int)(&zgsz3149);
    add_int(&zgsz3149, zgsz3147, zgsz3148);
    zgsz326 = CONVERT_OF(mach_int, sail_int)(zgsz3149);
    KILL(sail_int)(&zgsz3149);
    KILL(sail_int)(&zgsz3148);
    KILL(sail_int)(&zgsz3147);
  }
  zcap_perms_width = zgsz326;

let_end_72: ;
}
static void kill_letbind_21(void) {
}

static int64_t zcap_max_E;
static void create_letbind_22(void) {


  int64_t zgsz327;
  int64_t zgaz333;
  {
    sail_int zgsz3153;
    CREATE(sail_int)(&zgsz3153);
    CONVERT_OF(sail_int, mach_int)(&zgsz3153, zcap_len_width);
    sail_int zgsz3154;
    CREATE(sail_int)(&zgsz3154);
    CONVERT_OF(sail_int, mach_int)(&zgsz3154, zcap_mantissa_width);
    sail_int zgsz3155;
    CREATE(sail_int)(&zgsz3155);
    sub_int(&zgsz3155, zgsz3153, zgsz3154);
    zgaz333 = CONVERT_OF(mach_int, sail_int)(zgsz3155);
    KILL(sail_int)(&zgsz3155);
    KILL(sail_int)(&zgsz3154);
    KILL(sail_int)(&zgsz3153);
  }
  {
    sail_int zgsz3150;
    CREATE(sail_int)(&zgsz3150);
    CONVERT_OF(sail_int, mach_int)(&zgsz3150, zgaz333);
    sail_int zgsz3151;
    CREATE(sail_int)(&zgsz3151);
    CONVERT_OF(sail_int, mach_int)(&zgsz3151, INT64_C(1));
    sail_int zgsz3152;
    CREATE(sail_int)(&zgsz3152);
    add_int(&zgsz3152, zgsz3150, zgsz3151);
    zgsz327 = CONVERT_OF(mach_int, sail_int)(zgsz3152);
    KILL(sail_int)(&zgsz3152);
    KILL(sail_int)(&zgsz3151);
    KILL(sail_int)(&zgsz3150);
  }

  zcap_max_E = zgsz327;

let_end_73: ;
}
static void kill_letbind_22(void) {
}

static uint64_t zcap_reset_E;
static void create_letbind_23(void) {


  uint64_t zgsz328;
  {
    sail_int zgsz3156;
    CREATE(sail_int)(&zgsz3156);
    CONVERT_OF(sail_int, mach_int)(&zgsz3156, zcap_E_width);
    sail_int zgsz3157;
    CREATE(sail_int)(&zgsz3157);
    CONVERT_OF(sail_int, mach_int)(&zgsz3157, zcap_max_E);
    lbits zgsz3158;
    CREATE(lbits)(&zgsz3158);
    sailgen_to_bits(&zgsz3158, zgsz3156, zgsz3157);
    zgsz328 = CONVERT_OF(fbits, lbits)(zgsz3158, true);
    KILL(lbits)(&zgsz3158);
    KILL(sail_int)(&zgsz3157);
    KILL(sail_int)(&zgsz3156);
  }
  zcap_reset_E = zgsz328;

let_end_74: ;
}
static void kill_letbind_23(void) {
}

static uint64_t zcap_reset_T;
static void create_letbind_24(void) {


  uint64_t zgsz329;
  uint64_t zgaz335;
  {
    int64_t zgaz334;
    {
      sail_int zgsz3161;
      CREATE(sail_int)(&zgsz3161);
      CONVERT_OF(sail_int, mach_int)(&zgsz3161, zcap_mantissa_width);
      sail_int zgsz3162;
      CREATE(sail_int)(&zgsz3162);
      CONVERT_OF(sail_int, mach_int)(&zgsz3162, INT64_C(2));
      sail_int zgsz3163;
      CREATE(sail_int)(&zgsz3163);
      sub_int(&zgsz3163, zgsz3161, zgsz3162);
      zgaz334 = CONVERT_OF(mach_int, sail_int)(zgsz3163);
      KILL(sail_int)(&zgsz3163);
      KILL(sail_int)(&zgsz3162);
      KILL(sail_int)(&zgsz3161);
    }
    {
      sail_int zgsz3159;
      CREATE(sail_int)(&zgsz3159);
      CONVERT_OF(sail_int, mach_int)(&zgsz3159, zgaz334);
      lbits zgsz3160;
      CREATE(lbits)(&zgsz3160);
      sailgen_zzeros_implicit(&zgsz3160, zgsz3159);
      zgaz335 = CONVERT_OF(fbits, lbits)(zgsz3160, true);
      KILL(lbits)(&zgsz3160);
      KILL(sail_int)(&zgsz3159);
    }
  }
  {
    lbits zgsz3164;
    CREATE(lbits)(&zgsz3164);
    CONVERT_OF(lbits, fbits)(&zgsz3164, UINT64_C(0b01), UINT64_C(2) , true);
    lbits zgsz3165;
    CREATE(lbits)(&zgsz3165);
    CONVERT_OF(lbits, fbits)(&zgsz3165, zgaz335, UINT64_C(12) , true);
    lbits zgsz3166;
    CREATE(lbits)(&zgsz3166);
    append(&zgsz3166, zgsz3164, zgsz3165);
    zgsz329 = CONVERT_OF(fbits, lbits)(zgsz3166, true);
    KILL(lbits)(&zgsz3166);
    KILL(lbits)(&zgsz3165);
    KILL(lbits)(&zgsz3164);
  }

  zcap_reset_T = zgsz329;

let_end_75: ;
}
static void kill_letbind_24(void) {
}

static struct zCapability sailgen_undefined_Capability(unit);

static struct zCapability sailgen_undefined_Capability(unit zgsz330)
{
  __label__ end_function_77, end_block_exception_78;

  struct zCapability zcbz316;
  bool zgaz336;
  zgaz336 = undefined_bool(UNIT);
  uint64_t zgaz337;
  {
    sail_int zgsz3179;
    CREATE(sail_int)(&zgsz3179);
    CONVERT_OF(sail_int, mach_int)(&zgsz3179, INT64_C(4));
    lbits zgsz3180;
    CREATE(lbits)(&zgsz3180);
    UNDEFINED(lbits)(&zgsz3180, zgsz3179);
    zgaz337 = CONVERT_OF(fbits, lbits)(zgsz3180, true);
    KILL(lbits)(&zgsz3180);
    KILL(sail_int)(&zgsz3179);
  }
  bool zgaz338;
  zgaz338 = undefined_bool(UNIT);
  bool zgaz339;
  zgaz339 = undefined_bool(UNIT);
  bool zgaz340;
  zgaz340 = undefined_bool(UNIT);
  bool zgaz341;
  zgaz341 = undefined_bool(UNIT);
  bool zgaz342;
  zgaz342 = undefined_bool(UNIT);
  bool zgaz343;
  zgaz343 = undefined_bool(UNIT);
  bool zgaz344;
  zgaz344 = undefined_bool(UNIT);
  bool zgaz345;
  zgaz345 = undefined_bool(UNIT);
  bool zgaz346;
  zgaz346 = undefined_bool(UNIT);
  bool zgaz347;
  zgaz347 = undefined_bool(UNIT);
  bool zgaz348;
  zgaz348 = undefined_bool(UNIT);
  bool zgaz349;
  zgaz349 = undefined_bool(UNIT);
  uint64_t zgaz350;
  {
    sail_int zgsz3177;
    CREATE(sail_int)(&zgsz3177);
    CONVERT_OF(sail_int, mach_int)(&zgsz3177, INT64_C(2));
    lbits zgsz3178;
    CREATE(lbits)(&zgsz3178);
    UNDEFINED(lbits)(&zgsz3178, zgsz3177);
    zgaz350 = CONVERT_OF(fbits, lbits)(zgsz3178, true);
    KILL(lbits)(&zgsz3178);
    KILL(sail_int)(&zgsz3177);
  }
  bool zgaz351;
  zgaz351 = undefined_bool(UNIT);
  bool zgaz352;
  zgaz352 = undefined_bool(UNIT);
  uint64_t zgaz353;
  {
    sail_int zgsz3175;
    CREATE(sail_int)(&zgsz3175);
    CONVERT_OF(sail_int, mach_int)(&zgsz3175, INT64_C(6));
    lbits zgsz3176;
    CREATE(lbits)(&zgsz3176);
    UNDEFINED(lbits)(&zgsz3176, zgsz3175);
    zgaz353 = CONVERT_OF(fbits, lbits)(zgsz3176, true);
    KILL(lbits)(&zgsz3176);
    KILL(sail_int)(&zgsz3175);
  }
  uint64_t zgaz354;
  {
    sail_int zgsz3173;
    CREATE(sail_int)(&zgsz3173);
    CONVERT_OF(sail_int, mach_int)(&zgsz3173, INT64_C(14));
    lbits zgsz3174;
    CREATE(lbits)(&zgsz3174);
    UNDEFINED(lbits)(&zgsz3174, zgsz3173);
    zgaz354 = CONVERT_OF(fbits, lbits)(zgsz3174, true);
    KILL(lbits)(&zgsz3174);
    KILL(sail_int)(&zgsz3173);
  }
  uint64_t zgaz355;
  {
    sail_int zgsz3171;
    CREATE(sail_int)(&zgsz3171);
    CONVERT_OF(sail_int, mach_int)(&zgsz3171, INT64_C(14));
    lbits zgsz3172;
    CREATE(lbits)(&zgsz3172);
    UNDEFINED(lbits)(&zgsz3172, zgsz3171);
    zgaz355 = CONVERT_OF(fbits, lbits)(zgsz3172, true);
    KILL(lbits)(&zgsz3172);
    KILL(sail_int)(&zgsz3171);
  }
  uint64_t zgaz356;
  {
    sail_int zgsz3169;
    CREATE(sail_int)(&zgsz3169);
    CONVERT_OF(sail_int, mach_int)(&zgsz3169, INT64_C(18));
    lbits zgsz3170;
    CREATE(lbits)(&zgsz3170);
    UNDEFINED(lbits)(&zgsz3170, zgsz3169);
    zgaz356 = CONVERT_OF(fbits, lbits)(zgsz3170, true);
    KILL(lbits)(&zgsz3170);
    KILL(sail_int)(&zgsz3169);
  }
  uint64_t zgaz357;
  {
    sail_int zgsz3167;
    CREATE(sail_int)(&zgsz3167);
    CONVERT_OF(sail_int, mach_int)(&zgsz3167, INT64_C(64));
    lbits zgsz3168;
    CREATE(lbits)(&zgsz3168);
    UNDEFINED(lbits)(&zgsz3168, zgsz3167);
    zgaz357 = CONVERT_OF(fbits, lbits)(zgsz3168, true);
    KILL(lbits)(&zgsz3168);
    KILL(sail_int)(&zgsz3167);
  }
  struct zCapability zgsz331;
  zgsz331.zB = zgaz354;
  zgsz331.zE = zgaz353;
  zgsz331.zT = zgaz355;
  zgsz331.zaccess_system_regs = zgaz339;
  zgsz331.zaddress = zgaz357;
  zgsz331.zflag_cap_mode = zgaz351;
  zgsz331.zglobal = zgaz349;
  zgsz331.zinternal_E = zgaz352;
  zgsz331.zotype = zgaz356;
  zgsz331.zpermit_cinvoke = zgaz341;
  zgsz331.zpermit_execute = zgaz348;
  zgsz331.zpermit_load = zgaz347;
  zgsz331.zpermit_load_cap = zgaz345;
  zgsz331.zpermit_seal = zgaz342;
  zgsz331.zpermit_set_CID = zgaz338;
  zgsz331.zpermit_store = zgaz346;
  zgsz331.zpermit_store_cap = zgaz344;
  zgsz331.zpermit_store_local_cap = zgaz343;
  zgsz331.zpermit_unseal = zgaz340;
  zgsz331.zreserved = zgaz350;
  zgsz331.ztag = zgaz336;
  zgsz331.zuperms = zgaz337;
  zcbz316 = zgsz331;























end_function_77: ;
  return zcbz316;
end_block_exception_78: ;
  struct zCapability zcbz347 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz347;
}

static struct zCapability znull_cap;
static void create_letbind_25(void) {


  struct zCapability zgsz333;
  uint64_t zgaz358;
  {
    sail_int zgsz3190;
    CREATE(sail_int)(&zgsz3190);
    CONVERT_OF(sail_int, mach_int)(&zgsz3190, INT64_C(4));
    lbits zgsz3191;
    CREATE(lbits)(&zgsz3191);
    sailgen_zzeros_implicit(&zgsz3191, zgsz3190);
    zgaz358 = CONVERT_OF(fbits, lbits)(zgsz3191, true);
    KILL(lbits)(&zgsz3191);
    KILL(sail_int)(&zgsz3190);
  }
  uint64_t zgaz359;
  {
    sail_int zgsz3188;
    CREATE(sail_int)(&zgsz3188);
    CONVERT_OF(sail_int, mach_int)(&zgsz3188, INT64_C(2));
    lbits zgsz3189;
    CREATE(lbits)(&zgsz3189);
    sailgen_zzeros_implicit(&zgsz3189, zgsz3188);
    zgaz359 = CONVERT_OF(fbits, lbits)(zgsz3189, true);
    KILL(lbits)(&zgsz3189);
    KILL(sail_int)(&zgsz3188);
  }
  uint64_t zgaz360;
  {
    sail_int zgsz3186;
    CREATE(sail_int)(&zgsz3186);
    CONVERT_OF(sail_int, mach_int)(&zgsz3186, INT64_C(14));
    lbits zgsz3187;
    CREATE(lbits)(&zgsz3187);
    sailgen_zzeros_implicit(&zgsz3187, zgsz3186);
    zgaz360 = CONVERT_OF(fbits, lbits)(zgsz3187, true);
    KILL(lbits)(&zgsz3187);
    KILL(sail_int)(&zgsz3186);
  }
  uint64_t zgaz361;
  {
    sail_int zgsz3183;
    CREATE(sail_int)(&zgsz3183);
    CONVERT_OF(sail_int, mach_int)(&zgsz3183, zcap_otype_width);
    sail_int zgsz3184;
    CREATE(sail_int)(&zgsz3184);
    CONVERT_OF(sail_int, mach_int)(&zgsz3184, zotype_unsealed);
    lbits zgsz3185;
    CREATE(lbits)(&zgsz3185);
    sailgen_to_bits(&zgsz3185, zgsz3183, zgsz3184);
    zgaz361 = CONVERT_OF(fbits, lbits)(zgsz3185, true);
    KILL(lbits)(&zgsz3185);
    KILL(sail_int)(&zgsz3184);
    KILL(sail_int)(&zgsz3183);
  }
  uint64_t zgaz362;
  {
    sail_int zgsz3181;
    CREATE(sail_int)(&zgsz3181);
    CONVERT_OF(sail_int, mach_int)(&zgsz3181, INT64_C(64));
    lbits zgsz3182;
    CREATE(lbits)(&zgsz3182);
    sailgen_zzeros_implicit(&zgsz3182, zgsz3181);
    zgaz362 = CONVERT_OF(fbits, lbits)(zgsz3182, true);
    KILL(lbits)(&zgsz3182);
    KILL(sail_int)(&zgsz3181);
  }
  struct zCapability zgsz332;
  zgsz332.zB = zgaz360;
  zgsz332.zE = zcap_reset_E;
  zgsz332.zT = zcap_reset_T;
  zgsz332.zaccess_system_regs = false;
  zgsz332.zaddress = zgaz362;
  zgsz332.zflag_cap_mode = false;
  zgsz332.zglobal = false;
  zgsz332.zinternal_E = true;
  zgsz332.zotype = zgaz361;
  zgsz332.zpermit_cinvoke = false;
  zgsz332.zpermit_execute = false;
  zgsz332.zpermit_load = false;
  zgsz332.zpermit_load_cap = false;
  zgsz332.zpermit_seal = false;
  zgsz332.zpermit_set_CID = false;
  zgsz332.zpermit_store = false;
  zgsz332.zpermit_store_cap = false;
  zgsz332.zpermit_store_local_cap = false;
  zgsz332.zpermit_unseal = false;
  zgsz332.zreserved = zgaz359;
  zgsz332.ztag = false;
  zgsz332.zuperms = zgaz358;
  zgsz333 = zgsz332;






  znull_cap = zgsz333;

let_end_79: ;
}
static void kill_letbind_25(void) {
}

static struct zCapability zdefault_cap;
static void create_letbind_26(void) {


  struct zCapability zgsz335;
  uint64_t zgaz363;
  {
    sail_int zgsz3201;
    CREATE(sail_int)(&zgsz3201);
    CONVERT_OF(sail_int, mach_int)(&zgsz3201, INT64_C(4));
    lbits zgsz3202;
    CREATE(lbits)(&zgsz3202);
    sailgen_ones(&zgsz3202, zgsz3201);
    zgaz363 = CONVERT_OF(fbits, lbits)(zgsz3202, true);
    KILL(lbits)(&zgsz3202);
    KILL(sail_int)(&zgsz3201);
  }
  uint64_t zgaz364;
  {
    sail_int zgsz3199;
    CREATE(sail_int)(&zgsz3199);
    CONVERT_OF(sail_int, mach_int)(&zgsz3199, INT64_C(2));
    lbits zgsz3200;
    CREATE(lbits)(&zgsz3200);
    sailgen_zzeros_implicit(&zgsz3200, zgsz3199);
    zgaz364 = CONVERT_OF(fbits, lbits)(zgsz3200, true);
    KILL(lbits)(&zgsz3200);
    KILL(sail_int)(&zgsz3199);
  }
  uint64_t zgaz365;
  {
    sail_int zgsz3197;
    CREATE(sail_int)(&zgsz3197);
    CONVERT_OF(sail_int, mach_int)(&zgsz3197, INT64_C(14));
    lbits zgsz3198;
    CREATE(lbits)(&zgsz3198);
    sailgen_zzeros_implicit(&zgsz3198, zgsz3197);
    zgaz365 = CONVERT_OF(fbits, lbits)(zgsz3198, true);
    KILL(lbits)(&zgsz3198);
    KILL(sail_int)(&zgsz3197);
  }
  uint64_t zgaz366;
  {
    sail_int zgsz3194;
    CREATE(sail_int)(&zgsz3194);
    CONVERT_OF(sail_int, mach_int)(&zgsz3194, zcap_otype_width);
    sail_int zgsz3195;
    CREATE(sail_int)(&zgsz3195);
    CONVERT_OF(sail_int, mach_int)(&zgsz3195, zotype_unsealed);
    lbits zgsz3196;
    CREATE(lbits)(&zgsz3196);
    sailgen_to_bits(&zgsz3196, zgsz3194, zgsz3195);
    zgaz366 = CONVERT_OF(fbits, lbits)(zgsz3196, true);
    KILL(lbits)(&zgsz3196);
    KILL(sail_int)(&zgsz3195);
    KILL(sail_int)(&zgsz3194);
  }
  uint64_t zgaz367;
  {
    sail_int zgsz3192;
    CREATE(sail_int)(&zgsz3192);
    CONVERT_OF(sail_int, mach_int)(&zgsz3192, INT64_C(64));
    lbits zgsz3193;
    CREATE(lbits)(&zgsz3193);
    sailgen_zzeros_implicit(&zgsz3193, zgsz3192);
    zgaz367 = CONVERT_OF(fbits, lbits)(zgsz3193, true);
    KILL(lbits)(&zgsz3193);
    KILL(sail_int)(&zgsz3192);
  }
  struct zCapability zgsz334;
  zgsz334.zB = zgaz365;
  zgsz334.zE = zcap_reset_E;
  zgsz334.zT = zcap_reset_T;
  zgsz334.zaccess_system_regs = true;
  zgsz334.zaddress = zgaz367;
  zgsz334.zflag_cap_mode = false;
  zgsz334.zglobal = true;
  zgsz334.zinternal_E = true;
  zgsz334.zotype = zgaz366;
  zgsz334.zpermit_cinvoke = true;
  zgsz334.zpermit_execute = true;
  zgsz334.zpermit_load = true;
  zgsz334.zpermit_load_cap = true;
  zgsz334.zpermit_seal = true;
  zgsz334.zpermit_set_CID = true;
  zgsz334.zpermit_store = true;
  zgsz334.zpermit_store_cap = true;
  zgsz334.zpermit_store_local_cap = true;
  zgsz334.zpermit_unseal = true;
  zgsz334.zreserved = zgaz364;
  zgsz334.ztag = true;
  zgsz334.zuperms = zgaz363;
  zgsz335 = zgsz334;






  zdefault_cap = zgsz335;

let_end_80: ;
}
static void kill_letbind_26(void) {
}

static uint64_t sailgen_getCapHardPerms(struct zCapability);

static uint64_t sailgen_getCapHardPerms(struct zCapability zcap)
{
  __label__ end_function_82, end_block_exception_83;

  uint64_t zcbz317;
  uint64_t zgaz3100;
  {
    bool zgaz368;
    zgaz368 = zcap.zpermit_set_CID;
    zgaz3100 = sailgen_bool_to_bits(zgaz368);
  }
  uint64_t zgaz3101;
  {
    uint64_t zgaz398;
    {
      bool zgaz369;
      zgaz369 = zcap.zaccess_system_regs;
      zgaz398 = sailgen_bool_to_bits(zgaz369);
    }
    uint64_t zgaz399;
    {
      uint64_t zgaz396;
      {
        bool zgaz370;
        zgaz370 = zcap.zpermit_unseal;
        zgaz396 = sailgen_bool_to_bits(zgaz370);
      }
      uint64_t zgaz397;
      {
        uint64_t zgaz394;
        {
          bool zgaz371;
          zgaz371 = zcap.zpermit_cinvoke;
          zgaz394 = sailgen_bool_to_bits(zgaz371);
        }
        uint64_t zgaz395;
        {
          uint64_t zgaz392;
          {
            bool zgaz372;
            zgaz372 = zcap.zpermit_seal;
            zgaz392 = sailgen_bool_to_bits(zgaz372);
          }
          uint64_t zgaz393;
          {
            uint64_t zgaz390;
            {
              bool zgaz373;
              zgaz373 = zcap.zpermit_store_local_cap;
              zgaz390 = sailgen_bool_to_bits(zgaz373);
            }
            uint64_t zgaz391;
            {
              uint64_t zgaz388;
              {
                bool zgaz374;
                zgaz374 = zcap.zpermit_store_cap;
                zgaz388 = sailgen_bool_to_bits(zgaz374);
              }
              uint64_t zgaz389;
              {
                uint64_t zgaz386;
                {
                  bool zgaz375;
                  zgaz375 = zcap.zpermit_load_cap;
                  zgaz386 = sailgen_bool_to_bits(zgaz375);
                }
                uint64_t zgaz387;
                {
                  uint64_t zgaz384;
                  {
                    bool zgaz376;
                    zgaz376 = zcap.zpermit_store;
                    zgaz384 = sailgen_bool_to_bits(zgaz376);
                  }
                  uint64_t zgaz385;
                  {
                    uint64_t zgaz382;
                    {
                      bool zgaz377;
                      zgaz377 = zcap.zpermit_load;
                      zgaz382 = sailgen_bool_to_bits(zgaz377);
                    }
                    uint64_t zgaz383;
                    {
                      uint64_t zgaz380;
                      {
                        bool zgaz378;
                        zgaz378 = zcap.zpermit_execute;
                        zgaz380 = sailgen_bool_to_bits(zgaz378);
                      }
                      uint64_t zgaz381;
                      {
                        bool zgaz379;
                        zgaz379 = zcap.zglobal;
                        zgaz381 = sailgen_bool_to_bits(zgaz379);
                      }
                      {
                        lbits zgsz3203;
                        CREATE(lbits)(&zgsz3203);
                        CONVERT_OF(lbits, fbits)(&zgsz3203, zgaz380, UINT64_C(1) , true);
                        lbits zgsz3204;
                        CREATE(lbits)(&zgsz3204);
                        CONVERT_OF(lbits, fbits)(&zgsz3204, zgaz381, UINT64_C(1) , true);
                        lbits zgsz3205;
                        CREATE(lbits)(&zgsz3205);
                        append(&zgsz3205, zgsz3203, zgsz3204);
                        zgaz383 = CONVERT_OF(fbits, lbits)(zgsz3205, true);
                        KILL(lbits)(&zgsz3205);
                        KILL(lbits)(&zgsz3204);
                        KILL(lbits)(&zgsz3203);
                      }
                    }
                    {
                      lbits zgsz3206;
                      CREATE(lbits)(&zgsz3206);
                      CONVERT_OF(lbits, fbits)(&zgsz3206, zgaz382, UINT64_C(1) , true);
                      lbits zgsz3207;
                      CREATE(lbits)(&zgsz3207);
                      CONVERT_OF(lbits, fbits)(&zgsz3207, zgaz383, UINT64_C(2) , true);
                      lbits zgsz3208;
                      CREATE(lbits)(&zgsz3208);
                      append(&zgsz3208, zgsz3206, zgsz3207);
                      zgaz385 = CONVERT_OF(fbits, lbits)(zgsz3208, true);
                      KILL(lbits)(&zgsz3208);
                      KILL(lbits)(&zgsz3207);
                      KILL(lbits)(&zgsz3206);
                    }
                  }
                  {
                    lbits zgsz3209;
                    CREATE(lbits)(&zgsz3209);
                    CONVERT_OF(lbits, fbits)(&zgsz3209, zgaz384, UINT64_C(1) , true);
                    lbits zgsz3210;
                    CREATE(lbits)(&zgsz3210);
                    CONVERT_OF(lbits, fbits)(&zgsz3210, zgaz385, UINT64_C(3) , true);
                    lbits zgsz3211;
                    CREATE(lbits)(&zgsz3211);
                    append(&zgsz3211, zgsz3209, zgsz3210);
                    zgaz387 = CONVERT_OF(fbits, lbits)(zgsz3211, true);
                    KILL(lbits)(&zgsz3211);
                    KILL(lbits)(&zgsz3210);
                    KILL(lbits)(&zgsz3209);
                  }
                }
                {
                  lbits zgsz3212;
                  CREATE(lbits)(&zgsz3212);
                  CONVERT_OF(lbits, fbits)(&zgsz3212, zgaz386, UINT64_C(1) , true);
                  lbits zgsz3213;
                  CREATE(lbits)(&zgsz3213);
                  CONVERT_OF(lbits, fbits)(&zgsz3213, zgaz387, UINT64_C(4) , true);
                  lbits zgsz3214;
                  CREATE(lbits)(&zgsz3214);
                  append(&zgsz3214, zgsz3212, zgsz3213);
                  zgaz389 = CONVERT_OF(fbits, lbits)(zgsz3214, true);
                  KILL(lbits)(&zgsz3214);
                  KILL(lbits)(&zgsz3213);
                  KILL(lbits)(&zgsz3212);
                }
              }
              {
                lbits zgsz3215;
                CREATE(lbits)(&zgsz3215);
                CONVERT_OF(lbits, fbits)(&zgsz3215, zgaz388, UINT64_C(1) , true);
                lbits zgsz3216;
                CREATE(lbits)(&zgsz3216);
                CONVERT_OF(lbits, fbits)(&zgsz3216, zgaz389, UINT64_C(5) , true);
                lbits zgsz3217;
                CREATE(lbits)(&zgsz3217);
                append(&zgsz3217, zgsz3215, zgsz3216);
                zgaz391 = CONVERT_OF(fbits, lbits)(zgsz3217, true);
                KILL(lbits)(&zgsz3217);
                KILL(lbits)(&zgsz3216);
                KILL(lbits)(&zgsz3215);
              }
            }
            {
              lbits zgsz3218;
              CREATE(lbits)(&zgsz3218);
              CONVERT_OF(lbits, fbits)(&zgsz3218, zgaz390, UINT64_C(1) , true);
              lbits zgsz3219;
              CREATE(lbits)(&zgsz3219);
              CONVERT_OF(lbits, fbits)(&zgsz3219, zgaz391, UINT64_C(6) , true);
              lbits zgsz3220;
              CREATE(lbits)(&zgsz3220);
              append(&zgsz3220, zgsz3218, zgsz3219);
              zgaz393 = CONVERT_OF(fbits, lbits)(zgsz3220, true);
              KILL(lbits)(&zgsz3220);
              KILL(lbits)(&zgsz3219);
              KILL(lbits)(&zgsz3218);
            }
          }
          {
            lbits zgsz3221;
            CREATE(lbits)(&zgsz3221);
            CONVERT_OF(lbits, fbits)(&zgsz3221, zgaz392, UINT64_C(1) , true);
            lbits zgsz3222;
            CREATE(lbits)(&zgsz3222);
            CONVERT_OF(lbits, fbits)(&zgsz3222, zgaz393, UINT64_C(7) , true);
            lbits zgsz3223;
            CREATE(lbits)(&zgsz3223);
            append(&zgsz3223, zgsz3221, zgsz3222);
            zgaz395 = CONVERT_OF(fbits, lbits)(zgsz3223, true);
            KILL(lbits)(&zgsz3223);
            KILL(lbits)(&zgsz3222);
            KILL(lbits)(&zgsz3221);
          }
        }
        {
          lbits zgsz3224;
          CREATE(lbits)(&zgsz3224);
          CONVERT_OF(lbits, fbits)(&zgsz3224, zgaz394, UINT64_C(1) , true);
          lbits zgsz3225;
          CREATE(lbits)(&zgsz3225);
          CONVERT_OF(lbits, fbits)(&zgsz3225, zgaz395, UINT64_C(8) , true);
          lbits zgsz3226;
          CREATE(lbits)(&zgsz3226);
          append(&zgsz3226, zgsz3224, zgsz3225);
          zgaz397 = CONVERT_OF(fbits, lbits)(zgsz3226, true);
          KILL(lbits)(&zgsz3226);
          KILL(lbits)(&zgsz3225);
          KILL(lbits)(&zgsz3224);
        }
      }
      {
        lbits zgsz3227;
        CREATE(lbits)(&zgsz3227);
        CONVERT_OF(lbits, fbits)(&zgsz3227, zgaz396, UINT64_C(1) , true);
        lbits zgsz3228;
        CREATE(lbits)(&zgsz3228);
        CONVERT_OF(lbits, fbits)(&zgsz3228, zgaz397, UINT64_C(9) , true);
        lbits zgsz3229;
        CREATE(lbits)(&zgsz3229);
        append(&zgsz3229, zgsz3227, zgsz3228);
        zgaz399 = CONVERT_OF(fbits, lbits)(zgsz3229, true);
        KILL(lbits)(&zgsz3229);
        KILL(lbits)(&zgsz3228);
        KILL(lbits)(&zgsz3227);
      }
    }
    {
      lbits zgsz3230;
      CREATE(lbits)(&zgsz3230);
      CONVERT_OF(lbits, fbits)(&zgsz3230, zgaz398, UINT64_C(1) , true);
      lbits zgsz3231;
      CREATE(lbits)(&zgsz3231);
      CONVERT_OF(lbits, fbits)(&zgsz3231, zgaz399, UINT64_C(10) , true);
      lbits zgsz3232;
      CREATE(lbits)(&zgsz3232);
      append(&zgsz3232, zgsz3230, zgsz3231);
      zgaz3101 = CONVERT_OF(fbits, lbits)(zgsz3232, true);
      KILL(lbits)(&zgsz3232);
      KILL(lbits)(&zgsz3231);
      KILL(lbits)(&zgsz3230);
    }
  }
  {
    lbits zgsz3233;
    CREATE(lbits)(&zgsz3233);
    CONVERT_OF(lbits, fbits)(&zgsz3233, zgaz3100, UINT64_C(1) , true);
    lbits zgsz3234;
    CREATE(lbits)(&zgsz3234);
    CONVERT_OF(lbits, fbits)(&zgsz3234, zgaz3101, UINT64_C(11) , true);
    lbits zgsz3235;
    CREATE(lbits)(&zgsz3235);
    append(&zgsz3235, zgsz3233, zgsz3234);
    zcbz317 = CONVERT_OF(fbits, lbits)(zgsz3235, true);
    KILL(lbits)(&zgsz3235);
    KILL(lbits)(&zgsz3234);
    KILL(lbits)(&zgsz3233);
  }


end_function_82: ;
  return zcbz317;
end_block_exception_83: ;

  return UINT64_C(0xdeadc0de);
}

static struct zCapability sailgen_encCapabilityToCapability(bool, struct zEncCapability);

static struct zCapability sailgen_encCapabilityToCapability(bool zt, struct zEncCapability zc)
{
  __label__ cleanup_86, end_cleanup_87, end_function_85, end_block_exception_88;

  struct zCapability zcbz318;
  bool zinternal_E;
  {
    fbits zgaz3173;
    zgaz3173 = zc.zinternal_E;
    zinternal_E = sailgen_bit_to_bool(zgaz3173);
  }
  uint64_t zE;
  {
    sail_int zgsz3352;
    CREATE(sail_int)(&zgsz3352);
    CONVERT_OF(sail_int, mach_int)(&zgsz3352, INT64_C(6));
    lbits zgsz3353;
    CREATE(lbits)(&zgsz3353);
    sailgen_zzeros_implicit(&zgsz3353, zgsz3352);
    zE = CONVERT_OF(fbits, lbits)(zgsz3353, true);
    KILL(lbits)(&zgsz3353);
    KILL(sail_int)(&zgsz3352);
  }
  uint64_t zBs;
  {
    sail_int zgsz3350;
    CREATE(sail_int)(&zgsz3350);
    CONVERT_OF(sail_int, mach_int)(&zgsz3350, INT64_C(14));
    lbits zgsz3351;
    CREATE(lbits)(&zgsz3351);
    sailgen_zzeros_implicit(&zgsz3351, zgsz3350);
    zBs = CONVERT_OF(fbits, lbits)(zgsz3351, true);
    KILL(lbits)(&zgsz3351);
    KILL(sail_int)(&zgsz3350);
  }
  uint64_t zT;
  {
    int64_t zgaz3172;
    {
      sail_int zgsz3238;
      CREATE(sail_int)(&zgsz3238);
      CONVERT_OF(sail_int, mach_int)(&zgsz3238, INT64_C(14));
      sail_int zgsz3239;
      CREATE(sail_int)(&zgsz3239);
      CONVERT_OF(sail_int, mach_int)(&zgsz3239, INT64_C(2));
      sail_int zgsz3240;
      CREATE(sail_int)(&zgsz3240);
      sub_int(&zgsz3240, zgsz3238, zgsz3239);
      zgaz3172 = CONVERT_OF(mach_int, sail_int)(zgsz3240);
      KILL(sail_int)(&zgsz3240);
      KILL(sail_int)(&zgsz3239);
      KILL(sail_int)(&zgsz3238);
    }
    {
      sail_int zgsz3236;
      CREATE(sail_int)(&zgsz3236);
      CONVERT_OF(sail_int, mach_int)(&zgsz3236, zgaz3172);
      lbits zgsz3237;
      CREATE(lbits)(&zgsz3237);
      sailgen_zzeros_implicit(&zgsz3237, zgsz3236);
      zT = CONVERT_OF(fbits, lbits)(zgsz3237, true);
      KILL(lbits)(&zgsz3237);
      KILL(sail_int)(&zgsz3236);
    }
  }
  uint64_t zlenMSBs;
  {
    sail_int zgsz3348;
    CREATE(sail_int)(&zgsz3348);
    CONVERT_OF(sail_int, mach_int)(&zgsz3348, INT64_C(2));
    lbits zgsz3349;
    CREATE(lbits)(&zgsz3349);
    sailgen_zzeros_implicit(&zgsz3349, zgsz3348);
    zlenMSBs = CONVERT_OF(fbits, lbits)(zgsz3349, true);
    KILL(lbits)(&zgsz3349);
    KILL(sail_int)(&zgsz3348);
  }
  {
    unit zgsz336;
    if (zinternal_E) {
      {
        uint64_t zgaz3106;
        {
          uint64_t zgaz3102;
          zgaz3102 = zc.zT;
          int64_t zgaz3103;
          {
            sail_int zgsz3245;
            CREATE(sail_int)(&zgsz3245);
            CONVERT_OF(sail_int, mach_int)(&zgsz3245, zinternal_E_take_bits);
            sail_int zgsz3246;
            CREATE(sail_int)(&zgsz3246);
            CONVERT_OF(sail_int, mach_int)(&zgsz3246, INT64_C(1));
            sail_int zgsz3247;
            CREATE(sail_int)(&zgsz3247);
            sub_int(&zgsz3247, zgsz3245, zgsz3246);
            zgaz3103 = CONVERT_OF(mach_int, sail_int)(zgsz3247);
            KILL(sail_int)(&zgsz3247);
            KILL(sail_int)(&zgsz3246);
            KILL(sail_int)(&zgsz3245);
          }
          {
            lbits zgsz3241;
            CREATE(lbits)(&zgsz3241);
            CONVERT_OF(lbits, fbits)(&zgsz3241, zgaz3102, UINT64_C(12) , true);
            sail_int zgsz3242;
            CREATE(sail_int)(&zgsz3242);
            CONVERT_OF(sail_int, mach_int)(&zgsz3242, zgaz3103);
            sail_int zgsz3243;
            CREATE(sail_int)(&zgsz3243);
            CONVERT_OF(sail_int, mach_int)(&zgsz3243, INT64_C(0));
            lbits zgsz3244;
            CREATE(lbits)(&zgsz3244);
            vector_subrange_lbits(&zgsz3244, zgsz3241, zgsz3242, zgsz3243);
            zgaz3106 = CONVERT_OF(fbits, lbits)(zgsz3244, true);
            KILL(lbits)(&zgsz3244);
            KILL(sail_int)(&zgsz3243);
            KILL(sail_int)(&zgsz3242);
            KILL(lbits)(&zgsz3241);
          }
        }
        uint64_t zgaz3107;
        {
          uint64_t zgaz3104;
          zgaz3104 = zc.zB;
          int64_t zgaz3105;
          {
            sail_int zgsz3252;
            CREATE(sail_int)(&zgsz3252);
            CONVERT_OF(sail_int, mach_int)(&zgsz3252, zinternal_E_take_bits);
            sail_int zgsz3253;
            CREATE(sail_int)(&zgsz3253);
            CONVERT_OF(sail_int, mach_int)(&zgsz3253, INT64_C(1));
            sail_int zgsz3254;
            CREATE(sail_int)(&zgsz3254);
            sub_int(&zgsz3254, zgsz3252, zgsz3253);
            zgaz3105 = CONVERT_OF(mach_int, sail_int)(zgsz3254);
            KILL(sail_int)(&zgsz3254);
            KILL(sail_int)(&zgsz3253);
            KILL(sail_int)(&zgsz3252);
          }
          {
            lbits zgsz3248;
            CREATE(lbits)(&zgsz3248);
            CONVERT_OF(lbits, fbits)(&zgsz3248, zgaz3104, UINT64_C(14) , true);
            sail_int zgsz3249;
            CREATE(sail_int)(&zgsz3249);
            CONVERT_OF(sail_int, mach_int)(&zgsz3249, zgaz3105);
            sail_int zgsz3250;
            CREATE(sail_int)(&zgsz3250);
            CONVERT_OF(sail_int, mach_int)(&zgsz3250, INT64_C(0));
            lbits zgsz3251;
            CREATE(lbits)(&zgsz3251);
            vector_subrange_lbits(&zgsz3251, zgsz3248, zgsz3249, zgsz3250);
            zgaz3107 = CONVERT_OF(fbits, lbits)(zgsz3251, true);
            KILL(lbits)(&zgsz3251);
            KILL(sail_int)(&zgsz3250);
            KILL(sail_int)(&zgsz3249);
            KILL(lbits)(&zgsz3248);
          }
        }
        {
          lbits zgsz3255;
          CREATE(lbits)(&zgsz3255);
          CONVERT_OF(lbits, fbits)(&zgsz3255, zgaz3106, UINT64_C(3) , true);
          lbits zgsz3256;
          CREATE(lbits)(&zgsz3256);
          CONVERT_OF(lbits, fbits)(&zgsz3256, zgaz3107, UINT64_C(3) , true);
          lbits zgsz3257;
          CREATE(lbits)(&zgsz3257);
          append(&zgsz3257, zgsz3255, zgsz3256);
          zE = CONVERT_OF(fbits, lbits)(zgsz3257, true);
          KILL(lbits)(&zgsz3257);
          KILL(lbits)(&zgsz3256);
          KILL(lbits)(&zgsz3255);
        }
        unit zgsz341;
        zgsz341 = UNIT;
      }
      {
        zlenMSBs = UINT64_C(0b01);
        unit zgsz340;
        zgsz340 = UNIT;
      }
      {
        uint64_t zgaz3110;
        {
          uint64_t zgaz3108;
          zgaz3108 = zc.zT;
          int64_t zgaz3109;
          {
            sail_int zgsz3262;
            CREATE(sail_int)(&zgsz3262);
            CONVERT_OF(sail_int, mach_int)(&zgsz3262, zcap_mantissa_width);
            sail_int zgsz3263;
            CREATE(sail_int)(&zgsz3263);
            CONVERT_OF(sail_int, mach_int)(&zgsz3263, INT64_C(3));
            sail_int zgsz3264;
            CREATE(sail_int)(&zgsz3264);
            sub_int(&zgsz3264, zgsz3262, zgsz3263);
            zgaz3109 = CONVERT_OF(mach_int, sail_int)(zgsz3264);
            KILL(sail_int)(&zgsz3264);
            KILL(sail_int)(&zgsz3263);
            KILL(sail_int)(&zgsz3262);
          }
          {
            lbits zgsz3258;
            CREATE(lbits)(&zgsz3258);
            CONVERT_OF(lbits, fbits)(&zgsz3258, zgaz3108, UINT64_C(12) , true);
            sail_int zgsz3259;
            CREATE(sail_int)(&zgsz3259);
            CONVERT_OF(sail_int, mach_int)(&zgsz3259, zgaz3109);
            sail_int zgsz3260;
            CREATE(sail_int)(&zgsz3260);
            CONVERT_OF(sail_int, mach_int)(&zgsz3260, INT64_C(3));
            lbits zgsz3261;
            CREATE(lbits)(&zgsz3261);
            vector_subrange_lbits(&zgsz3261, zgsz3258, zgsz3259, zgsz3260);
            zgaz3110 = CONVERT_OF(fbits, lbits)(zgsz3261, true);
            KILL(lbits)(&zgsz3261);
            KILL(sail_int)(&zgsz3260);
            KILL(sail_int)(&zgsz3259);
            KILL(lbits)(&zgsz3258);
          }
        }
        uint64_t zgaz3111;
        {
          sail_int zgsz3268;
          CREATE(sail_int)(&zgsz3268);
          CONVERT_OF(sail_int, mach_int)(&zgsz3268, zinternal_E_take_bits);
          lbits zgsz3269;
          CREATE(lbits)(&zgsz3269);
          sailgen_zzeros_implicit(&zgsz3269, zgsz3268);
          zgaz3111 = CONVERT_OF(fbits, lbits)(zgsz3269, true);
          KILL(lbits)(&zgsz3269);
          KILL(sail_int)(&zgsz3268);
        }
        {
          lbits zgsz3265;
          CREATE(lbits)(&zgsz3265);
          CONVERT_OF(lbits, fbits)(&zgsz3265, zgaz3110, UINT64_C(9) , true);
          lbits zgsz3266;
          CREATE(lbits)(&zgsz3266);
          CONVERT_OF(lbits, fbits)(&zgsz3266, zgaz3111, UINT64_C(3) , true);
          lbits zgsz3267;
          CREATE(lbits)(&zgsz3267);
          append(&zgsz3267, zgsz3265, zgsz3266);
          zT = CONVERT_OF(fbits, lbits)(zgsz3267, true);
          KILL(lbits)(&zgsz3267);
          KILL(lbits)(&zgsz3266);
          KILL(lbits)(&zgsz3265);
        }
        unit zgsz339;
        zgsz339 = UNIT;
      }
      uint64_t zgaz3114;
      {
        uint64_t zgaz3112;
        zgaz3112 = zc.zB;
        int64_t zgaz3113;
        {
          sail_int zgsz3274;
          CREATE(sail_int)(&zgsz3274);
          CONVERT_OF(sail_int, mach_int)(&zgsz3274, zcap_mantissa_width);
          sail_int zgsz3275;
          CREATE(sail_int)(&zgsz3275);
          CONVERT_OF(sail_int, mach_int)(&zgsz3275, INT64_C(1));
          sail_int zgsz3276;
          CREATE(sail_int)(&zgsz3276);
          sub_int(&zgsz3276, zgsz3274, zgsz3275);
          zgaz3113 = CONVERT_OF(mach_int, sail_int)(zgsz3276);
          KILL(sail_int)(&zgsz3276);
          KILL(sail_int)(&zgsz3275);
          KILL(sail_int)(&zgsz3274);
        }
        {
          lbits zgsz3270;
          CREATE(lbits)(&zgsz3270);
          CONVERT_OF(lbits, fbits)(&zgsz3270, zgaz3112, UINT64_C(14) , true);
          sail_int zgsz3271;
          CREATE(sail_int)(&zgsz3271);
          CONVERT_OF(sail_int, mach_int)(&zgsz3271, zgaz3113);
          sail_int zgsz3272;
          CREATE(sail_int)(&zgsz3272);
          CONVERT_OF(sail_int, mach_int)(&zgsz3272, INT64_C(3));
          lbits zgsz3273;
          CREATE(lbits)(&zgsz3273);
          vector_subrange_lbits(&zgsz3273, zgsz3270, zgsz3271, zgsz3272);
          zgaz3114 = CONVERT_OF(fbits, lbits)(zgsz3273, true);
          KILL(lbits)(&zgsz3273);
          KILL(sail_int)(&zgsz3272);
          KILL(sail_int)(&zgsz3271);
          KILL(lbits)(&zgsz3270);
        }
      }
      uint64_t zgaz3115;
      {
        sail_int zgsz3280;
        CREATE(sail_int)(&zgsz3280);
        CONVERT_OF(sail_int, mach_int)(&zgsz3280, zinternal_E_take_bits);
        lbits zgsz3281;
        CREATE(lbits)(&zgsz3281);
        sailgen_zzeros_implicit(&zgsz3281, zgsz3280);
        zgaz3115 = CONVERT_OF(fbits, lbits)(zgsz3281, true);
        KILL(lbits)(&zgsz3281);
        KILL(sail_int)(&zgsz3280);
      }
      {
        lbits zgsz3277;
        CREATE(lbits)(&zgsz3277);
        CONVERT_OF(lbits, fbits)(&zgsz3277, zgaz3114, UINT64_C(11) , true);
        lbits zgsz3278;
        CREATE(lbits)(&zgsz3278);
        CONVERT_OF(lbits, fbits)(&zgsz3278, zgaz3115, UINT64_C(3) , true);
        lbits zgsz3279;
        CREATE(lbits)(&zgsz3279);
        append(&zgsz3279, zgsz3277, zgsz3278);
        zBs = CONVERT_OF(fbits, lbits)(zgsz3279, true);
        KILL(lbits)(&zgsz3279);
        KILL(lbits)(&zgsz3278);
        KILL(lbits)(&zgsz3277);
      }
      zgsz336 = UNIT;
    } else {
      {
        zlenMSBs = UINT64_C(0b00);
        unit zgsz338;
        zgsz338 = UNIT;
      }
      {
        zT = zc.zT;
        unit zgsz337;
        zgsz337 = UNIT;
      }
      zBs = zc.zB;
      zgsz336 = UNIT;
    }
  }
  uint64_t zcarry_out;
  {
    bool zgaz3171;
    {
      uint64_t zgaz3170;
      {
        int64_t zgaz3169;
        {
          sail_int zgsz3286;
          CREATE(sail_int)(&zgsz3286);
          CONVERT_OF(sail_int, mach_int)(&zgsz3286, zcap_mantissa_width);
          sail_int zgsz3287;
          CREATE(sail_int)(&zgsz3287);
          CONVERT_OF(sail_int, mach_int)(&zgsz3287, INT64_C(3));
          sail_int zgsz3288;
          CREATE(sail_int)(&zgsz3288);
          sub_int(&zgsz3288, zgsz3286, zgsz3287);
          zgaz3169 = CONVERT_OF(mach_int, sail_int)(zgsz3288);
          KILL(sail_int)(&zgsz3288);
          KILL(sail_int)(&zgsz3287);
          KILL(sail_int)(&zgsz3286);
        }
        {
          lbits zgsz3282;
          CREATE(lbits)(&zgsz3282);
          CONVERT_OF(lbits, fbits)(&zgsz3282, zBs, UINT64_C(14) , true);
          sail_int zgsz3283;
          CREATE(sail_int)(&zgsz3283);
          CONVERT_OF(sail_int, mach_int)(&zgsz3283, zgaz3169);
          sail_int zgsz3284;
          CREATE(sail_int)(&zgsz3284);
          CONVERT_OF(sail_int, mach_int)(&zgsz3284, INT64_C(0));
          lbits zgsz3285;
          CREATE(lbits)(&zgsz3285);
          vector_subrange_lbits(&zgsz3285, zgsz3282, zgsz3283, zgsz3284);
          zgaz3170 = CONVERT_OF(fbits, lbits)(zgsz3285, true);
          KILL(lbits)(&zgsz3285);
          KILL(sail_int)(&zgsz3284);
          KILL(sail_int)(&zgsz3283);
          KILL(lbits)(&zgsz3282);
        }
      }
      {
        lbits zgsz3289;
        CREATE(lbits)(&zgsz3289);
        CONVERT_OF(lbits, fbits)(&zgsz3289, zT, UINT64_C(12) , true);
        lbits zgsz3290;
        CREATE(lbits)(&zgsz3290);
        CONVERT_OF(lbits, fbits)(&zgsz3290, zgaz3170, UINT64_C(12) , true);
        zgaz3171 = sailgen_z8operatorz0zI_uz9(zgsz3289, zgsz3290);
        KILL(lbits)(&zgsz3290);
        KILL(lbits)(&zgsz3289);
      }
    }
    if (zgaz3171) {  zcarry_out = UINT64_C(0b01);  } else {  zcarry_out = UINT64_C(0b00);  }
  }
  uint64_t zTtop2;
  {
    uint64_t zgaz3168;
    {
      uint64_t zgaz3167;
      {
        int64_t zgaz3165;
        {
          sail_int zgsz3298;
          CREATE(sail_int)(&zgsz3298);
          CONVERT_OF(sail_int, mach_int)(&zgsz3298, zcap_mantissa_width);
          sail_int zgsz3299;
          CREATE(sail_int)(&zgsz3299);
          CONVERT_OF(sail_int, mach_int)(&zgsz3299, INT64_C(1));
          sail_int zgsz3300;
          CREATE(sail_int)(&zgsz3300);
          sub_int(&zgsz3300, zgsz3298, zgsz3299);
          zgaz3165 = CONVERT_OF(mach_int, sail_int)(zgsz3300);
          KILL(sail_int)(&zgsz3300);
          KILL(sail_int)(&zgsz3299);
          KILL(sail_int)(&zgsz3298);
        }
        int64_t zgaz3166;
        {
          sail_int zgsz3295;
          CREATE(sail_int)(&zgsz3295);
          CONVERT_OF(sail_int, mach_int)(&zgsz3295, zcap_mantissa_width);
          sail_int zgsz3296;
          CREATE(sail_int)(&zgsz3296);
          CONVERT_OF(sail_int, mach_int)(&zgsz3296, INT64_C(2));
          sail_int zgsz3297;
          CREATE(sail_int)(&zgsz3297);
          sub_int(&zgsz3297, zgsz3295, zgsz3296);
          zgaz3166 = CONVERT_OF(mach_int, sail_int)(zgsz3297);
          KILL(sail_int)(&zgsz3297);
          KILL(sail_int)(&zgsz3296);
          KILL(sail_int)(&zgsz3295);
        }
        {
          lbits zgsz3291;
          CREATE(lbits)(&zgsz3291);
          CONVERT_OF(lbits, fbits)(&zgsz3291, zBs, UINT64_C(14) , true);
          sail_int zgsz3292;
          CREATE(sail_int)(&zgsz3292);
          CONVERT_OF(sail_int, mach_int)(&zgsz3292, zgaz3165);
          sail_int zgsz3293;
          CREATE(sail_int)(&zgsz3293);
          CONVERT_OF(sail_int, mach_int)(&zgsz3293, zgaz3166);
          lbits zgsz3294;
          CREATE(lbits)(&zgsz3294);
          vector_subrange_lbits(&zgsz3294, zgsz3291, zgsz3292, zgsz3293);
          zgaz3167 = CONVERT_OF(fbits, lbits)(zgsz3294, true);
          KILL(lbits)(&zgsz3294);
          KILL(sail_int)(&zgsz3293);
          KILL(sail_int)(&zgsz3292);
          KILL(lbits)(&zgsz3291);
        }
      }
      {
        lbits zgsz3301;
        CREATE(lbits)(&zgsz3301);
        CONVERT_OF(lbits, fbits)(&zgsz3301, zgaz3167, UINT64_C(2) , true);
        lbits zgsz3302;
        CREATE(lbits)(&zgsz3302);
        CONVERT_OF(lbits, fbits)(&zgsz3302, zlenMSBs, UINT64_C(2) , true);
        lbits zgsz3303;
        CREATE(lbits)(&zgsz3303);
        add_bits(&zgsz3303, zgsz3301, zgsz3302);
        zgaz3168 = CONVERT_OF(fbits, lbits)(zgsz3303, true);
        KILL(lbits)(&zgsz3303);
        KILL(lbits)(&zgsz3302);
        KILL(lbits)(&zgsz3301);
      }
    }
    {
      lbits zgsz3304;
      CREATE(lbits)(&zgsz3304);
      CONVERT_OF(lbits, fbits)(&zgsz3304, zgaz3168, UINT64_C(2) , true);
      lbits zgsz3305;
      CREATE(lbits)(&zgsz3305);
      CONVERT_OF(lbits, fbits)(&zgsz3305, zcarry_out, UINT64_C(2) , true);
      lbits zgsz3306;
      CREATE(lbits)(&zgsz3306);
      add_bits(&zgsz3306, zgsz3304, zgsz3305);
      zTtop2 = CONVERT_OF(fbits, lbits)(zgsz3306, true);
      KILL(lbits)(&zgsz3306);
      KILL(lbits)(&zgsz3305);
      KILL(lbits)(&zgsz3304);
    }
  }
  struct zCapability zgaz3164;
  {
    uint64_t zgaz3120;
    {
      bool zgaz3116;
      {
        sail_int zgsz3317;
        CREATE(sail_int)(&zgsz3317);
        CONVERT_OF(sail_int, mach_int)(&zgsz3317, zcap_uperms_width);
        sail_int zgsz3318;
        CREATE(sail_int)(&zgsz3318);
        CONVERT_OF(sail_int, mach_int)(&zgsz3318, INT64_C(0));
        zgaz3116 = gt(zgsz3317, zgsz3318);
        KILL(sail_int)(&zgsz3318);
        KILL(sail_int)(&zgsz3317);
      }
      uint64_t zgaz3118;
      zgaz3118 = zc.zperms;
      int64_t zgaz3119;
      {
        int64_t zgaz3117;
        {
          sail_int zgsz3310;
          CREATE(sail_int)(&zgsz3310);
          CONVERT_OF(sail_int, mach_int)(&zgsz3310, zcap_hperms_width);
          sail_int zgsz3311;
          CREATE(sail_int)(&zgsz3311);
          CONVERT_OF(sail_int, mach_int)(&zgsz3311, zcap_uperms_width);
          sail_int zgsz3312;
          CREATE(sail_int)(&zgsz3312);
          add_int(&zgsz3312, zgsz3310, zgsz3311);
          zgaz3117 = CONVERT_OF(mach_int, sail_int)(zgsz3312);
          KILL(sail_int)(&zgsz3312);
          KILL(sail_int)(&zgsz3311);
          KILL(sail_int)(&zgsz3310);
        }
        {
          sail_int zgsz3307;
          CREATE(sail_int)(&zgsz3307);
          CONVERT_OF(sail_int, mach_int)(&zgsz3307, zgaz3117);
          sail_int zgsz3308;
          CREATE(sail_int)(&zgsz3308);
          CONVERT_OF(sail_int, mach_int)(&zgsz3308, INT64_C(1));
          sail_int zgsz3309;
          CREATE(sail_int)(&zgsz3309);
          sub_int(&zgsz3309, zgsz3307, zgsz3308);
          zgaz3119 = CONVERT_OF(mach_int, sail_int)(zgsz3309);
          KILL(sail_int)(&zgsz3309);
          KILL(sail_int)(&zgsz3308);
          KILL(sail_int)(&zgsz3307);
        }
      }
      {
        lbits zgsz3313;
        CREATE(lbits)(&zgsz3313);
        CONVERT_OF(lbits, fbits)(&zgsz3313, zgaz3118, UINT64_C(16) , true);
        sail_int zgsz3314;
        CREATE(sail_int)(&zgsz3314);
        CONVERT_OF(sail_int, mach_int)(&zgsz3314, zgaz3119);
        sail_int zgsz3315;
        CREATE(sail_int)(&zgsz3315);
        CONVERT_OF(sail_int, mach_int)(&zgsz3315, zcap_hperms_width);
        lbits zgsz3316;
        CREATE(lbits)(&zgsz3316);
        vector_subrange_lbits(&zgsz3316, zgsz3313, zgsz3314, zgsz3315);
        zgaz3120 = CONVERT_OF(fbits, lbits)(zgsz3316, true);
        KILL(lbits)(&zgsz3316);
        KILL(sail_int)(&zgsz3315);
        KILL(sail_int)(&zgsz3314);
        KILL(lbits)(&zgsz3313);
      }
    }
    bool zgaz3123;
    {
      fbits zgaz3122;
      {
        uint64_t zgaz3121;
        zgaz3121 = zc.zperms;
        {
          lbits zgsz3319;
          CREATE(lbits)(&zgsz3319);
          CONVERT_OF(lbits, fbits)(&zgsz3319, zgaz3121, UINT64_C(16) , true);
          sail_int zgsz3320;
          CREATE(sail_int)(&zgsz3320);
          CONVERT_OF(sail_int, mach_int)(&zgsz3320, INT64_C(11));
          zgaz3122 = bitvector_access(zgsz3319, zgsz3320);
          KILL(sail_int)(&zgsz3320);
          KILL(lbits)(&zgsz3319);
        }
      }
      zgaz3123 = sailgen_bit_to_bool(zgaz3122);
    }
    bool zgaz3126;
    {
      fbits zgaz3125;
      {
        uint64_t zgaz3124;
        zgaz3124 = zc.zperms;
        {
          lbits zgsz3321;
          CREATE(lbits)(&zgsz3321);
          CONVERT_OF(lbits, fbits)(&zgsz3321, zgaz3124, UINT64_C(16) , true);
          sail_int zgsz3322;
          CREATE(sail_int)(&zgsz3322);
          CONVERT_OF(sail_int, mach_int)(&zgsz3322, INT64_C(10));
          zgaz3125 = bitvector_access(zgsz3321, zgsz3322);
          KILL(sail_int)(&zgsz3322);
          KILL(lbits)(&zgsz3321);
        }
      }
      zgaz3126 = sailgen_bit_to_bool(zgaz3125);
    }
    bool zgaz3129;
    {
      fbits zgaz3128;
      {
        uint64_t zgaz3127;
        zgaz3127 = zc.zperms;
        {
          lbits zgsz3323;
          CREATE(lbits)(&zgsz3323);
          CONVERT_OF(lbits, fbits)(&zgsz3323, zgaz3127, UINT64_C(16) , true);
          sail_int zgsz3324;
          CREATE(sail_int)(&zgsz3324);
          CONVERT_OF(sail_int, mach_int)(&zgsz3324, INT64_C(9));
          zgaz3128 = bitvector_access(zgsz3323, zgsz3324);
          KILL(sail_int)(&zgsz3324);
          KILL(lbits)(&zgsz3323);
        }
      }
      zgaz3129 = sailgen_bit_to_bool(zgaz3128);
    }
    bool zgaz3132;
    {
      fbits zgaz3131;
      {
        uint64_t zgaz3130;
        zgaz3130 = zc.zperms;
        {
          lbits zgsz3325;
          CREATE(lbits)(&zgsz3325);
          CONVERT_OF(lbits, fbits)(&zgsz3325, zgaz3130, UINT64_C(16) , true);
          sail_int zgsz3326;
          CREATE(sail_int)(&zgsz3326);
          CONVERT_OF(sail_int, mach_int)(&zgsz3326, INT64_C(8));
          zgaz3131 = bitvector_access(zgsz3325, zgsz3326);
          KILL(sail_int)(&zgsz3326);
          KILL(lbits)(&zgsz3325);
        }
      }
      zgaz3132 = sailgen_bit_to_bool(zgaz3131);
    }
    bool zgaz3135;
    {
      fbits zgaz3134;
      {
        uint64_t zgaz3133;
        zgaz3133 = zc.zperms;
        {
          lbits zgsz3327;
          CREATE(lbits)(&zgsz3327);
          CONVERT_OF(lbits, fbits)(&zgsz3327, zgaz3133, UINT64_C(16) , true);
          sail_int zgsz3328;
          CREATE(sail_int)(&zgsz3328);
          CONVERT_OF(sail_int, mach_int)(&zgsz3328, INT64_C(7));
          zgaz3134 = bitvector_access(zgsz3327, zgsz3328);
          KILL(sail_int)(&zgsz3328);
          KILL(lbits)(&zgsz3327);
        }
      }
      zgaz3135 = sailgen_bit_to_bool(zgaz3134);
    }
    bool zgaz3138;
    {
      fbits zgaz3137;
      {
        uint64_t zgaz3136;
        zgaz3136 = zc.zperms;
        {
          lbits zgsz3329;
          CREATE(lbits)(&zgsz3329);
          CONVERT_OF(lbits, fbits)(&zgsz3329, zgaz3136, UINT64_C(16) , true);
          sail_int zgsz3330;
          CREATE(sail_int)(&zgsz3330);
          CONVERT_OF(sail_int, mach_int)(&zgsz3330, INT64_C(6));
          zgaz3137 = bitvector_access(zgsz3329, zgsz3330);
          KILL(sail_int)(&zgsz3330);
          KILL(lbits)(&zgsz3329);
        }
      }
      zgaz3138 = sailgen_bit_to_bool(zgaz3137);
    }
    bool zgaz3141;
    {
      fbits zgaz3140;
      {
        uint64_t zgaz3139;
        zgaz3139 = zc.zperms;
        {
          lbits zgsz3331;
          CREATE(lbits)(&zgsz3331);
          CONVERT_OF(lbits, fbits)(&zgsz3331, zgaz3139, UINT64_C(16) , true);
          sail_int zgsz3332;
          CREATE(sail_int)(&zgsz3332);
          CONVERT_OF(sail_int, mach_int)(&zgsz3332, INT64_C(5));
          zgaz3140 = bitvector_access(zgsz3331, zgsz3332);
          KILL(sail_int)(&zgsz3332);
          KILL(lbits)(&zgsz3331);
        }
      }
      zgaz3141 = sailgen_bit_to_bool(zgaz3140);
    }
    bool zgaz3144;
    {
      fbits zgaz3143;
      {
        uint64_t zgaz3142;
        zgaz3142 = zc.zperms;
        {
          lbits zgsz3333;
          CREATE(lbits)(&zgsz3333);
          CONVERT_OF(lbits, fbits)(&zgsz3333, zgaz3142, UINT64_C(16) , true);
          sail_int zgsz3334;
          CREATE(sail_int)(&zgsz3334);
          CONVERT_OF(sail_int, mach_int)(&zgsz3334, INT64_C(4));
          zgaz3143 = bitvector_access(zgsz3333, zgsz3334);
          KILL(sail_int)(&zgsz3334);
          KILL(lbits)(&zgsz3333);
        }
      }
      zgaz3144 = sailgen_bit_to_bool(zgaz3143);
    }
    bool zgaz3147;
    {
      fbits zgaz3146;
      {
        uint64_t zgaz3145;
        zgaz3145 = zc.zperms;
        {
          lbits zgsz3335;
          CREATE(lbits)(&zgsz3335);
          CONVERT_OF(lbits, fbits)(&zgsz3335, zgaz3145, UINT64_C(16) , true);
          sail_int zgsz3336;
          CREATE(sail_int)(&zgsz3336);
          CONVERT_OF(sail_int, mach_int)(&zgsz3336, INT64_C(3));
          zgaz3146 = bitvector_access(zgsz3335, zgsz3336);
          KILL(sail_int)(&zgsz3336);
          KILL(lbits)(&zgsz3335);
        }
      }
      zgaz3147 = sailgen_bit_to_bool(zgaz3146);
    }
    bool zgaz3150;
    {
      fbits zgaz3149;
      {
        uint64_t zgaz3148;
        zgaz3148 = zc.zperms;
        {
          lbits zgsz3337;
          CREATE(lbits)(&zgsz3337);
          CONVERT_OF(lbits, fbits)(&zgsz3337, zgaz3148, UINT64_C(16) , true);
          sail_int zgsz3338;
          CREATE(sail_int)(&zgsz3338);
          CONVERT_OF(sail_int, mach_int)(&zgsz3338, INT64_C(2));
          zgaz3149 = bitvector_access(zgsz3337, zgsz3338);
          KILL(sail_int)(&zgsz3338);
          KILL(lbits)(&zgsz3337);
        }
      }
      zgaz3150 = sailgen_bit_to_bool(zgaz3149);
    }
    bool zgaz3153;
    {
      fbits zgaz3152;
      {
        uint64_t zgaz3151;
        zgaz3151 = zc.zperms;
        {
          lbits zgsz3339;
          CREATE(lbits)(&zgsz3339);
          CONVERT_OF(lbits, fbits)(&zgsz3339, zgaz3151, UINT64_C(16) , true);
          sail_int zgsz3340;
          CREATE(sail_int)(&zgsz3340);
          CONVERT_OF(sail_int, mach_int)(&zgsz3340, INT64_C(1));
          zgaz3152 = bitvector_access(zgsz3339, zgsz3340);
          KILL(sail_int)(&zgsz3340);
          KILL(lbits)(&zgsz3339);
        }
      }
      zgaz3153 = sailgen_bit_to_bool(zgaz3152);
    }
    bool zgaz3156;
    {
      fbits zgaz3155;
      {
        uint64_t zgaz3154;
        zgaz3154 = zc.zperms;
        {
          lbits zgsz3341;
          CREATE(lbits)(&zgsz3341);
          CONVERT_OF(lbits, fbits)(&zgsz3341, zgaz3154, UINT64_C(16) , true);
          sail_int zgsz3342;
          CREATE(sail_int)(&zgsz3342);
          CONVERT_OF(sail_int, mach_int)(&zgsz3342, INT64_C(0));
          zgaz3155 = bitvector_access(zgsz3341, zgsz3342);
          KILL(sail_int)(&zgsz3342);
          KILL(lbits)(&zgsz3341);
        }
      }
      zgaz3156 = sailgen_bit_to_bool(zgaz3155);
    }
    uint64_t zgaz3157;
    zgaz3157 = zc.zreserved;
    bool zgaz3160;
    {
      fbits zgaz3159;
      {
        uint64_t zgaz3158;
        zgaz3158 = zc.zflags;
        {
          lbits zgsz3343;
          CREATE(lbits)(&zgsz3343);
          CONVERT_OF(lbits, fbits)(&zgsz3343, zgaz3158, UINT64_C(1) , true);
          sail_int zgsz3344;
          CREATE(sail_int)(&zgsz3344);
          CONVERT_OF(sail_int, mach_int)(&zgsz3344, INT64_C(0));
          zgaz3159 = bitvector_access(zgsz3343, zgsz3344);
          KILL(sail_int)(&zgsz3344);
          KILL(lbits)(&zgsz3343);
        }
      }
      zgaz3160 = sailgen_bit_to_bool(zgaz3159);
    }
    uint64_t zgaz3161;
    {
      lbits zgsz3345;
      CREATE(lbits)(&zgsz3345);
      CONVERT_OF(lbits, fbits)(&zgsz3345, zTtop2, UINT64_C(2) , true);
      lbits zgsz3346;
      CREATE(lbits)(&zgsz3346);
      CONVERT_OF(lbits, fbits)(&zgsz3346, zT, UINT64_C(12) , true);
      lbits zgsz3347;
      CREATE(lbits)(&zgsz3347);
      append(&zgsz3347, zgsz3345, zgsz3346);
      zgaz3161 = CONVERT_OF(fbits, lbits)(zgsz3347, true);
      KILL(lbits)(&zgsz3347);
      KILL(lbits)(&zgsz3346);
      KILL(lbits)(&zgsz3345);
    }
    uint64_t zgaz3162;
    zgaz3162 = zc.zotype;
    uint64_t zgaz3163;
    zgaz3163 = zc.zaddress;
    struct zCapability zgsz342;
    zgsz342.zB = zBs;
    zgsz342.zE = zE;
    zgsz342.zT = zgaz3161;
    zgsz342.zaccess_system_regs = zgaz3126;
    zgsz342.zaddress = zgaz3163;
    zgsz342.zflag_cap_mode = zgaz3160;
    zgsz342.zglobal = zgaz3156;
    zgsz342.zinternal_E = zinternal_E;
    zgsz342.zotype = zgaz3162;
    zgsz342.zpermit_cinvoke = zgaz3132;
    zgsz342.zpermit_execute = zgaz3153;
    zgsz342.zpermit_load = zgaz3150;
    zgsz342.zpermit_load_cap = zgaz3144;
    zgsz342.zpermit_seal = zgaz3135;
    zgsz342.zpermit_set_CID = zgaz3123;
    zgsz342.zpermit_store = zgaz3147;
    zgsz342.zpermit_store_cap = zgaz3141;
    zgsz342.zpermit_store_local_cap = zgaz3138;
    zgsz342.zpermit_unseal = zgaz3129;
    zgsz342.zreserved = zgaz3157;
    zgsz342.ztag = zt;
    zgsz342.zuperms = zgaz3120;
    zgaz3164 = zgsz342;
  }
  zcbz318 = zgaz3164;
  goto cleanup_86;
  /* unreachable after return */








  goto end_cleanup_87;
cleanup_86: ;








  goto end_function_85;
end_cleanup_87: ;
end_function_85: ;
  return zcbz318;
end_block_exception_88: ;
  struct zCapability zcbz348 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz348;
}

static struct zCapability sailgen_capBitsToCapability(bool, lbits);

static struct zCapability sailgen_capBitsToCapability(bool zt, lbits zc)
{
  __label__ end_function_90, end_block_exception_91;

  struct zCapability zcbz319;
  struct zEncCapability zgaz3174;
  zgaz3174 = sailgen_capBitsToEncCapability(zc);
  zcbz319 = sailgen_encCapabilityToCapability(zt, zgaz3174);

end_function_90: ;
  return zcbz319;
end_block_exception_91: ;
  struct zCapability zcbz349 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz349;
}

static struct zEncCapability sailgen_capToEncCap(struct zCapability);

static struct zEncCapability sailgen_capToEncCap(struct zCapability zcap)
{
  __label__ cleanup_94, end_cleanup_95, end_function_93, end_block_exception_96;

  struct zEncCapability zcbz320;
  uint64_t zt_hi;
  {
    uint64_t zgaz3201;
    zgaz3201 = zcap.zT;
    int64_t zgaz3202;
    {
      sail_int zgsz3358;
      CREATE(sail_int)(&zgsz3358);
      CONVERT_OF(sail_int, mach_int)(&zgsz3358, zcap_mantissa_width);
      sail_int zgsz3359;
      CREATE(sail_int)(&zgsz3359);
      CONVERT_OF(sail_int, mach_int)(&zgsz3359, INT64_C(3));
      sail_int zgsz3360;
      CREATE(sail_int)(&zgsz3360);
      sub_int(&zgsz3360, zgsz3358, zgsz3359);
      zgaz3202 = CONVERT_OF(mach_int, sail_int)(zgsz3360);
      KILL(sail_int)(&zgsz3360);
      KILL(sail_int)(&zgsz3359);
      KILL(sail_int)(&zgsz3358);
    }
    {
      lbits zgsz3354;
      CREATE(lbits)(&zgsz3354);
      CONVERT_OF(lbits, fbits)(&zgsz3354, zgaz3201, UINT64_C(14) , true);
      sail_int zgsz3355;
      CREATE(sail_int)(&zgsz3355);
      CONVERT_OF(sail_int, mach_int)(&zgsz3355, zgaz3202);
      sail_int zgsz3356;
      CREATE(sail_int)(&zgsz3356);
      CONVERT_OF(sail_int, mach_int)(&zgsz3356, zinternal_E_take_bits);
      lbits zgsz3357;
      CREATE(lbits)(&zgsz3357);
      vector_subrange_lbits(&zgsz3357, zgsz3354, zgsz3355, zgsz3356);
      zt_hi = CONVERT_OF(fbits, lbits)(zgsz3357, true);
      KILL(lbits)(&zgsz3357);
      KILL(sail_int)(&zgsz3356);
      KILL(sail_int)(&zgsz3355);
      KILL(lbits)(&zgsz3354);
    }
  }
  uint64_t zt_lo;
  {
    uint64_t zgaz3199;
    zgaz3199 = zcap.zT;
    int64_t zgaz3200;
    {
      sail_int zgsz3365;
      CREATE(sail_int)(&zgsz3365);
      CONVERT_OF(sail_int, mach_int)(&zgsz3365, zinternal_E_take_bits);
      sail_int zgsz3366;
      CREATE(sail_int)(&zgsz3366);
      CONVERT_OF(sail_int, mach_int)(&zgsz3366, INT64_C(1));
      sail_int zgsz3367;
      CREATE(sail_int)(&zgsz3367);
      sub_int(&zgsz3367, zgsz3365, zgsz3366);
      zgaz3200 = CONVERT_OF(mach_int, sail_int)(zgsz3367);
      KILL(sail_int)(&zgsz3367);
      KILL(sail_int)(&zgsz3366);
      KILL(sail_int)(&zgsz3365);
    }
    {
      lbits zgsz3361;
      CREATE(lbits)(&zgsz3361);
      CONVERT_OF(lbits, fbits)(&zgsz3361, zgaz3199, UINT64_C(14) , true);
      sail_int zgsz3362;
      CREATE(sail_int)(&zgsz3362);
      CONVERT_OF(sail_int, mach_int)(&zgsz3362, zgaz3200);
      sail_int zgsz3363;
      CREATE(sail_int)(&zgsz3363);
      CONVERT_OF(sail_int, mach_int)(&zgsz3363, INT64_C(0));
      lbits zgsz3364;
      CREATE(lbits)(&zgsz3364);
      vector_subrange_lbits(&zgsz3364, zgsz3361, zgsz3362, zgsz3363);
      zt_lo = CONVERT_OF(fbits, lbits)(zgsz3364, true);
      KILL(lbits)(&zgsz3364);
      KILL(sail_int)(&zgsz3363);
      KILL(sail_int)(&zgsz3362);
      KILL(lbits)(&zgsz3361);
    }
  }
  uint64_t zb_hi;
  {
    uint64_t zgaz3197;
    zgaz3197 = zcap.zB;
    int64_t zgaz3198;
    {
      sail_int zgsz3372;
      CREATE(sail_int)(&zgsz3372);
      CONVERT_OF(sail_int, mach_int)(&zgsz3372, zcap_mantissa_width);
      sail_int zgsz3373;
      CREATE(sail_int)(&zgsz3373);
      CONVERT_OF(sail_int, mach_int)(&zgsz3373, INT64_C(1));
      sail_int zgsz3374;
      CREATE(sail_int)(&zgsz3374);
      sub_int(&zgsz3374, zgsz3372, zgsz3373);
      zgaz3198 = CONVERT_OF(mach_int, sail_int)(zgsz3374);
      KILL(sail_int)(&zgsz3374);
      KILL(sail_int)(&zgsz3373);
      KILL(sail_int)(&zgsz3372);
    }
    {
      lbits zgsz3368;
      CREATE(lbits)(&zgsz3368);
      CONVERT_OF(lbits, fbits)(&zgsz3368, zgaz3197, UINT64_C(14) , true);
      sail_int zgsz3369;
      CREATE(sail_int)(&zgsz3369);
      CONVERT_OF(sail_int, mach_int)(&zgsz3369, zgaz3198);
      sail_int zgsz3370;
      CREATE(sail_int)(&zgsz3370);
      CONVERT_OF(sail_int, mach_int)(&zgsz3370, zinternal_E_take_bits);
      lbits zgsz3371;
      CREATE(lbits)(&zgsz3371);
      vector_subrange_lbits(&zgsz3371, zgsz3368, zgsz3369, zgsz3370);
      zb_hi = CONVERT_OF(fbits, lbits)(zgsz3371, true);
      KILL(lbits)(&zgsz3371);
      KILL(sail_int)(&zgsz3370);
      KILL(sail_int)(&zgsz3369);
      KILL(lbits)(&zgsz3368);
    }
  }
  uint64_t zb_lo;
  {
    uint64_t zgaz3195;
    zgaz3195 = zcap.zB;
    int64_t zgaz3196;
    {
      sail_int zgsz3379;
      CREATE(sail_int)(&zgsz3379);
      CONVERT_OF(sail_int, mach_int)(&zgsz3379, zinternal_E_take_bits);
      sail_int zgsz3380;
      CREATE(sail_int)(&zgsz3380);
      CONVERT_OF(sail_int, mach_int)(&zgsz3380, INT64_C(1));
      sail_int zgsz3381;
      CREATE(sail_int)(&zgsz3381);
      sub_int(&zgsz3381, zgsz3379, zgsz3380);
      zgaz3196 = CONVERT_OF(mach_int, sail_int)(zgsz3381);
      KILL(sail_int)(&zgsz3381);
      KILL(sail_int)(&zgsz3380);
      KILL(sail_int)(&zgsz3379);
    }
    {
      lbits zgsz3375;
      CREATE(lbits)(&zgsz3375);
      CONVERT_OF(lbits, fbits)(&zgsz3375, zgaz3195, UINT64_C(14) , true);
      sail_int zgsz3376;
      CREATE(sail_int)(&zgsz3376);
      CONVERT_OF(sail_int, mach_int)(&zgsz3376, zgaz3196);
      sail_int zgsz3377;
      CREATE(sail_int)(&zgsz3377);
      CONVERT_OF(sail_int, mach_int)(&zgsz3377, INT64_C(0));
      lbits zgsz3378;
      CREATE(lbits)(&zgsz3378);
      vector_subrange_lbits(&zgsz3378, zgsz3375, zgsz3376, zgsz3377);
      zb_lo = CONVERT_OF(fbits, lbits)(zgsz3378, true);
      KILL(lbits)(&zgsz3378);
      KILL(sail_int)(&zgsz3377);
      KILL(sail_int)(&zgsz3376);
      KILL(lbits)(&zgsz3375);
    }
  }
  {
    bool zgaz3175;
    zgaz3175 = zcap.zinternal_E;
    unit zgsz343;
    if (zgaz3175) {
      {
        uint64_t zgaz3177;
        zgaz3177 = zcap.zE;
        int64_t zgaz3178;
        {
          int64_t zgaz3176;
          {
            sail_int zgsz3385;
            CREATE(sail_int)(&zgsz3385);
            CONVERT_OF(sail_int, mach_int)(&zgsz3385, INT64_C(2));
            sail_int zgsz3386;
            CREATE(sail_int)(&zgsz3386);
            CONVERT_OF(sail_int, mach_int)(&zgsz3386, zinternal_E_take_bits);
            sail_int zgsz3387;
            CREATE(sail_int)(&zgsz3387);
            mult_int(&zgsz3387, zgsz3385, zgsz3386);
            zgaz3176 = CONVERT_OF(mach_int, sail_int)(zgsz3387);
            KILL(sail_int)(&zgsz3387);
            KILL(sail_int)(&zgsz3386);
            KILL(sail_int)(&zgsz3385);
          }
          {
            sail_int zgsz3382;
            CREATE(sail_int)(&zgsz3382);
            CONVERT_OF(sail_int, mach_int)(&zgsz3382, zgaz3176);
            sail_int zgsz3383;
            CREATE(sail_int)(&zgsz3383);
            CONVERT_OF(sail_int, mach_int)(&zgsz3383, INT64_C(1));
            sail_int zgsz3384;
            CREATE(sail_int)(&zgsz3384);
            sub_int(&zgsz3384, zgsz3382, zgsz3383);
            zgaz3178 = CONVERT_OF(mach_int, sail_int)(zgsz3384);
            KILL(sail_int)(&zgsz3384);
            KILL(sail_int)(&zgsz3383);
            KILL(sail_int)(&zgsz3382);
          }
        }
        {
          lbits zgsz3388;
          CREATE(lbits)(&zgsz3388);
          CONVERT_OF(lbits, fbits)(&zgsz3388, zgaz3177, UINT64_C(6) , true);
          sail_int zgsz3389;
          CREATE(sail_int)(&zgsz3389);
          CONVERT_OF(sail_int, mach_int)(&zgsz3389, zgaz3178);
          sail_int zgsz3390;
          CREATE(sail_int)(&zgsz3390);
          CONVERT_OF(sail_int, mach_int)(&zgsz3390, zinternal_E_take_bits);
          lbits zgsz3391;
          CREATE(lbits)(&zgsz3391);
          vector_subrange_lbits(&zgsz3391, zgsz3388, zgsz3389, zgsz3390);
          zt_lo = CONVERT_OF(fbits, lbits)(zgsz3391, true);
          KILL(lbits)(&zgsz3391);
          KILL(sail_int)(&zgsz3390);
          KILL(sail_int)(&zgsz3389);
          KILL(lbits)(&zgsz3388);
        }
        unit zgsz344;
        zgsz344 = UNIT;
      }
      uint64_t zgaz3179;
      zgaz3179 = zcap.zE;
      int64_t zgaz3180;
      {
        sail_int zgsz3396;
        CREATE(sail_int)(&zgsz3396);
        CONVERT_OF(sail_int, mach_int)(&zgsz3396, zinternal_E_take_bits);
        sail_int zgsz3397;
        CREATE(sail_int)(&zgsz3397);
        CONVERT_OF(sail_int, mach_int)(&zgsz3397, INT64_C(1));
        sail_int zgsz3398;
        CREATE(sail_int)(&zgsz3398);
        sub_int(&zgsz3398, zgsz3396, zgsz3397);
        zgaz3180 = CONVERT_OF(mach_int, sail_int)(zgsz3398);
        KILL(sail_int)(&zgsz3398);
        KILL(sail_int)(&zgsz3397);
        KILL(sail_int)(&zgsz3396);
      }
      {
        lbits zgsz3392;
        CREATE(lbits)(&zgsz3392);
        CONVERT_OF(lbits, fbits)(&zgsz3392, zgaz3179, UINT64_C(6) , true);
        sail_int zgsz3393;
        CREATE(sail_int)(&zgsz3393);
        CONVERT_OF(sail_int, mach_int)(&zgsz3393, zgaz3180);
        sail_int zgsz3394;
        CREATE(sail_int)(&zgsz3394);
        CONVERT_OF(sail_int, mach_int)(&zgsz3394, INT64_C(0));
        lbits zgsz3395;
        CREATE(lbits)(&zgsz3395);
        vector_subrange_lbits(&zgsz3395, zgsz3392, zgsz3393, zgsz3394);
        zb_lo = CONVERT_OF(fbits, lbits)(zgsz3395, true);
        KILL(lbits)(&zgsz3395);
        KILL(sail_int)(&zgsz3394);
        KILL(sail_int)(&zgsz3393);
        KILL(lbits)(&zgsz3392);
      }
      zgsz343 = UNIT;
    } else {  zgsz343 = UNIT;  }
  }
  struct zEncCapability zgaz3194;
  {
    uint64_t zgaz3183;
    {
      uint64_t zgaz3181;
      zgaz3181 = zcap.zuperms;
      uint64_t zgaz3182;
      zgaz3182 = sailgen_getCapHardPerms(zcap);
      {
        lbits zgsz3399;
        CREATE(lbits)(&zgsz3399);
        CONVERT_OF(lbits, fbits)(&zgsz3399, zgaz3181, UINT64_C(4) , true);
        lbits zgsz3400;
        CREATE(lbits)(&zgsz3400);
        CONVERT_OF(lbits, fbits)(&zgsz3400, zgaz3182, UINT64_C(12) , true);
        lbits zgsz3401;
        CREATE(lbits)(&zgsz3401);
        append(&zgsz3401, zgsz3399, zgsz3400);
        zgaz3183 = CONVERT_OF(fbits, lbits)(zgsz3401, true);
        KILL(lbits)(&zgsz3401);
        KILL(lbits)(&zgsz3400);
        KILL(lbits)(&zgsz3399);
      }
    }
    uint64_t zgaz3184;
    zgaz3184 = zcap.zreserved;
    uint64_t zgaz3186;
    {
      bool zgaz3185;
      zgaz3185 = zcap.zflag_cap_mode;
      zgaz3186 = sailgen_bool_to_bits(zgaz3185);
    }
    uint64_t zgaz3187;
    zgaz3187 = zcap.zotype;
    fbits zgaz3190;
    {
      uint64_t zgaz3189;
      {
        bool zgaz3188;
        zgaz3188 = zcap.zinternal_E;
        zgaz3189 = sailgen_bool_to_bits(zgaz3188);
      }
      {
        lbits zgsz3402;
        CREATE(lbits)(&zgsz3402);
        CONVERT_OF(lbits, fbits)(&zgsz3402, zgaz3189, UINT64_C(1) , true);
        sail_int zgsz3403;
        CREATE(sail_int)(&zgsz3403);
        CONVERT_OF(sail_int, mach_int)(&zgsz3403, INT64_C(0));
        zgaz3190 = bitvector_access(zgsz3402, zgsz3403);
        KILL(sail_int)(&zgsz3403);
        KILL(lbits)(&zgsz3402);
      }
    }
    uint64_t zgaz3191;
    {
      lbits zgsz3407;
      CREATE(lbits)(&zgsz3407);
      CONVERT_OF(lbits, fbits)(&zgsz3407, zt_hi, UINT64_C(9) , true);
      lbits zgsz3408;
      CREATE(lbits)(&zgsz3408);
      CONVERT_OF(lbits, fbits)(&zgsz3408, zt_lo, UINT64_C(3) , true);
      lbits zgsz3409;
      CREATE(lbits)(&zgsz3409);
      append(&zgsz3409, zgsz3407, zgsz3408);
      zgaz3191 = CONVERT_OF(fbits, lbits)(zgsz3409, true);
      KILL(lbits)(&zgsz3409);
      KILL(lbits)(&zgsz3408);
      KILL(lbits)(&zgsz3407);
    }
    uint64_t zgaz3192;
    {
      lbits zgsz3404;
      CREATE(lbits)(&zgsz3404);
      CONVERT_OF(lbits, fbits)(&zgsz3404, zb_hi, UINT64_C(11) , true);
      lbits zgsz3405;
      CREATE(lbits)(&zgsz3405);
      CONVERT_OF(lbits, fbits)(&zgsz3405, zb_lo, UINT64_C(3) , true);
      lbits zgsz3406;
      CREATE(lbits)(&zgsz3406);
      append(&zgsz3406, zgsz3404, zgsz3405);
      zgaz3192 = CONVERT_OF(fbits, lbits)(zgsz3406, true);
      KILL(lbits)(&zgsz3406);
      KILL(lbits)(&zgsz3405);
      KILL(lbits)(&zgsz3404);
    }
    uint64_t zgaz3193;
    zgaz3193 = zcap.zaddress;
    struct zEncCapability zgsz345;
    zgsz345.zB = zgaz3192;
    zgsz345.zT = zgaz3191;
    zgsz345.zaddress = zgaz3193;
    zgsz345.zflags = zgaz3186;
    zgsz345.zinternal_E = zgaz3190;
    zgsz345.zotype = zgaz3187;
    zgsz345.zperms = zgaz3183;
    zgsz345.zreserved = zgaz3184;
    zgaz3194 = zgsz345;
  }
  zcbz320 = zgaz3194;
  goto cleanup_94;
  /* unreachable after return */





  goto end_cleanup_95;
cleanup_94: ;





  goto end_function_93;
end_cleanup_95: ;
end_function_93: ;
  return zcbz320;
end_block_exception_96: ;
  struct zEncCapability zcbz350 = { .zreserved = UINT64_C(0xdeadc0de), .zperms = UINT64_C(0xdeadc0de), .zotype = UINT64_C(0xdeadc0de), .zinternal_E = UINT64_C(0), .zflags = UINT64_C(0xdeadc0de), .zaddress = UINT64_C(0xdeadc0de), .zT = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz350;
}

static void sailgen_capToBits(lbits *rop, struct zCapability);

static void sailgen_capToBits(lbits *zcbz321, struct zCapability zcap)
{
  __label__ end_function_98, end_block_exception_99, end_function_200;

  struct zEncCapability zgaz3203;
  zgaz3203 = sailgen_capToEncCap(zcap);
  sailgen_encCapToBits((*(&zcbz321)), zgaz3203);

end_function_98: ;
  goto end_function_200;
end_block_exception_99: ;
  goto end_function_200;
end_function_200: ;
}

static lbits znull_cap_bits;
static void create_letbind_27(void) {    CREATE(lbits)(&znull_cap_bits);


  lbits zgsz346;
  CREATE(lbits)(&zgsz346);
  sailgen_capToBits(&zgsz346, znull_cap);
  COPY(lbits)(&znull_cap_bits, zgsz346);
  KILL(lbits)(&zgsz346);
let_end_100: ;
}
static void kill_letbind_27(void) {    KILL(lbits)(&znull_cap_bits);
}

static void sailgen_capToMemBits(lbits *rop, struct zCapability);

static void sailgen_capToMemBits(lbits *zcbz322, struct zCapability zcap)
{
  __label__ end_function_102, end_block_exception_103, end_function_199;

  lbits zgaz3204;
  CREATE(lbits)(&zgaz3204);
  sailgen_capToBits(&zgaz3204, zcap);
  xor_bits((*(&zcbz322)), zgaz3204, znull_cap_bits);
  KILL(lbits)(&zgaz3204);
end_function_102: ;
  goto end_function_199;
end_block_exception_103: ;
  goto end_function_199;
end_function_199: ;
}

static struct zCapability sailgen_memBitsToCapability(bool, lbits);

static struct zCapability sailgen_memBitsToCapability(bool ztag, lbits zb)
{
  __label__ end_function_105, end_block_exception_106;

  struct zCapability zcbz323;
  lbits zgaz3205;
  CREATE(lbits)(&zgaz3205);
  xor_bits(&zgaz3205, zb, znull_cap_bits);
  zcbz323 = sailgen_capBitsToCapability(ztag, zgaz3205);
  KILL(lbits)(&zgaz3205);
end_function_105: ;
  return zcbz323;
end_block_exception_106: ;
  struct zCapability zcbz351 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz351;
}

// struct tuple_(%bv64, %bv)
struct ztuple_z8z5bv64zCz0z5bvz9 {
  uint64_t ztup0;
  lbits ztup1;
};

static void COPY(ztuple_z8z5bv64zCz0z5bvz9)(struct ztuple_z8z5bv64zCz0z5bvz9 *rop, const struct ztuple_z8z5bv64zCz0z5bvz9 op) {
  rop->ztup0 = op.ztup0;
  COPY(lbits)(&rop->ztup1, op.ztup1);
}

static void CREATE(ztuple_z8z5bv64zCz0z5bvz9)(struct ztuple_z8z5bv64zCz0z5bvz9 *op) {CREATE(lbits)(&op->ztup1);}

static void RECREATE(ztuple_z8z5bv64zCz0z5bvz9)(struct ztuple_z8z5bv64zCz0z5bvz9 *op) {RECREATE(lbits)(&op->ztup1);}

static void KILL(ztuple_z8z5bv64zCz0z5bvz9)(struct ztuple_z8z5bv64zCz0z5bvz9 *op) {KILL(lbits)(&op->ztup1);}

static bool EQUAL(ztuple_z8z5bv64zCz0z5bvz9)(struct ztuple_z8z5bv64zCz0z5bvz9 op1, struct ztuple_z8z5bv64zCz0z5bvz9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(lbits)(op1.ztup1, op2.ztup1);
}

static void sailgen_getCapBoundsBits(struct ztuple_z8z5bv64zCz0z5bvz9 *rop, struct zCapability);

static void sailgen_getCapBoundsBits(struct ztuple_z8z5bv64zCz0z5bvz9 *zcbz324, struct zCapability zc)
{
  __label__ end_function_108, end_block_exception_109, end_function_198;

  int64_t zE;
  {
    int64_t zgaz3239;
    {
      uint64_t zgaz3238;
      zgaz3238 = zc.zE;
      {
        lbits zgsz3410;
        CREATE(lbits)(&zgsz3410);
        CONVERT_OF(lbits, fbits)(&zgsz3410, zgaz3238, UINT64_C(6) , true);
        sail_int zgsz3411;
        CREATE(sail_int)(&zgsz3411);
        sail_unsigned(&zgsz3411, zgsz3410);
        zgaz3239 = CONVERT_OF(mach_int, sail_int)(zgsz3411);
        KILL(sail_int)(&zgsz3411);
        KILL(lbits)(&zgsz3410);
      }
    }
    {
      sail_int zgsz3412;
      CREATE(sail_int)(&zgsz3412);
      CONVERT_OF(sail_int, mach_int)(&zgsz3412, zcap_max_E);
      sail_int zgsz3413;
      CREATE(sail_int)(&zgsz3413);
      CONVERT_OF(sail_int, mach_int)(&zgsz3413, zgaz3239);
      sail_int zgsz3414;
      CREATE(sail_int)(&zgsz3414);
      min_int(&zgsz3414, zgsz3412, zgsz3413);
      zE = CONVERT_OF(mach_int, sail_int)(zgsz3414);
      KILL(sail_int)(&zgsz3414);
      KILL(sail_int)(&zgsz3413);
      KILL(sail_int)(&zgsz3412);
    }
  }
  uint64_t za;
  za = zc.zaddress;
  uint64_t za3;
  {
    uint64_t zgaz3237;
    {
      int64_t zgaz3236;
      {
        int64_t zgaz3235;
        {
          sail_int zgsz3418;
          CREATE(sail_int)(&zgsz3418);
          CONVERT_OF(sail_int, mach_int)(&zgsz3418, zE);
          sail_int zgsz3419;
          CREATE(sail_int)(&zgsz3419);
          CONVERT_OF(sail_int, mach_int)(&zgsz3419, zcap_mantissa_width);
          sail_int zgsz3420;
          CREATE(sail_int)(&zgsz3420);
          add_int(&zgsz3420, zgsz3418, zgsz3419);
          zgaz3235 = CONVERT_OF(mach_int, sail_int)(zgsz3420);
          KILL(sail_int)(&zgsz3420);
          KILL(sail_int)(&zgsz3419);
          KILL(sail_int)(&zgsz3418);
        }
        {
          sail_int zgsz3415;
          CREATE(sail_int)(&zgsz3415);
          CONVERT_OF(sail_int, mach_int)(&zgsz3415, zgaz3235);
          sail_int zgsz3416;
          CREATE(sail_int)(&zgsz3416);
          CONVERT_OF(sail_int, mach_int)(&zgsz3416, INT64_C(3));
          sail_int zgsz3417;
          CREATE(sail_int)(&zgsz3417);
          sub_int(&zgsz3417, zgsz3415, zgsz3416);
          zgaz3236 = CONVERT_OF(mach_int, sail_int)(zgsz3417);
          KILL(sail_int)(&zgsz3417);
          KILL(sail_int)(&zgsz3416);
          KILL(sail_int)(&zgsz3415);
        }
      }
      {
        lbits zgsz3421;
        CREATE(lbits)(&zgsz3421);
        CONVERT_OF(lbits, fbits)(&zgsz3421, za, UINT64_C(64) , true);
        sail_int zgsz3422;
        CREATE(sail_int)(&zgsz3422);
        CONVERT_OF(sail_int, mach_int)(&zgsz3422, zgaz3236);
        lbits zgsz3423;
        CREATE(lbits)(&zgsz3423);
        shiftr(&zgsz3423, zgsz3421, zgsz3422);
        zgaz3237 = CONVERT_OF(fbits, lbits)(zgsz3423, true);
        KILL(lbits)(&zgsz3423);
        KILL(sail_int)(&zgsz3422);
        KILL(lbits)(&zgsz3421);
      }
    }
    {
      lbits zgsz3424;
      CREATE(lbits)(&zgsz3424);
      CONVERT_OF(lbits, fbits)(&zgsz3424, zgaz3237, UINT64_C(64) , true);
      sail_int zgsz3425;
      CREATE(sail_int)(&zgsz3425);
      CONVERT_OF(sail_int, mach_int)(&zgsz3425, INT64_C(3));
      lbits zgsz3426;
      CREATE(lbits)(&zgsz3426);
      sail_truncate(&zgsz3426, zgsz3424, zgsz3425);
      za3 = CONVERT_OF(fbits, lbits)(zgsz3426, true);
      KILL(lbits)(&zgsz3426);
      KILL(sail_int)(&zgsz3425);
      KILL(lbits)(&zgsz3424);
    }
  }
  uint64_t zB3;
  {
    uint64_t zgaz3234;
    zgaz3234 = zc.zB;
    {
      lbits zgsz3427;
      CREATE(lbits)(&zgsz3427);
      CONVERT_OF(lbits, fbits)(&zgsz3427, zgaz3234, UINT64_C(14) , true);
      sail_int zgsz3428;
      CREATE(sail_int)(&zgsz3428);
      CONVERT_OF(sail_int, mach_int)(&zgsz3428, INT64_C(3));
      lbits zgsz3429;
      CREATE(lbits)(&zgsz3429);
      sail_truncateLSB(&zgsz3429, zgsz3427, zgsz3428);
      zB3 = CONVERT_OF(fbits, lbits)(zgsz3429, true);
      KILL(lbits)(&zgsz3429);
      KILL(sail_int)(&zgsz3428);
      KILL(lbits)(&zgsz3427);
    }
  }
  uint64_t zT3;
  {
    uint64_t zgaz3233;
    zgaz3233 = zc.zT;
    {
      lbits zgsz3430;
      CREATE(lbits)(&zgsz3430);
      CONVERT_OF(lbits, fbits)(&zgsz3430, zgaz3233, UINT64_C(14) , true);
      sail_int zgsz3431;
      CREATE(sail_int)(&zgsz3431);
      CONVERT_OF(sail_int, mach_int)(&zgsz3431, INT64_C(3));
      lbits zgsz3432;
      CREATE(lbits)(&zgsz3432);
      sail_truncateLSB(&zgsz3432, zgsz3430, zgsz3431);
      zT3 = CONVERT_OF(fbits, lbits)(zgsz3432, true);
      KILL(lbits)(&zgsz3432);
      KILL(sail_int)(&zgsz3431);
      KILL(lbits)(&zgsz3430);
    }
  }
  uint64_t zR3;
  {
    lbits zgsz3502;
    CREATE(lbits)(&zgsz3502);
    CONVERT_OF(lbits, fbits)(&zgsz3502, zB3, UINT64_C(3) , true);
    lbits zgsz3503;
    CREATE(lbits)(&zgsz3503);
    CONVERT_OF(lbits, fbits)(&zgsz3503, UINT64_C(0b001), UINT64_C(3) , true);
    lbits zgsz3504;
    CREATE(lbits)(&zgsz3504);
    sub_bits(&zgsz3504, zgsz3502, zgsz3503);
    zR3 = CONVERT_OF(fbits, lbits)(zgsz3504, true);
    KILL(lbits)(&zgsz3504);
    KILL(lbits)(&zgsz3503);
    KILL(lbits)(&zgsz3502);
  }
  int64_t zaHi;
  {
    bool zgaz3232;
    {
      lbits zgsz3433;
      CREATE(lbits)(&zgsz3433);
      CONVERT_OF(lbits, fbits)(&zgsz3433, za3, UINT64_C(3) , true);
      lbits zgsz3434;
      CREATE(lbits)(&zgsz3434);
      CONVERT_OF(lbits, fbits)(&zgsz3434, zR3, UINT64_C(3) , true);
      zgaz3232 = sailgen_z8operatorz0zI_uz9(zgsz3433, zgsz3434);
      KILL(lbits)(&zgsz3434);
      KILL(lbits)(&zgsz3433);
    }
    if (zgaz3232) {  zaHi = INT64_C(1);  } else {  zaHi = INT64_C(0);  }
  }
  int64_t zbHi;
  {
    bool zgaz3231;
    {
      lbits zgsz3435;
      CREATE(lbits)(&zgsz3435);
      CONVERT_OF(lbits, fbits)(&zgsz3435, zB3, UINT64_C(3) , true);
      lbits zgsz3436;
      CREATE(lbits)(&zgsz3436);
      CONVERT_OF(lbits, fbits)(&zgsz3436, zR3, UINT64_C(3) , true);
      zgaz3231 = sailgen_z8operatorz0zI_uz9(zgsz3435, zgsz3436);
      KILL(lbits)(&zgsz3436);
      KILL(lbits)(&zgsz3435);
    }
    if (zgaz3231) {  zbHi = INT64_C(1);  } else {  zbHi = INT64_C(0);  }
  }
  int64_t ztHi;
  {
    bool zgaz3230;
    {
      lbits zgsz3437;
      CREATE(lbits)(&zgsz3437);
      CONVERT_OF(lbits, fbits)(&zgsz3437, zT3, UINT64_C(3) , true);
      lbits zgsz3438;
      CREATE(lbits)(&zgsz3438);
      CONVERT_OF(lbits, fbits)(&zgsz3438, zR3, UINT64_C(3) , true);
      zgaz3230 = sailgen_z8operatorz0zI_uz9(zgsz3437, zgsz3438);
      KILL(lbits)(&zgsz3438);
      KILL(lbits)(&zgsz3437);
    }
    if (zgaz3230) {  ztHi = INT64_C(1);  } else {  ztHi = INT64_C(0);  }
  }
  int64_t zcorrection_base;
  {
    sail_int zgsz3499;
    CREATE(sail_int)(&zgsz3499);
    CONVERT_OF(sail_int, mach_int)(&zgsz3499, zbHi);
    sail_int zgsz3500;
    CREATE(sail_int)(&zgsz3500);
    CONVERT_OF(sail_int, mach_int)(&zgsz3500, zaHi);
    sail_int zgsz3501;
    CREATE(sail_int)(&zgsz3501);
    sub_int(&zgsz3501, zgsz3499, zgsz3500);
    zcorrection_base = CONVERT_OF(mach_int, sail_int)(zgsz3501);
    KILL(sail_int)(&zgsz3501);
    KILL(sail_int)(&zgsz3500);
    KILL(sail_int)(&zgsz3499);
  }
  int64_t zcorrection_top;
  {
    sail_int zgsz3496;
    CREATE(sail_int)(&zgsz3496);
    CONVERT_OF(sail_int, mach_int)(&zgsz3496, ztHi);
    sail_int zgsz3497;
    CREATE(sail_int)(&zgsz3497);
    CONVERT_OF(sail_int, mach_int)(&zgsz3497, zaHi);
    sail_int zgsz3498;
    CREATE(sail_int)(&zgsz3498);
    sub_int(&zgsz3498, zgsz3496, zgsz3497);
    zcorrection_top = CONVERT_OF(mach_int, sail_int)(zgsz3498);
    KILL(sail_int)(&zgsz3498);
    KILL(sail_int)(&zgsz3497);
    KILL(sail_int)(&zgsz3496);
  }
  uint64_t za_top;
  {
    int64_t zgaz3229;
    {
      sail_int zgsz3442;
      CREATE(sail_int)(&zgsz3442);
      CONVERT_OF(sail_int, mach_int)(&zgsz3442, zE);
      sail_int zgsz3443;
      CREATE(sail_int)(&zgsz3443);
      CONVERT_OF(sail_int, mach_int)(&zgsz3443, zcap_mantissa_width);
      sail_int zgsz3444;
      CREATE(sail_int)(&zgsz3444);
      add_int(&zgsz3444, zgsz3442, zgsz3443);
      zgaz3229 = CONVERT_OF(mach_int, sail_int)(zgsz3444);
      KILL(sail_int)(&zgsz3444);
      KILL(sail_int)(&zgsz3443);
      KILL(sail_int)(&zgsz3442);
    }
    {
      lbits zgsz3439;
      CREATE(lbits)(&zgsz3439);
      CONVERT_OF(lbits, fbits)(&zgsz3439, za, UINT64_C(64) , true);
      sail_int zgsz3440;
      CREATE(sail_int)(&zgsz3440);
      CONVERT_OF(sail_int, mach_int)(&zgsz3440, zgaz3229);
      lbits zgsz3441;
      CREATE(lbits)(&zgsz3441);
      shiftr(&zgsz3441, zgsz3439, zgsz3440);
      za_top = CONVERT_OF(fbits, lbits)(zgsz3441, true);
      KILL(lbits)(&zgsz3441);
      KILL(sail_int)(&zgsz3440);
      KILL(lbits)(&zgsz3439);
    }
  }
  lbits zbase;
  CREATE(lbits)(&zbase);
  {
    lbits zgaz3228;
    CREATE(lbits)(&zgaz3228);
    {
      uint64_t zgaz3226;
      {
        lbits zgsz3450;
        CREATE(lbits)(&zgsz3450);
        CONVERT_OF(lbits, fbits)(&zgsz3450, za_top, UINT64_C(64) , true);
        sail_int zgsz3451;
        CREATE(sail_int)(&zgsz3451);
        CONVERT_OF(sail_int, mach_int)(&zgsz3451, zcorrection_base);
        lbits zgsz3452;
        CREATE(lbits)(&zgsz3452);
        add_bits_int(&zgsz3452, zgsz3450, zgsz3451);
        zgaz3226 = CONVERT_OF(fbits, lbits)(zgsz3452, true);
        KILL(lbits)(&zgsz3452);
        KILL(sail_int)(&zgsz3451);
        KILL(lbits)(&zgsz3450);
      }
      lbits zgaz3227;
      CREATE(lbits)(&zgaz3227);
      {
        uint64_t zgaz3224;
        zgaz3224 = zc.zB;
        sbits zgaz3225;
        {
          sail_int zgsz3447;
          CREATE(sail_int)(&zgsz3447);
          CONVERT_OF(sail_int, mach_int)(&zgsz3447, zE);
          lbits zgsz3448;
          CREATE(lbits)(&zgsz3448);
          sailgen_zzeros_implicit(&zgsz3448, zgsz3447);
          zgaz3225 = CONVERT_OF(sbits, lbits)(zgsz3448, true);
          KILL(lbits)(&zgsz3448);
          KILL(sail_int)(&zgsz3447);
        }
        {
          lbits zgsz3445;
          CREATE(lbits)(&zgsz3445);
          CONVERT_OF(lbits, fbits)(&zgsz3445, zgaz3224, UINT64_C(14) , true);
          lbits zgsz3446;
          CREATE(lbits)(&zgsz3446);
          CONVERT_OF(lbits, sbits)(&zgsz3446, zgaz3225, true);
          append(&zgaz3227, zgsz3445, zgsz3446);
          KILL(lbits)(&zgsz3446);
          KILL(lbits)(&zgsz3445);
        }
      }
      {
        lbits zgsz3449;
        CREATE(lbits)(&zgsz3449);
        CONVERT_OF(lbits, fbits)(&zgsz3449, zgaz3226, UINT64_C(64) , true);
        append(&zgaz3228, zgsz3449, zgaz3227);
        KILL(lbits)(&zgsz3449);
      }
      KILL(lbits)(&zgaz3227);
    }
    {
      sail_int zgsz3453;
      CREATE(sail_int)(&zgsz3453);
      CONVERT_OF(sail_int, mach_int)(&zgsz3453, zcap_len_width);
      sail_truncate(&zbase, zgaz3228, zgsz3453);
      KILL(sail_int)(&zgsz3453);
    }
    KILL(lbits)(&zgaz3228);
  }
  lbits ztop;
  CREATE(lbits)(&ztop);
  {
    lbits zgaz3223;
    CREATE(lbits)(&zgaz3223);
    {
      uint64_t zgaz3221;
      {
        lbits zgsz3459;
        CREATE(lbits)(&zgsz3459);
        CONVERT_OF(lbits, fbits)(&zgsz3459, za_top, UINT64_C(64) , true);
        sail_int zgsz3460;
        CREATE(sail_int)(&zgsz3460);
        CONVERT_OF(sail_int, mach_int)(&zgsz3460, zcorrection_top);
        lbits zgsz3461;
        CREATE(lbits)(&zgsz3461);
        add_bits_int(&zgsz3461, zgsz3459, zgsz3460);
        zgaz3221 = CONVERT_OF(fbits, lbits)(zgsz3461, true);
        KILL(lbits)(&zgsz3461);
        KILL(sail_int)(&zgsz3460);
        KILL(lbits)(&zgsz3459);
      }
      lbits zgaz3222;
      CREATE(lbits)(&zgaz3222);
      {
        uint64_t zgaz3219;
        zgaz3219 = zc.zT;
        sbits zgaz3220;
        {
          sail_int zgsz3456;
          CREATE(sail_int)(&zgsz3456);
          CONVERT_OF(sail_int, mach_int)(&zgsz3456, zE);
          lbits zgsz3457;
          CREATE(lbits)(&zgsz3457);
          sailgen_zzeros_implicit(&zgsz3457, zgsz3456);
          zgaz3220 = CONVERT_OF(sbits, lbits)(zgsz3457, true);
          KILL(lbits)(&zgsz3457);
          KILL(sail_int)(&zgsz3456);
        }
        {
          lbits zgsz3454;
          CREATE(lbits)(&zgsz3454);
          CONVERT_OF(lbits, fbits)(&zgsz3454, zgaz3219, UINT64_C(14) , true);
          lbits zgsz3455;
          CREATE(lbits)(&zgsz3455);
          CONVERT_OF(lbits, sbits)(&zgsz3455, zgaz3220, true);
          append(&zgaz3222, zgsz3454, zgsz3455);
          KILL(lbits)(&zgsz3455);
          KILL(lbits)(&zgsz3454);
        }
      }
      {
        lbits zgsz3458;
        CREATE(lbits)(&zgsz3458);
        CONVERT_OF(lbits, fbits)(&zgsz3458, zgaz3221, UINT64_C(64) , true);
        append(&zgaz3223, zgsz3458, zgaz3222);
        KILL(lbits)(&zgsz3458);
      }
      KILL(lbits)(&zgaz3222);
    }
    {
      sail_int zgsz3462;
      CREATE(sail_int)(&zgsz3462);
      CONVERT_OF(sail_int, mach_int)(&zgsz3462, zcap_len_width);
      sail_truncate(&ztop, zgaz3223, zgsz3462);
      KILL(sail_int)(&zgsz3462);
    }
    KILL(lbits)(&zgaz3223);
  }
  uint64_t zbase2;
  {
    uint64_t zgaz3218;
    {
      fbits zgaz3217;
      {
        int64_t zgaz3216;
        {
          sail_int zgsz3464;
          CREATE(sail_int)(&zgsz3464);
          CONVERT_OF(sail_int, mach_int)(&zgsz3464, zcap_addr_width);
          sail_int zgsz3465;
          CREATE(sail_int)(&zgsz3465);
          CONVERT_OF(sail_int, mach_int)(&zgsz3465, INT64_C(1));
          sail_int zgsz3466;
          CREATE(sail_int)(&zgsz3466);
          sub_int(&zgsz3466, zgsz3464, zgsz3465);
          zgaz3216 = CONVERT_OF(mach_int, sail_int)(zgsz3466);
          KILL(sail_int)(&zgsz3466);
          KILL(sail_int)(&zgsz3465);
          KILL(sail_int)(&zgsz3464);
        }
        {
          sail_int zgsz3463;
          CREATE(sail_int)(&zgsz3463);
          CONVERT_OF(sail_int, mach_int)(&zgsz3463, zgaz3216);
          zgaz3217 = bitvector_access(zbase, zgsz3463);
          KILL(sail_int)(&zgsz3463);
        }
      }
      uint64_t zgsz347;
      zgsz347 = UINT64_C(0b0);
      zgsz347 = update_fbits(zgsz347, INT64_C(0), zgaz3217);
      zgaz3218 = zgsz347;
    }
    {
      lbits zgsz3467;
      CREATE(lbits)(&zgsz3467);
      CONVERT_OF(lbits, fbits)(&zgsz3467, UINT64_C(0b0), UINT64_C(1) , true);
      lbits zgsz3468;
      CREATE(lbits)(&zgsz3468);
      CONVERT_OF(lbits, fbits)(&zgsz3468, zgaz3218, UINT64_C(1) , true);
      lbits zgsz3469;
      CREATE(lbits)(&zgsz3469);
      append(&zgsz3469, zgsz3467, zgsz3468);
      zbase2 = CONVERT_OF(fbits, lbits)(zgsz3469, true);
      KILL(lbits)(&zgsz3469);
      KILL(lbits)(&zgsz3468);
      KILL(lbits)(&zgsz3467);
    }
  }
  uint64_t ztop2;
  {
    int64_t zgaz3215;
    {
      sail_int zgsz3473;
      CREATE(sail_int)(&zgsz3473);
      CONVERT_OF(sail_int, mach_int)(&zgsz3473, zcap_addr_width);
      sail_int zgsz3474;
      CREATE(sail_int)(&zgsz3474);
      CONVERT_OF(sail_int, mach_int)(&zgsz3474, INT64_C(1));
      sail_int zgsz3475;
      CREATE(sail_int)(&zgsz3475);
      sub_int(&zgsz3475, zgsz3473, zgsz3474);
      zgaz3215 = CONVERT_OF(mach_int, sail_int)(zgsz3475);
      KILL(sail_int)(&zgsz3475);
      KILL(sail_int)(&zgsz3474);
      KILL(sail_int)(&zgsz3473);
    }
    {
      sail_int zgsz3470;
      CREATE(sail_int)(&zgsz3470);
      CONVERT_OF(sail_int, mach_int)(&zgsz3470, zcap_addr_width);
      sail_int zgsz3471;
      CREATE(sail_int)(&zgsz3471);
      CONVERT_OF(sail_int, mach_int)(&zgsz3471, zgaz3215);
      lbits zgsz3472;
      CREATE(lbits)(&zgsz3472);
      vector_subrange_lbits(&zgsz3472, ztop, zgsz3470, zgsz3471);
      ztop2 = CONVERT_OF(fbits, lbits)(zgsz3472, true);
      KILL(lbits)(&zgsz3472);
      KILL(sail_int)(&zgsz3471);
      KILL(sail_int)(&zgsz3470);
    }
  }
  {
    bool zgaz3210;
    {
      bool zgaz3209;
      {
        int64_t zgaz3206;
        {
          sail_int zgsz3478;
          CREATE(sail_int)(&zgsz3478);
          CONVERT_OF(sail_int, mach_int)(&zgsz3478, zcap_max_E);
          sail_int zgsz3479;
          CREATE(sail_int)(&zgsz3479);
          CONVERT_OF(sail_int, mach_int)(&zgsz3479, INT64_C(1));
          sail_int zgsz3480;
          CREATE(sail_int)(&zgsz3480);
          sub_int(&zgsz3480, zgsz3478, zgsz3479);
          zgaz3206 = CONVERT_OF(mach_int, sail_int)(zgsz3480);
          KILL(sail_int)(&zgsz3480);
          KILL(sail_int)(&zgsz3479);
          KILL(sail_int)(&zgsz3478);
        }
        {
          sail_int zgsz3476;
          CREATE(sail_int)(&zgsz3476);
          CONVERT_OF(sail_int, mach_int)(&zgsz3476, zE);
          sail_int zgsz3477;
          CREATE(sail_int)(&zgsz3477);
          CONVERT_OF(sail_int, mach_int)(&zgsz3477, zgaz3206);
          zgaz3209 = lt(zgsz3476, zgsz3477);
          KILL(sail_int)(&zgsz3477);
          KILL(sail_int)(&zgsz3476);
        }
      }
      bool zgsz348;
      if (zgaz3209) {
        int64_t zgaz3208;
        {
          uint64_t zgaz3207;
          {
            lbits zgsz3483;
            CREATE(lbits)(&zgsz3483);
            CONVERT_OF(lbits, fbits)(&zgsz3483, ztop2, UINT64_C(2) , true);
            lbits zgsz3484;
            CREATE(lbits)(&zgsz3484);
            CONVERT_OF(lbits, fbits)(&zgsz3484, zbase2, UINT64_C(2) , true);
            lbits zgsz3485;
            CREATE(lbits)(&zgsz3485);
            sub_bits(&zgsz3485, zgsz3483, zgsz3484);
            zgaz3207 = CONVERT_OF(fbits, lbits)(zgsz3485, true);
            KILL(lbits)(&zgsz3485);
            KILL(lbits)(&zgsz3484);
            KILL(lbits)(&zgsz3483);
          }
          {
            lbits zgsz3481;
            CREATE(lbits)(&zgsz3481);
            CONVERT_OF(lbits, fbits)(&zgsz3481, zgaz3207, UINT64_C(2) , true);
            sail_int zgsz3482;
            CREATE(sail_int)(&zgsz3482);
            sail_unsigned(&zgsz3482, zgsz3481);
            zgaz3208 = CONVERT_OF(mach_int, sail_int)(zgsz3482);
            KILL(sail_int)(&zgsz3482);
            KILL(lbits)(&zgsz3481);
          }
        }
        {
          sail_int zgsz3486;
          CREATE(sail_int)(&zgsz3486);
          CONVERT_OF(sail_int, mach_int)(&zgsz3486, zgaz3208);
          sail_int zgsz3487;
          CREATE(sail_int)(&zgsz3487);
          CONVERT_OF(sail_int, mach_int)(&zgsz3487, INT64_C(1));
          zgsz348 = gt(zgsz3486, zgsz3487);
          KILL(sail_int)(&zgsz3487);
          KILL(sail_int)(&zgsz3486);
        }
      } else {  zgsz348 = false;  }
      zgaz3210 = zgsz348;
    }
    unit zgsz349;
    if (zgaz3210) {
      fbits zgaz3212;
      {
        fbits zgaz3211;
        {
          sail_int zgsz3488;
          CREATE(sail_int)(&zgsz3488);
          CONVERT_OF(sail_int, mach_int)(&zgsz3488, zcap_addr_width);
          zgaz3211 = bitvector_access(ztop, zgsz3488);
          KILL(sail_int)(&zgsz3488);
        }
        zgaz3212 = sailgen_not_bit(zgaz3211);
      }
      {
        sail_int zgsz3489;
        CREATE(sail_int)(&zgsz3489);
        CONVERT_OF(sail_int, mach_int)(&zgsz3489, zcap_addr_width);
        update_lbits(&ztop, ztop, zgsz3489, zgaz3212);
        KILL(sail_int)(&zgsz3489);
      }
      zgsz349 = UNIT;
    } else {  zgsz349 = UNIT;  }
  }
  uint64_t zgaz3214;
  {
    int64_t zgaz3213;
    {
      sail_int zgsz3493;
      CREATE(sail_int)(&zgsz3493);
      CONVERT_OF(sail_int, mach_int)(&zgsz3493, zcap_addr_width);
      sail_int zgsz3494;
      CREATE(sail_int)(&zgsz3494);
      CONVERT_OF(sail_int, mach_int)(&zgsz3494, INT64_C(1));
      sail_int zgsz3495;
      CREATE(sail_int)(&zgsz3495);
      sub_int(&zgsz3495, zgsz3493, zgsz3494);
      zgaz3213 = CONVERT_OF(mach_int, sail_int)(zgsz3495);
      KILL(sail_int)(&zgsz3495);
      KILL(sail_int)(&zgsz3494);
      KILL(sail_int)(&zgsz3493);
    }
    {
      sail_int zgsz3490;
      CREATE(sail_int)(&zgsz3490);
      CONVERT_OF(sail_int, mach_int)(&zgsz3490, zgaz3213);
      sail_int zgsz3491;
      CREATE(sail_int)(&zgsz3491);
      CONVERT_OF(sail_int, mach_int)(&zgsz3491, INT64_C(0));
      lbits zgsz3492;
      CREATE(lbits)(&zgsz3492);
      vector_subrange_lbits(&zgsz3492, zbase, zgsz3490, zgsz3491);
      zgaz3214 = CONVERT_OF(fbits, lbits)(zgsz3492, true);
      KILL(lbits)(&zgsz3492);
      KILL(sail_int)(&zgsz3491);
      KILL(sail_int)(&zgsz3490);
    }
  }
  struct ztuple_z8z5bv64zCz0z5bvz9 zgsz350;
  CREATE(ztuple_z8z5bv64zCz0z5bvz9)(&zgsz350);
  zgsz350.ztup0 = zgaz3214;
  COPY(lbits)(&((&zgsz350)->ztup1), ztop);
  COPY(ztuple_z8z5bv64zCz0z5bvz9)((*(&zcbz324)), zgsz350);
  KILL(ztuple_z8z5bv64zCz0z5bvz9)(&zgsz350);



  KILL(lbits)(&ztop);
  KILL(lbits)(&zbase);












end_function_108: ;
  goto end_function_198;
end_block_exception_109: ;
  goto end_function_198;
end_function_198: ;
}

// struct tuple_(%i, %i)
struct ztuple_z8z5izCz0z5iz9 {
  sail_int ztup0;
  sail_int ztup1;
};

static void COPY(ztuple_z8z5izCz0z5iz9)(struct ztuple_z8z5izCz0z5iz9 *rop, const struct ztuple_z8z5izCz0z5iz9 op) {
  COPY(sail_int)(&rop->ztup0, op.ztup0);
  COPY(sail_int)(&rop->ztup1, op.ztup1);
}

static void CREATE(ztuple_z8z5izCz0z5iz9)(struct ztuple_z8z5izCz0z5iz9 *op) {
  CREATE(sail_int)(&op->ztup0);
  CREATE(sail_int)(&op->ztup1);
}

static void RECREATE(ztuple_z8z5izCz0z5iz9)(struct ztuple_z8z5izCz0z5iz9 *op) {
  RECREATE(sail_int)(&op->ztup0);
  RECREATE(sail_int)(&op->ztup1);
}

static void KILL(ztuple_z8z5izCz0z5iz9)(struct ztuple_z8z5izCz0z5iz9 *op) {
  KILL(sail_int)(&op->ztup0);
  KILL(sail_int)(&op->ztup1);
}

static bool EQUAL(ztuple_z8z5izCz0z5iz9)(struct ztuple_z8z5izCz0z5iz9 op1, struct ztuple_z8z5izCz0z5iz9 op2) {
  return EQUAL(sail_int)(op1.ztup0, op2.ztup0) && EQUAL(sail_int)(op1.ztup1, op2.ztup1);
}

static void sailgen_getCapBounds(struct ztuple_z8z5izCz0z5iz9 *rop, struct zCapability);


static void sailgen_getCapBounds(struct ztuple_z8z5izCz0z5iz9 *zcbz325, struct zCapability zcap)
{
  __label__ case_112, finish_match_111, end_function_113, end_block_exception_114, end_function_197;

  struct ztuple_z8z5bv64zCz0z5bvz9 zgaz3240;
  CREATE(ztuple_z8z5bv64zCz0z5bvz9)(&zgaz3240);
  sailgen_getCapBoundsBits(&zgaz3240, zcap);
  struct ztuple_z8z5izCz0z5iz9 zgsz351;
  CREATE(ztuple_z8z5izCz0z5iz9)(&zgsz351);
  {
    uint64_t zbase;
    zbase = zgaz3240.ztup0;
    lbits ztop;
    CREATE(lbits)(&ztop);
    COPY(lbits)(&ztop, zgaz3240.ztup1);
    sail_int zgaz3241;
    CREATE(sail_int)(&zgaz3241);
    {
      lbits zgsz3505;
      CREATE(lbits)(&zgsz3505);
      CONVERT_OF(lbits, fbits)(&zgsz3505, zbase, UINT64_C(64) , true);
      sail_unsigned(&zgaz3241, zgsz3505);
      KILL(lbits)(&zgsz3505);
    }
    sail_int zgaz3242;
    CREATE(sail_int)(&zgaz3242);
    sail_unsigned(&zgaz3242, ztop);
    struct ztuple_z8z5izCz0z5iz9 zgsz352;
    CREATE(ztuple_z8z5izCz0z5iz9)(&zgsz352);
    COPY(sail_int)(&((&zgsz352)->ztup0), zgaz3241);
    COPY(sail_int)(&((&zgsz352)->ztup1), zgaz3242);
    COPY(ztuple_z8z5izCz0z5iz9)(&zgsz351, zgsz352);
    KILL(ztuple_z8z5izCz0z5iz9)(&zgsz352);
    KILL(sail_int)(&zgaz3242);
    KILL(sail_int)(&zgaz3241);
    KILL(lbits)(&ztop);
    goto finish_match_111;
  }
case_112: ;
  sail_match_failure("getCapBounds");
finish_match_111: ;
  COPY(ztuple_z8z5izCz0z5iz9)((*(&zcbz325)), zgsz351);
  KILL(ztuple_z8z5izCz0z5iz9)(&zgsz351);
  KILL(ztuple_z8z5bv64zCz0z5bvz9)(&zgaz3240);
end_function_113: ;
  goto end_function_197;
end_block_exception_114: ;
  goto end_function_197;
end_function_197: ;
}

// struct tuple_(%bool, %struct zCapability)
struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 {
  bool ztup0;
  struct zCapability ztup1;
};

static void COPY(ztuple_z8z5boolzCz0z5structz0zzCapabilityz9)(struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 *rop, const struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5boolzCz0z5structz0zzCapabilityz9)(struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 op1, struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 op2) {
  return EQUAL(bool)(op1.ztup0, op2.ztup0) && EQUAL(zCapability)(op1.ztup1, op2.ztup1);
}

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_setCapBounds(struct zCapability, uint64_t, lbits);

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_setCapBounds(struct zCapability zcap, uint64_t zbase, lbits ztop)
{
  __label__ end_function_116, end_block_exception_117;

  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz326;
  lbits zext_base;
  CREATE(lbits)(&zext_base);
  {
    lbits zgsz3612;
    CREATE(lbits)(&zgsz3612);
    CONVERT_OF(lbits, fbits)(&zgsz3612, UINT64_C(0b0), UINT64_C(1) , true);
    lbits zgsz3613;
    CREATE(lbits)(&zgsz3613);
    CONVERT_OF(lbits, fbits)(&zgsz3613, zbase, UINT64_C(64) , true);
    append(&zext_base, zgsz3612, zgsz3613);
    KILL(lbits)(&zgsz3613);
    KILL(lbits)(&zgsz3612);
  }
  lbits zlength;
  CREATE(lbits)(&zlength);
  sub_bits(&zlength, ztop, zext_base);
  int64_t ze;
  {
    int64_t zgaz3276;
    {
      uint64_t zgaz3275;
      {
        int64_t zgaz3274;
        {
          sail_int zgsz3509;
          CREATE(sail_int)(&zgsz3509);
          CONVERT_OF(sail_int, mach_int)(&zgsz3509, zcap_mantissa_width);
          sail_int zgsz3510;
          CREATE(sail_int)(&zgsz3510);
          CONVERT_OF(sail_int, mach_int)(&zgsz3510, INT64_C(1));
          sail_int zgsz3511;
          CREATE(sail_int)(&zgsz3511);
          sub_int(&zgsz3511, zgsz3509, zgsz3510);
          zgaz3274 = CONVERT_OF(mach_int, sail_int)(zgsz3511);
          KILL(sail_int)(&zgsz3511);
          KILL(sail_int)(&zgsz3510);
          KILL(sail_int)(&zgsz3509);
        }
        {
          sail_int zgsz3506;
          CREATE(sail_int)(&zgsz3506);
          CONVERT_OF(sail_int, mach_int)(&zgsz3506, zcap_addr_width);
          sail_int zgsz3507;
          CREATE(sail_int)(&zgsz3507);
          CONVERT_OF(sail_int, mach_int)(&zgsz3507, zgaz3274);
          lbits zgsz3508;
          CREATE(lbits)(&zgsz3508);
          vector_subrange_lbits(&zgsz3508, zlength, zgsz3506, zgsz3507);
          zgaz3275 = CONVERT_OF(fbits, lbits)(zgsz3508, true);
          KILL(lbits)(&zgsz3508);
          KILL(sail_int)(&zgsz3507);
          KILL(sail_int)(&zgsz3506);
        }
      }
      {
        lbits zgsz3512;
        CREATE(lbits)(&zgsz3512);
        CONVERT_OF(lbits, fbits)(&zgsz3512, zgaz3275, UINT64_C(52) , true);
        sail_int zgsz3513;
        CREATE(sail_int)(&zgsz3513);
        count_leading_zeros(&zgsz3513, zgsz3512);
        zgaz3276 = CONVERT_OF(mach_int, sail_int)(zgsz3513);
        KILL(sail_int)(&zgsz3513);
        KILL(lbits)(&zgsz3512);
      }
    }
    {
      sail_int zgsz3514;
      CREATE(sail_int)(&zgsz3514);
      CONVERT_OF(sail_int, mach_int)(&zgsz3514, zcap_max_E);
      sail_int zgsz3515;
      CREATE(sail_int)(&zgsz3515);
      CONVERT_OF(sail_int, mach_int)(&zgsz3515, zgaz3276);
      sail_int zgsz3516;
      CREATE(sail_int)(&zgsz3516);
      sub_int(&zgsz3516, zgsz3514, zgsz3515);
      ze = CONVERT_OF(mach_int, sail_int)(zgsz3516);
      KILL(sail_int)(&zgsz3516);
      KILL(sail_int)(&zgsz3515);
      KILL(sail_int)(&zgsz3514);
    }
  }
  bool zie;
  {
    bool zgaz3273;
    {
      sail_int zgsz3521;
      CREATE(sail_int)(&zgsz3521);
      CONVERT_OF(sail_int, mach_int)(&zgsz3521, ze);
      sail_int zgsz3522;
      CREATE(sail_int)(&zgsz3522);
      CONVERT_OF(sail_int, mach_int)(&zgsz3522, INT64_C(0));
      zgaz3273 = sailgen_neq_int(zgsz3521, zgsz3522);
      KILL(sail_int)(&zgsz3522);
      KILL(sail_int)(&zgsz3521);
    }
    bool zgsz354;
    if (zgaz3273) {  zgsz354 = true;  } else {
      fbits zgaz3272;
      {
        int64_t zgaz3271;
        {
          sail_int zgsz3518;
          CREATE(sail_int)(&zgsz3518);
          CONVERT_OF(sail_int, mach_int)(&zgsz3518, zcap_mantissa_width);
          sail_int zgsz3519;
          CREATE(sail_int)(&zgsz3519);
          CONVERT_OF(sail_int, mach_int)(&zgsz3519, INT64_C(2));
          sail_int zgsz3520;
          CREATE(sail_int)(&zgsz3520);
          sub_int(&zgsz3520, zgsz3518, zgsz3519);
          zgaz3271 = CONVERT_OF(mach_int, sail_int)(zgsz3520);
          KILL(sail_int)(&zgsz3520);
          KILL(sail_int)(&zgsz3519);
          KILL(sail_int)(&zgsz3518);
        }
        {
          sail_int zgsz3517;
          CREATE(sail_int)(&zgsz3517);
          CONVERT_OF(sail_int, mach_int)(&zgsz3517, zgaz3271);
          zgaz3272 = bitvector_access(zlength, zgsz3517);
          KILL(sail_int)(&zgsz3517);
        }
      }
      zgsz354 = eq_bit(zgaz3272, UINT64_C(1));
    }
    zie = zgsz354;
  }
  uint64_t zBbits;
  {
    lbits zgsz3609;
    CREATE(lbits)(&zgsz3609);
    CONVERT_OF(lbits, fbits)(&zgsz3609, zbase, UINT64_C(64) , true);
    sail_int zgsz3610;
    CREATE(sail_int)(&zgsz3610);
    CONVERT_OF(sail_int, mach_int)(&zgsz3610, zcap_mantissa_width);
    lbits zgsz3611;
    CREATE(lbits)(&zgsz3611);
    sail_truncate(&zgsz3611, zgsz3609, zgsz3610);
    zBbits = CONVERT_OF(fbits, lbits)(zgsz3611, true);
    KILL(lbits)(&zgsz3611);
    KILL(sail_int)(&zgsz3610);
    KILL(lbits)(&zgsz3609);
  }
  uint64_t zTbits;
  {
    sail_int zgsz3607;
    CREATE(sail_int)(&zgsz3607);
    CONVERT_OF(sail_int, mach_int)(&zgsz3607, zcap_mantissa_width);
    lbits zgsz3608;
    CREATE(lbits)(&zgsz3608);
    sail_truncate(&zgsz3608, ztop, zgsz3607);
    zTbits = CONVERT_OF(fbits, lbits)(zgsz3608, true);
    KILL(lbits)(&zgsz3608);
    KILL(sail_int)(&zgsz3607);
  }
  bool zlostSignificantTop;
  zlostSignificantTop = false;
  bool zlostSignificantBase;
  zlostSignificantBase = false;
  bool zincE;
  zincE = false;
  {
    unit zgsz355;
    if (zie) {
      uint64_t zB_ie;
      {
        uint64_t zgaz3266;
        {
          int64_t zgaz3265;
          {
            sail_int zgsz3526;
            CREATE(sail_int)(&zgsz3526);
            CONVERT_OF(sail_int, mach_int)(&zgsz3526, ze);
            sail_int zgsz3527;
            CREATE(sail_int)(&zgsz3527);
            CONVERT_OF(sail_int, mach_int)(&zgsz3527, INT64_C(3));
            sail_int zgsz3528;
            CREATE(sail_int)(&zgsz3528);
            add_int(&zgsz3528, zgsz3526, zgsz3527);
            zgaz3265 = CONVERT_OF(mach_int, sail_int)(zgsz3528);
            KILL(sail_int)(&zgsz3528);
            KILL(sail_int)(&zgsz3527);
            KILL(sail_int)(&zgsz3526);
          }
          {
            lbits zgsz3523;
            CREATE(lbits)(&zgsz3523);
            CONVERT_OF(lbits, fbits)(&zgsz3523, zbase, UINT64_C(64) , true);
            sail_int zgsz3524;
            CREATE(sail_int)(&zgsz3524);
            CONVERT_OF(sail_int, mach_int)(&zgsz3524, zgaz3265);
            lbits zgsz3525;
            CREATE(lbits)(&zgsz3525);
            shiftr(&zgsz3525, zgsz3523, zgsz3524);
            zgaz3266 = CONVERT_OF(fbits, lbits)(zgsz3525, true);
            KILL(lbits)(&zgsz3525);
            KILL(sail_int)(&zgsz3524);
            KILL(lbits)(&zgsz3523);
          }
        }
        int64_t zgaz3267;
        {
          sail_int zgsz3532;
          CREATE(sail_int)(&zgsz3532);
          CONVERT_OF(sail_int, mach_int)(&zgsz3532, zcap_mantissa_width);
          sail_int zgsz3533;
          CREATE(sail_int)(&zgsz3533);
          CONVERT_OF(sail_int, mach_int)(&zgsz3533, INT64_C(3));
          sail_int zgsz3534;
          CREATE(sail_int)(&zgsz3534);
          sub_int(&zgsz3534, zgsz3532, zgsz3533);
          zgaz3267 = CONVERT_OF(mach_int, sail_int)(zgsz3534);
          KILL(sail_int)(&zgsz3534);
          KILL(sail_int)(&zgsz3533);
          KILL(sail_int)(&zgsz3532);
        }
        {
          lbits zgsz3529;
          CREATE(lbits)(&zgsz3529);
          CONVERT_OF(lbits, fbits)(&zgsz3529, zgaz3266, UINT64_C(64) , true);
          sail_int zgsz3530;
          CREATE(sail_int)(&zgsz3530);
          CONVERT_OF(sail_int, mach_int)(&zgsz3530, zgaz3267);
          lbits zgsz3531;
          CREATE(lbits)(&zgsz3531);
          sail_truncate(&zgsz3531, zgsz3529, zgsz3530);
          zB_ie = CONVERT_OF(fbits, lbits)(zgsz3531, true);
          KILL(lbits)(&zgsz3531);
          KILL(sail_int)(&zgsz3530);
          KILL(lbits)(&zgsz3529);
        }
      }
      uint64_t zT_ie;
      {
        lbits zgaz3263;
        CREATE(lbits)(&zgaz3263);
        {
          int64_t zgaz3262;
          {
            sail_int zgsz3536;
            CREATE(sail_int)(&zgsz3536);
            CONVERT_OF(sail_int, mach_int)(&zgsz3536, ze);
            sail_int zgsz3537;
            CREATE(sail_int)(&zgsz3537);
            CONVERT_OF(sail_int, mach_int)(&zgsz3537, INT64_C(3));
            sail_int zgsz3538;
            CREATE(sail_int)(&zgsz3538);
            add_int(&zgsz3538, zgsz3536, zgsz3537);
            zgaz3262 = CONVERT_OF(mach_int, sail_int)(zgsz3538);
            KILL(sail_int)(&zgsz3538);
            KILL(sail_int)(&zgsz3537);
            KILL(sail_int)(&zgsz3536);
          }
          {
            sail_int zgsz3535;
            CREATE(sail_int)(&zgsz3535);
            CONVERT_OF(sail_int, mach_int)(&zgsz3535, zgaz3262);
            shiftr(&zgaz3263, ztop, zgsz3535);
            KILL(sail_int)(&zgsz3535);
          }
        }
        int64_t zgaz3264;
        {
          sail_int zgsz3541;
          CREATE(sail_int)(&zgsz3541);
          CONVERT_OF(sail_int, mach_int)(&zgsz3541, zcap_mantissa_width);
          sail_int zgsz3542;
          CREATE(sail_int)(&zgsz3542);
          CONVERT_OF(sail_int, mach_int)(&zgsz3542, INT64_C(3));
          sail_int zgsz3543;
          CREATE(sail_int)(&zgsz3543);
          sub_int(&zgsz3543, zgsz3541, zgsz3542);
          zgaz3264 = CONVERT_OF(mach_int, sail_int)(zgsz3543);
          KILL(sail_int)(&zgsz3543);
          KILL(sail_int)(&zgsz3542);
          KILL(sail_int)(&zgsz3541);
        }
        {
          sail_int zgsz3539;
          CREATE(sail_int)(&zgsz3539);
          CONVERT_OF(sail_int, mach_int)(&zgsz3539, zgaz3264);
          lbits zgsz3540;
          CREATE(lbits)(&zgsz3540);
          sail_truncate(&zgsz3540, zgaz3263, zgsz3539);
          zT_ie = CONVERT_OF(fbits, lbits)(zgsz3540, true);
          KILL(lbits)(&zgsz3540);
          KILL(sail_int)(&zgsz3539);
        }
        KILL(lbits)(&zgaz3263);
      }
      lbits zmaskLo;
      CREATE(lbits)(&zmaskLo);
      {
        int64_t zgaz3260;
        {
          sail_int zgsz3551;
          CREATE(sail_int)(&zgsz3551);
          CONVERT_OF(sail_int, mach_int)(&zgsz3551, INT64_C(64));
          sail_int zgsz3552;
          CREATE(sail_int)(&zgsz3552);
          CONVERT_OF(sail_int, mach_int)(&zgsz3552, INT64_C(1));
          sail_int zgsz3553;
          CREATE(sail_int)(&zgsz3553);
          add_int(&zgsz3553, zgsz3551, zgsz3552);
          zgaz3260 = CONVERT_OF(mach_int, sail_int)(zgsz3553);
          KILL(sail_int)(&zgsz3553);
          KILL(sail_int)(&zgsz3552);
          KILL(sail_int)(&zgsz3551);
        }
        sbits zgaz3261;
        {
          int64_t zgaz3259;
          {
            sail_int zgsz3546;
            CREATE(sail_int)(&zgsz3546);
            CONVERT_OF(sail_int, mach_int)(&zgsz3546, ze);
            sail_int zgsz3547;
            CREATE(sail_int)(&zgsz3547);
            CONVERT_OF(sail_int, mach_int)(&zgsz3547, INT64_C(3));
            sail_int zgsz3548;
            CREATE(sail_int)(&zgsz3548);
            add_int(&zgsz3548, zgsz3546, zgsz3547);
            zgaz3259 = CONVERT_OF(mach_int, sail_int)(zgsz3548);
            KILL(sail_int)(&zgsz3548);
            KILL(sail_int)(&zgsz3547);
            KILL(sail_int)(&zgsz3546);
          }
          {
            sail_int zgsz3544;
            CREATE(sail_int)(&zgsz3544);
            CONVERT_OF(sail_int, mach_int)(&zgsz3544, zgaz3259);
            lbits zgsz3545;
            CREATE(lbits)(&zgsz3545);
            sailgen_ones(&zgsz3545, zgsz3544);
            zgaz3261 = CONVERT_OF(sbits, lbits)(zgsz3545, true);
            KILL(lbits)(&zgsz3545);
            KILL(sail_int)(&zgsz3544);
          }
        }
        {
          sail_int zgsz3549;
          CREATE(sail_int)(&zgsz3549);
          CONVERT_OF(sail_int, mach_int)(&zgsz3549, zgaz3260);
          lbits zgsz3550;
          CREATE(lbits)(&zgsz3550);
          CONVERT_OF(lbits, sbits)(&zgsz3550, zgaz3261, true);
          sailgen_EXTZ(&zmaskLo, zgsz3549, zgsz3550);
          KILL(lbits)(&zgsz3550);
          KILL(sail_int)(&zgsz3549);
        }
      }
      {
        sail_int zgaz3244;
        CREATE(sail_int)(&zgaz3244);
        {
          lbits zgaz3243;
          CREATE(lbits)(&zgaz3243);
          and_bits(&zgaz3243, zext_base, zmaskLo);
          sail_unsigned(&zgaz3244, zgaz3243);
          KILL(lbits)(&zgaz3243);
        }
        {
          sail_int zgsz3554;
          CREATE(sail_int)(&zgsz3554);
          CONVERT_OF(sail_int, mach_int)(&zgsz3554, INT64_C(0));
          zlostSignificantBase = sailgen_neq_int(zgaz3244, zgsz3554);
          KILL(sail_int)(&zgsz3554);
        }
        unit zgsz358;
        zgsz358 = UNIT;
        KILL(sail_int)(&zgaz3244);
      }
      {
        sail_int zgaz3246;
        CREATE(sail_int)(&zgaz3246);
        {
          lbits zgaz3245;
          CREATE(lbits)(&zgaz3245);
          and_bits(&zgaz3245, ztop, zmaskLo);
          sail_unsigned(&zgaz3246, zgaz3245);
          KILL(lbits)(&zgaz3245);
        }
        {
          sail_int zgsz3555;
          CREATE(sail_int)(&zgsz3555);
          CONVERT_OF(sail_int, mach_int)(&zgsz3555, INT64_C(0));
          zlostSignificantTop = sailgen_neq_int(zgaz3246, zgsz3555);
          KILL(sail_int)(&zgsz3555);
        }
        unit zgsz357;
        zgsz357 = UNIT;
        KILL(sail_int)(&zgaz3246);
      }
      {
        unit zgsz356;
        if (zlostSignificantTop) {
          {
            lbits zgsz3556;
            CREATE(lbits)(&zgsz3556);
            CONVERT_OF(lbits, fbits)(&zgsz3556, zT_ie, UINT64_C(11) , true);
            sail_int zgsz3557;
            CREATE(sail_int)(&zgsz3557);
            CONVERT_OF(sail_int, mach_int)(&zgsz3557, INT64_C(1));
            lbits zgsz3558;
            CREATE(lbits)(&zgsz3558);
            add_bits_int(&zgsz3558, zgsz3556, zgsz3557);
            zT_ie = CONVERT_OF(fbits, lbits)(zgsz3558, true);
            KILL(lbits)(&zgsz3558);
            KILL(sail_int)(&zgsz3557);
            KILL(lbits)(&zgsz3556);
          }
          zgsz356 = UNIT;
        } else {  zgsz356 = UNIT;  }
      }
      uint64_t zlen_ie;
      {
        lbits zgsz3598;
        CREATE(lbits)(&zgsz3598);
        CONVERT_OF(lbits, fbits)(&zgsz3598, zT_ie, UINT64_C(11) , true);
        lbits zgsz3599;
        CREATE(lbits)(&zgsz3599);
        CONVERT_OF(lbits, fbits)(&zgsz3599, zB_ie, UINT64_C(11) , true);
        lbits zgsz3600;
        CREATE(lbits)(&zgsz3600);
        sub_bits(&zgsz3600, zgsz3598, zgsz3599);
        zlen_ie = CONVERT_OF(fbits, lbits)(zgsz3600, true);
        KILL(lbits)(&zgsz3600);
        KILL(lbits)(&zgsz3599);
        KILL(lbits)(&zgsz3598);
      }
      {
        bool zgaz3249;
        {
          fbits zgaz3248;
          {
            int64_t zgaz3247;
            {
              sail_int zgsz3561;
              CREATE(sail_int)(&zgsz3561);
              CONVERT_OF(sail_int, mach_int)(&zgsz3561, zcap_mantissa_width);
              sail_int zgsz3562;
              CREATE(sail_int)(&zgsz3562);
              CONVERT_OF(sail_int, mach_int)(&zgsz3562, INT64_C(4));
              sail_int zgsz3563;
              CREATE(sail_int)(&zgsz3563);
              sub_int(&zgsz3563, zgsz3561, zgsz3562);
              zgaz3247 = CONVERT_OF(mach_int, sail_int)(zgsz3563);
              KILL(sail_int)(&zgsz3563);
              KILL(sail_int)(&zgsz3562);
              KILL(sail_int)(&zgsz3561);
            }
            {
              lbits zgsz3559;
              CREATE(lbits)(&zgsz3559);
              CONVERT_OF(lbits, fbits)(&zgsz3559, zlen_ie, UINT64_C(11) , true);
              sail_int zgsz3560;
              CREATE(sail_int)(&zgsz3560);
              CONVERT_OF(sail_int, mach_int)(&zgsz3560, zgaz3247);
              zgaz3248 = bitvector_access(zgsz3559, zgsz3560);
              KILL(sail_int)(&zgsz3560);
              KILL(lbits)(&zgsz3559);
            }
          }
          zgaz3249 = eq_bit(zgaz3248, UINT64_C(1));
        }
        unit zgsz360;
        if (zgaz3249) {
          {
            zincE = true;
            unit zgsz366;
            zgsz366 = UNIT;
          }
          {
            bool zgsz361;
            if (zlostSignificantBase) {  zgsz361 = true;  } else {
              fbits zgaz3250;
              {
                lbits zgsz3564;
                CREATE(lbits)(&zgsz3564);
                CONVERT_OF(lbits, fbits)(&zgsz3564, zB_ie, UINT64_C(11) , true);
                sail_int zgsz3565;
                CREATE(sail_int)(&zgsz3565);
                CONVERT_OF(sail_int, mach_int)(&zgsz3565, INT64_C(0));
                zgaz3250 = bitvector_access(zgsz3564, zgsz3565);
                KILL(sail_int)(&zgsz3565);
                KILL(lbits)(&zgsz3564);
              }
              zgsz361 = eq_bit(zgaz3250, UINT64_C(1));
            }
            zlostSignificantBase = zgsz361;
            unit zgsz365;
            zgsz365 = UNIT;
          }
          {
            bool zgsz362;
            if (zlostSignificantTop) {  zgsz362 = true;  } else {
              fbits zgaz3251;
              {
                lbits zgsz3566;
                CREATE(lbits)(&zgsz3566);
                CONVERT_OF(lbits, fbits)(&zgsz3566, zT_ie, UINT64_C(11) , true);
                sail_int zgsz3567;
                CREATE(sail_int)(&zgsz3567);
                CONVERT_OF(sail_int, mach_int)(&zgsz3567, INT64_C(0));
                zgaz3251 = bitvector_access(zgsz3566, zgsz3567);
                KILL(sail_int)(&zgsz3567);
                KILL(lbits)(&zgsz3566);
              }
              zgsz362 = eq_bit(zgaz3251, UINT64_C(1));
            }
            zlostSignificantTop = zgsz362;
            unit zgsz364;
            zgsz364 = UNIT;
          }
          {
            uint64_t zgaz3253;
            {
              int64_t zgaz3252;
              {
                sail_int zgsz3571;
                CREATE(sail_int)(&zgsz3571);
                CONVERT_OF(sail_int, mach_int)(&zgsz3571, ze);
                sail_int zgsz3572;
                CREATE(sail_int)(&zgsz3572);
                CONVERT_OF(sail_int, mach_int)(&zgsz3572, INT64_C(4));
                sail_int zgsz3573;
                CREATE(sail_int)(&zgsz3573);
                add_int(&zgsz3573, zgsz3571, zgsz3572);
                zgaz3252 = CONVERT_OF(mach_int, sail_int)(zgsz3573);
                KILL(sail_int)(&zgsz3573);
                KILL(sail_int)(&zgsz3572);
                KILL(sail_int)(&zgsz3571);
              }
              {
                lbits zgsz3568;
                CREATE(lbits)(&zgsz3568);
                CONVERT_OF(lbits, fbits)(&zgsz3568, zbase, UINT64_C(64) , true);
                sail_int zgsz3569;
                CREATE(sail_int)(&zgsz3569);
                CONVERT_OF(sail_int, mach_int)(&zgsz3569, zgaz3252);
                lbits zgsz3570;
                CREATE(lbits)(&zgsz3570);
                shiftr(&zgsz3570, zgsz3568, zgsz3569);
                zgaz3253 = CONVERT_OF(fbits, lbits)(zgsz3570, true);
                KILL(lbits)(&zgsz3570);
                KILL(sail_int)(&zgsz3569);
                KILL(lbits)(&zgsz3568);
              }
            }
            int64_t zgaz3254;
            {
              sail_int zgsz3577;
              CREATE(sail_int)(&zgsz3577);
              CONVERT_OF(sail_int, mach_int)(&zgsz3577, zcap_mantissa_width);
              sail_int zgsz3578;
              CREATE(sail_int)(&zgsz3578);
              CONVERT_OF(sail_int, mach_int)(&zgsz3578, INT64_C(3));
              sail_int zgsz3579;
              CREATE(sail_int)(&zgsz3579);
              sub_int(&zgsz3579, zgsz3577, zgsz3578);
              zgaz3254 = CONVERT_OF(mach_int, sail_int)(zgsz3579);
              KILL(sail_int)(&zgsz3579);
              KILL(sail_int)(&zgsz3578);
              KILL(sail_int)(&zgsz3577);
            }
            {
              lbits zgsz3574;
              CREATE(lbits)(&zgsz3574);
              CONVERT_OF(lbits, fbits)(&zgsz3574, zgaz3253, UINT64_C(64) , true);
              sail_int zgsz3575;
              CREATE(sail_int)(&zgsz3575);
              CONVERT_OF(sail_int, mach_int)(&zgsz3575, zgaz3254);
              lbits zgsz3576;
              CREATE(lbits)(&zgsz3576);
              sail_truncate(&zgsz3576, zgsz3574, zgsz3575);
              zB_ie = CONVERT_OF(fbits, lbits)(zgsz3576, true);
              KILL(lbits)(&zgsz3576);
              KILL(sail_int)(&zgsz3575);
              KILL(lbits)(&zgsz3574);
            }
            unit zgsz363;
            zgsz363 = UNIT;
          }
          int64_t zincT;
          if (zlostSignificantTop) {  zincT = INT64_C(1);  } else {  zincT = INT64_C(0);  }
          uint64_t zgaz3258;
          {
            lbits zgaz3256;
            CREATE(lbits)(&zgaz3256);
            {
              int64_t zgaz3255;
              {
                sail_int zgsz3581;
                CREATE(sail_int)(&zgsz3581);
                CONVERT_OF(sail_int, mach_int)(&zgsz3581, ze);
                sail_int zgsz3582;
                CREATE(sail_int)(&zgsz3582);
                CONVERT_OF(sail_int, mach_int)(&zgsz3582, INT64_C(4));
                sail_int zgsz3583;
                CREATE(sail_int)(&zgsz3583);
                add_int(&zgsz3583, zgsz3581, zgsz3582);
                zgaz3255 = CONVERT_OF(mach_int, sail_int)(zgsz3583);
                KILL(sail_int)(&zgsz3583);
                KILL(sail_int)(&zgsz3582);
                KILL(sail_int)(&zgsz3581);
              }
              {
                sail_int zgsz3580;
                CREATE(sail_int)(&zgsz3580);
                CONVERT_OF(sail_int, mach_int)(&zgsz3580, zgaz3255);
                shiftr(&zgaz3256, ztop, zgsz3580);
                KILL(sail_int)(&zgsz3580);
              }
            }
            int64_t zgaz3257;
            {
              sail_int zgsz3586;
              CREATE(sail_int)(&zgsz3586);
              CONVERT_OF(sail_int, mach_int)(&zgsz3586, zcap_mantissa_width);
              sail_int zgsz3587;
              CREATE(sail_int)(&zgsz3587);
              CONVERT_OF(sail_int, mach_int)(&zgsz3587, INT64_C(3));
              sail_int zgsz3588;
              CREATE(sail_int)(&zgsz3588);
              sub_int(&zgsz3588, zgsz3586, zgsz3587);
              zgaz3257 = CONVERT_OF(mach_int, sail_int)(zgsz3588);
              KILL(sail_int)(&zgsz3588);
              KILL(sail_int)(&zgsz3587);
              KILL(sail_int)(&zgsz3586);
            }
            {
              sail_int zgsz3584;
              CREATE(sail_int)(&zgsz3584);
              CONVERT_OF(sail_int, mach_int)(&zgsz3584, zgaz3257);
              lbits zgsz3585;
              CREATE(lbits)(&zgsz3585);
              sail_truncate(&zgsz3585, zgaz3256, zgsz3584);
              zgaz3258 = CONVERT_OF(fbits, lbits)(zgsz3585, true);
              KILL(lbits)(&zgsz3585);
              KILL(sail_int)(&zgsz3584);
            }
            KILL(lbits)(&zgaz3256);
          }
          {
            lbits zgsz3589;
            CREATE(lbits)(&zgsz3589);
            CONVERT_OF(lbits, fbits)(&zgsz3589, zgaz3258, UINT64_C(11) , true);
            sail_int zgsz3590;
            CREATE(sail_int)(&zgsz3590);
            CONVERT_OF(sail_int, mach_int)(&zgsz3590, zincT);
            lbits zgsz3591;
            CREATE(lbits)(&zgsz3591);
            add_bits_int(&zgsz3591, zgsz3589, zgsz3590);
            zT_ie = CONVERT_OF(fbits, lbits)(zgsz3591, true);
            KILL(lbits)(&zgsz3591);
            KILL(sail_int)(&zgsz3590);
            KILL(lbits)(&zgsz3589);
          }
          zgsz360 = UNIT;
        } else {  zgsz360 = UNIT;  }
      }
      {
        {
          lbits zgsz3592;
          CREATE(lbits)(&zgsz3592);
          CONVERT_OF(lbits, fbits)(&zgsz3592, zB_ie, UINT64_C(11) , true);
          lbits zgsz3593;
          CREATE(lbits)(&zgsz3593);
          CONVERT_OF(lbits, fbits)(&zgsz3593, UINT64_C(0b000), UINT64_C(3) , true);
          lbits zgsz3594;
          CREATE(lbits)(&zgsz3594);
          append(&zgsz3594, zgsz3592, zgsz3593);
          zBbits = CONVERT_OF(fbits, lbits)(zgsz3594, true);
          KILL(lbits)(&zgsz3594);
          KILL(lbits)(&zgsz3593);
          KILL(lbits)(&zgsz3592);
        }
        unit zgsz359;
        zgsz359 = UNIT;
      }
      {
        lbits zgsz3595;
        CREATE(lbits)(&zgsz3595);
        CONVERT_OF(lbits, fbits)(&zgsz3595, zT_ie, UINT64_C(11) , true);
        lbits zgsz3596;
        CREATE(lbits)(&zgsz3596);
        CONVERT_OF(lbits, fbits)(&zgsz3596, UINT64_C(0b000), UINT64_C(3) , true);
        lbits zgsz3597;
        CREATE(lbits)(&zgsz3597);
        append(&zgsz3597, zgsz3595, zgsz3596);
        zTbits = CONVERT_OF(fbits, lbits)(zgsz3597, true);
        KILL(lbits)(&zgsz3597);
        KILL(lbits)(&zgsz3596);
        KILL(lbits)(&zgsz3595);
      }
      zgsz355 = UNIT;
      KILL(lbits)(&zmaskLo);
    } else {  zgsz355 = UNIT;  }
  }
  struct zCapability znewCap;
  {
    uint64_t zgaz3270;
    {
      int64_t zgaz3269;
      if (zincE) {
        {
          sail_int zgsz3601;
          CREATE(sail_int)(&zgsz3601);
          CONVERT_OF(sail_int, mach_int)(&zgsz3601, ze);
          sail_int zgsz3602;
          CREATE(sail_int)(&zgsz3602);
          CONVERT_OF(sail_int, mach_int)(&zgsz3602, INT64_C(1));
          sail_int zgsz3603;
          CREATE(sail_int)(&zgsz3603);
          add_int(&zgsz3603, zgsz3601, zgsz3602);
          zgaz3269 = CONVERT_OF(mach_int, sail_int)(zgsz3603);
          KILL(sail_int)(&zgsz3603);
          KILL(sail_int)(&zgsz3602);
          KILL(sail_int)(&zgsz3601);
        }
      } else {  zgaz3269 = ze;  }
      {
        sail_int zgsz3604;
        CREATE(sail_int)(&zgsz3604);
        CONVERT_OF(sail_int, mach_int)(&zgsz3604, INT64_C(6));
        sail_int zgsz3605;
        CREATE(sail_int)(&zgsz3605);
        CONVERT_OF(sail_int, mach_int)(&zgsz3605, zgaz3269);
        lbits zgsz3606;
        CREATE(lbits)(&zgsz3606);
        sailgen_to_bits(&zgsz3606, zgsz3604, zgsz3605);
        zgaz3270 = CONVERT_OF(fbits, lbits)(zgsz3606, true);
        KILL(lbits)(&zgsz3606);
        KILL(sail_int)(&zgsz3605);
        KILL(sail_int)(&zgsz3604);
      }
    }
    struct zCapability zgsz367;
    zgsz367 = zcap;
    zgsz367.zB = zBbits;
    zgsz367.zE = zgaz3270;
    zgsz367.zT = zTbits;
    zgsz367.zaddress = zbase;
    zgsz367.zinternal_E = zie;
    znewCap = zgsz367;
  }
  bool zexact;
  {
    bool zgaz3268;
    {
      bool zgsz368;
      if (zlostSignificantBase) {  zgsz368 = true;  } else {  zgsz368 = zlostSignificantTop;  }
      zgaz3268 = zgsz368;
    }
    zexact = not(zgaz3268);
  }
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz369;
  zgsz369.ztup0 = zexact;
  zgsz369.ztup1 = znewCap;
  zcbz326 = zgsz369;










  KILL(lbits)(&zlength);
  KILL(lbits)(&zext_base);
end_function_116: ;
  return zcbz326;
end_block_exception_117: ;
  struct zCapability zcbz353 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz352 = { .ztup1 = zcbz353, .ztup0 = false };
  return zcbz352;
}

static uint64_t sailgen_getCapPerms(struct zCapability);

static uint64_t sailgen_getCapPerms(struct zCapability zcap)
{
  __label__ end_function_119, end_block_exception_120;

  uint64_t zcbz327;
  uint64_t zperms;
  {
    uint64_t zgaz3278;
    zgaz3278 = sailgen_getCapHardPerms(zcap);
    {
      sail_int zgsz3614;
      CREATE(sail_int)(&zgsz3614);
      CONVERT_OF(sail_int, mach_int)(&zgsz3614, INT64_C(15));
      lbits zgsz3615;
      CREATE(lbits)(&zgsz3615);
      CONVERT_OF(lbits, fbits)(&zgsz3615, zgaz3278, UINT64_C(12) , true);
      lbits zgsz3616;
      CREATE(lbits)(&zgsz3616);
      sailgen_EXTZ(&zgsz3616, zgsz3614, zgsz3615);
      zperms = CONVERT_OF(fbits, lbits)(zgsz3616, true);
      KILL(lbits)(&zgsz3616);
      KILL(lbits)(&zgsz3615);
      KILL(sail_int)(&zgsz3614);
    }
  }
  uint64_t zgaz3277;
  zgaz3277 = zcap.zuperms;
  {
    lbits zgsz3617;
    CREATE(lbits)(&zgsz3617);
    CONVERT_OF(lbits, fbits)(&zgsz3617, zgaz3277, UINT64_C(4) , true);
    lbits zgsz3618;
    CREATE(lbits)(&zgsz3618);
    CONVERT_OF(lbits, fbits)(&zgsz3618, zperms, UINT64_C(15) , true);
    lbits zgsz3619;
    CREATE(lbits)(&zgsz3619);
    append(&zgsz3619, zgsz3617, zgsz3618);
    zcbz327 = CONVERT_OF(fbits, lbits)(zgsz3619, true);
    KILL(lbits)(&zgsz3619);
    KILL(lbits)(&zgsz3618);
    KILL(lbits)(&zgsz3617);
  }


end_function_119: ;
  return zcbz327;
end_block_exception_120: ;

  return UINT64_C(0xdeadc0de);
}

static struct zCapability sailgen_setCapPerms(struct zCapability, uint64_t);

static struct zCapability sailgen_setCapPerms(struct zCapability zcap, uint64_t zperms)
{
  __label__ end_function_122, end_block_exception_123;

  struct zCapability zcbz328;
  uint64_t zgaz3280;
  {
    uint64_t zgaz3279;
    {
      lbits zgsz3623;
      CREATE(lbits)(&zgsz3623);
      CONVERT_OF(lbits, fbits)(&zgsz3623, zperms, UINT64_C(19) , true);
      sail_int zgsz3624;
      CREATE(sail_int)(&zgsz3624);
      CONVERT_OF(sail_int, mach_int)(&zgsz3624, zcap_uperms_shift);
      lbits zgsz3625;
      CREATE(lbits)(&zgsz3625);
      shiftr(&zgsz3625, zgsz3623, zgsz3624);
      zgaz3279 = CONVERT_OF(fbits, lbits)(zgsz3625, true);
      KILL(lbits)(&zgsz3625);
      KILL(sail_int)(&zgsz3624);
      KILL(lbits)(&zgsz3623);
    }
    {
      lbits zgsz3620;
      CREATE(lbits)(&zgsz3620);
      CONVERT_OF(lbits, fbits)(&zgsz3620, zgaz3279, UINT64_C(19) , true);
      sail_int zgsz3621;
      CREATE(sail_int)(&zgsz3621);
      CONVERT_OF(sail_int, mach_int)(&zgsz3621, zcap_uperms_width);
      lbits zgsz3622;
      CREATE(lbits)(&zgsz3622);
      sail_truncate(&zgsz3622, zgsz3620, zgsz3621);
      zgaz3280 = CONVERT_OF(fbits, lbits)(zgsz3622, true);
      KILL(lbits)(&zgsz3622);
      KILL(sail_int)(&zgsz3621);
      KILL(lbits)(&zgsz3620);
    }
  }
  bool zgaz3282;
  {
    fbits zgaz3281;
    {
      lbits zgsz3626;
      CREATE(lbits)(&zgsz3626);
      CONVERT_OF(lbits, fbits)(&zgsz3626, zperms, UINT64_C(19) , true);
      sail_int zgsz3627;
      CREATE(sail_int)(&zgsz3627);
      CONVERT_OF(sail_int, mach_int)(&zgsz3627, INT64_C(11));
      zgaz3281 = bitvector_access(zgsz3626, zgsz3627);
      KILL(sail_int)(&zgsz3627);
      KILL(lbits)(&zgsz3626);
    }
    zgaz3282 = sailgen_bit_to_bool(zgaz3281);
  }
  bool zgaz3284;
  {
    fbits zgaz3283;
    {
      lbits zgsz3628;
      CREATE(lbits)(&zgsz3628);
      CONVERT_OF(lbits, fbits)(&zgsz3628, zperms, UINT64_C(19) , true);
      sail_int zgsz3629;
      CREATE(sail_int)(&zgsz3629);
      CONVERT_OF(sail_int, mach_int)(&zgsz3629, INT64_C(10));
      zgaz3283 = bitvector_access(zgsz3628, zgsz3629);
      KILL(sail_int)(&zgsz3629);
      KILL(lbits)(&zgsz3628);
    }
    zgaz3284 = sailgen_bit_to_bool(zgaz3283);
  }
  bool zgaz3286;
  {
    fbits zgaz3285;
    {
      lbits zgsz3630;
      CREATE(lbits)(&zgsz3630);
      CONVERT_OF(lbits, fbits)(&zgsz3630, zperms, UINT64_C(19) , true);
      sail_int zgsz3631;
      CREATE(sail_int)(&zgsz3631);
      CONVERT_OF(sail_int, mach_int)(&zgsz3631, INT64_C(9));
      zgaz3285 = bitvector_access(zgsz3630, zgsz3631);
      KILL(sail_int)(&zgsz3631);
      KILL(lbits)(&zgsz3630);
    }
    zgaz3286 = sailgen_bit_to_bool(zgaz3285);
  }
  bool zgaz3288;
  {
    fbits zgaz3287;
    {
      lbits zgsz3632;
      CREATE(lbits)(&zgsz3632);
      CONVERT_OF(lbits, fbits)(&zgsz3632, zperms, UINT64_C(19) , true);
      sail_int zgsz3633;
      CREATE(sail_int)(&zgsz3633);
      CONVERT_OF(sail_int, mach_int)(&zgsz3633, INT64_C(8));
      zgaz3287 = bitvector_access(zgsz3632, zgsz3633);
      KILL(sail_int)(&zgsz3633);
      KILL(lbits)(&zgsz3632);
    }
    zgaz3288 = sailgen_bit_to_bool(zgaz3287);
  }
  bool zgaz3290;
  {
    fbits zgaz3289;
    {
      lbits zgsz3634;
      CREATE(lbits)(&zgsz3634);
      CONVERT_OF(lbits, fbits)(&zgsz3634, zperms, UINT64_C(19) , true);
      sail_int zgsz3635;
      CREATE(sail_int)(&zgsz3635);
      CONVERT_OF(sail_int, mach_int)(&zgsz3635, INT64_C(7));
      zgaz3289 = bitvector_access(zgsz3634, zgsz3635);
      KILL(sail_int)(&zgsz3635);
      KILL(lbits)(&zgsz3634);
    }
    zgaz3290 = sailgen_bit_to_bool(zgaz3289);
  }
  bool zgaz3292;
  {
    fbits zgaz3291;
    {
      lbits zgsz3636;
      CREATE(lbits)(&zgsz3636);
      CONVERT_OF(lbits, fbits)(&zgsz3636, zperms, UINT64_C(19) , true);
      sail_int zgsz3637;
      CREATE(sail_int)(&zgsz3637);
      CONVERT_OF(sail_int, mach_int)(&zgsz3637, INT64_C(6));
      zgaz3291 = bitvector_access(zgsz3636, zgsz3637);
      KILL(sail_int)(&zgsz3637);
      KILL(lbits)(&zgsz3636);
    }
    zgaz3292 = sailgen_bit_to_bool(zgaz3291);
  }
  bool zgaz3294;
  {
    fbits zgaz3293;
    {
      lbits zgsz3638;
      CREATE(lbits)(&zgsz3638);
      CONVERT_OF(lbits, fbits)(&zgsz3638, zperms, UINT64_C(19) , true);
      sail_int zgsz3639;
      CREATE(sail_int)(&zgsz3639);
      CONVERT_OF(sail_int, mach_int)(&zgsz3639, INT64_C(5));
      zgaz3293 = bitvector_access(zgsz3638, zgsz3639);
      KILL(sail_int)(&zgsz3639);
      KILL(lbits)(&zgsz3638);
    }
    zgaz3294 = sailgen_bit_to_bool(zgaz3293);
  }
  bool zgaz3296;
  {
    fbits zgaz3295;
    {
      lbits zgsz3640;
      CREATE(lbits)(&zgsz3640);
      CONVERT_OF(lbits, fbits)(&zgsz3640, zperms, UINT64_C(19) , true);
      sail_int zgsz3641;
      CREATE(sail_int)(&zgsz3641);
      CONVERT_OF(sail_int, mach_int)(&zgsz3641, INT64_C(4));
      zgaz3295 = bitvector_access(zgsz3640, zgsz3641);
      KILL(sail_int)(&zgsz3641);
      KILL(lbits)(&zgsz3640);
    }
    zgaz3296 = sailgen_bit_to_bool(zgaz3295);
  }
  bool zgaz3298;
  {
    fbits zgaz3297;
    {
      lbits zgsz3642;
      CREATE(lbits)(&zgsz3642);
      CONVERT_OF(lbits, fbits)(&zgsz3642, zperms, UINT64_C(19) , true);
      sail_int zgsz3643;
      CREATE(sail_int)(&zgsz3643);
      CONVERT_OF(sail_int, mach_int)(&zgsz3643, INT64_C(3));
      zgaz3297 = bitvector_access(zgsz3642, zgsz3643);
      KILL(sail_int)(&zgsz3643);
      KILL(lbits)(&zgsz3642);
    }
    zgaz3298 = sailgen_bit_to_bool(zgaz3297);
  }
  bool zgaz3300;
  {
    fbits zgaz3299;
    {
      lbits zgsz3644;
      CREATE(lbits)(&zgsz3644);
      CONVERT_OF(lbits, fbits)(&zgsz3644, zperms, UINT64_C(19) , true);
      sail_int zgsz3645;
      CREATE(sail_int)(&zgsz3645);
      CONVERT_OF(sail_int, mach_int)(&zgsz3645, INT64_C(2));
      zgaz3299 = bitvector_access(zgsz3644, zgsz3645);
      KILL(sail_int)(&zgsz3645);
      KILL(lbits)(&zgsz3644);
    }
    zgaz3300 = sailgen_bit_to_bool(zgaz3299);
  }
  bool zgaz3302;
  {
    fbits zgaz3301;
    {
      lbits zgsz3646;
      CREATE(lbits)(&zgsz3646);
      CONVERT_OF(lbits, fbits)(&zgsz3646, zperms, UINT64_C(19) , true);
      sail_int zgsz3647;
      CREATE(sail_int)(&zgsz3647);
      CONVERT_OF(sail_int, mach_int)(&zgsz3647, INT64_C(1));
      zgaz3301 = bitvector_access(zgsz3646, zgsz3647);
      KILL(sail_int)(&zgsz3647);
      KILL(lbits)(&zgsz3646);
    }
    zgaz3302 = sailgen_bit_to_bool(zgaz3301);
  }
  bool zgaz3304;
  {
    fbits zgaz3303;
    {
      lbits zgsz3648;
      CREATE(lbits)(&zgsz3648);
      CONVERT_OF(lbits, fbits)(&zgsz3648, zperms, UINT64_C(19) , true);
      sail_int zgsz3649;
      CREATE(sail_int)(&zgsz3649);
      CONVERT_OF(sail_int, mach_int)(&zgsz3649, INT64_C(0));
      zgaz3303 = bitvector_access(zgsz3648, zgsz3649);
      KILL(sail_int)(&zgsz3649);
      KILL(lbits)(&zgsz3648);
    }
    zgaz3304 = sailgen_bit_to_bool(zgaz3303);
  }
  struct zCapability zgsz370;
  zgsz370 = zcap;
  zgsz370.zaccess_system_regs = zgaz3284;
  zgsz370.zglobal = zgaz3304;
  zgsz370.zpermit_cinvoke = zgaz3288;
  zgsz370.zpermit_execute = zgaz3302;
  zgsz370.zpermit_load = zgaz3300;
  zgsz370.zpermit_load_cap = zgaz3296;
  zgsz370.zpermit_seal = zgaz3290;
  zgsz370.zpermit_set_CID = zgaz3282;
  zgsz370.zpermit_store = zgaz3298;
  zgsz370.zpermit_store_cap = zgaz3294;
  zgsz370.zpermit_store_local_cap = zgaz3292;
  zgsz370.zpermit_unseal = zgaz3286;
  zgsz370.zuperms = zgaz3280;
  zcbz328 = zgsz370;














end_function_122: ;
  return zcbz328;
end_block_exception_123: ;
  struct zCapability zcbz354 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz354;
}

static uint64_t sailgen_getCapFlags(struct zCapability);

static uint64_t sailgen_getCapFlags(struct zCapability zcap)
{
  __label__ end_function_125, end_block_exception_126;

  uint64_t zcbz329;
  bool zgaz3305;
  zgaz3305 = zcap.zflag_cap_mode;
  zcbz329 = sailgen_bool_to_bits(zgaz3305);

end_function_125: ;
  return zcbz329;
end_block_exception_126: ;

  return UINT64_C(0xdeadc0de);
}

static bool sailgen_isCapSealed(struct zCapability);

static bool sailgen_isCapSealed(struct zCapability zcap)
{
  __label__ end_function_128, end_block_exception_129;

  bool zcbz330;
  int64_t zgaz3307;
  {
    uint64_t zgaz3306;
    zgaz3306 = zcap.zotype;
    {
      lbits zgsz3650;
      CREATE(lbits)(&zgsz3650);
      CONVERT_OF(lbits, fbits)(&zgsz3650, zgaz3306, UINT64_C(18) , true);
      sail_int zgsz3651;
      CREATE(sail_int)(&zgsz3651);
      sail_signed(&zgsz3651, zgsz3650);
      zgaz3307 = CONVERT_OF(mach_int, sail_int)(zgsz3651);
      KILL(sail_int)(&zgsz3651);
      KILL(lbits)(&zgsz3650);
    }
  }
  {
    sail_int zgsz3652;
    CREATE(sail_int)(&zgsz3652);
    CONVERT_OF(sail_int, mach_int)(&zgsz3652, zgaz3307);
    sail_int zgsz3653;
    CREATE(sail_int)(&zgsz3653);
    CONVERT_OF(sail_int, mach_int)(&zgsz3653, zotype_unsealed);
    zcbz330 = sailgen_neq_int(zgsz3652, zgsz3653);
    KILL(sail_int)(&zgsz3653);
    KILL(sail_int)(&zgsz3652);
  }

end_function_128: ;
  return zcbz330;
end_block_exception_129: ;

  return false;
}

static bool sailgen_hasReservedOType(struct zCapability);

static bool sailgen_hasReservedOType(struct zCapability zcap)
{
  __label__ end_function_131, end_block_exception_132;

  bool zcbz331;
  int64_t zgaz3309;
  {
    uint64_t zgaz3308;
    zgaz3308 = zcap.zotype;
    {
      lbits zgsz3654;
      CREATE(lbits)(&zgsz3654);
      CONVERT_OF(lbits, fbits)(&zgsz3654, zgaz3308, UINT64_C(18) , true);
      sail_int zgsz3655;
      CREATE(sail_int)(&zgsz3655);
      sail_unsigned(&zgsz3655, zgsz3654);
      zgaz3309 = CONVERT_OF(mach_int, sail_int)(zgsz3655);
      KILL(sail_int)(&zgsz3655);
      KILL(lbits)(&zgsz3654);
    }
  }
  {
    sail_int zgsz3656;
    CREATE(sail_int)(&zgsz3656);
    CONVERT_OF(sail_int, mach_int)(&zgsz3656, zgaz3309);
    sail_int zgsz3657;
    CREATE(sail_int)(&zgsz3657);
    CONVERT_OF(sail_int, mach_int)(&zgsz3657, zcap_max_otype);
    zcbz331 = gt(zgsz3656, zgsz3657);
    KILL(sail_int)(&zgsz3657);
    KILL(sail_int)(&zgsz3656);
  }

end_function_131: ;
  return zcbz331;
end_block_exception_132: ;

  return false;
}

static uint64_t sailgen_getCapBaseBits(struct zCapability);

static uint64_t sailgen_getCapBaseBits(struct zCapability zc)
{
  __label__ case_135, finish_match_134, end_function_136, end_block_exception_137;

  uint64_t zcbz332;
  struct ztuple_z8z5bv64zCz0z5bvz9 zgaz3310;
  CREATE(ztuple_z8z5bv64zCz0z5bvz9)(&zgaz3310);
  sailgen_getCapBoundsBits(&zgaz3310, zc);
  uint64_t zgsz371;
  {
    uint64_t zbase;
    zbase = zgaz3310.ztup0;
    zgsz371 = zbase;
    goto finish_match_134;
  }
case_135: ;
  sail_match_failure("getCapBaseBits");
finish_match_134: ;
  zcbz332 = zgsz371;

  KILL(ztuple_z8z5bv64zCz0z5bvz9)(&zgaz3310);
end_function_136: ;
  return zcbz332;
end_block_exception_137: ;

  return UINT64_C(0xdeadc0de);
}

static void sailgen_getCapLength(sail_int *rop, struct zCapability);

static void sailgen_getCapLength(sail_int *zcbz333, struct zCapability zc)
{
  __label__ case_140, finish_match_139, end_function_141, end_block_exception_142, end_function_196;

  struct ztuple_z8z5izCz0z5iz9 zgaz3311;
  CREATE(ztuple_z8z5izCz0z5iz9)(&zgaz3311);
  sailgen_getCapBounds(&zgaz3311, zc);
  sail_int zgsz373;
  CREATE(sail_int)(&zgsz373);
  {
    sail_int zbase;
    CREATE(sail_int)(&zbase);
    COPY(sail_int)(&zbase, zgaz3311.ztup0);
    sail_int ztop;
    CREATE(sail_int)(&ztop);
    COPY(sail_int)(&ztop, zgaz3311.ztup1);
    {
      bool zgaz3314;
      {
        bool zgaz3313;
        {
          bool zgaz3312;
          zgaz3312 = zc.ztag;
          zgaz3313 = not(zgaz3312);
        }
        bool zgsz374;
        if (zgaz3313) {  zgsz374 = true;  } else {  zgsz374 = gteq(ztop, zbase);  }
        zgaz3314 = zgsz374;
      }
      unit zgsz375;
      zgsz375 = sail_assert(zgaz3314, "src/cheri_cap_common.sail:453.40-453.41");
    }
    sail_int zgaz3315;
    CREATE(sail_int)(&zgaz3315);
    sub_int(&zgaz3315, ztop, zbase);
    sail_int zgaz3316;
    CREATE(sail_int)(&zgaz3316);
    {
      sail_int zgsz3658;
      CREATE(sail_int)(&zgsz3658);
      CONVERT_OF(sail_int, mach_int)(&zgsz3658, zcap_len_width);
      pow2(&zgaz3316, zgsz3658);
      KILL(sail_int)(&zgsz3658);
    }
    emod_int(&zgsz373, zgaz3315, zgaz3316);
    KILL(sail_int)(&zgaz3316);
    KILL(sail_int)(&zgaz3315);
    KILL(sail_int)(&ztop);
    KILL(sail_int)(&zbase);
    goto finish_match_139;
  }
case_140: ;
  sail_match_failure("getCapLength");
finish_match_139: ;
  COPY(sail_int)((*(&zcbz333)), zgsz373);
  KILL(sail_int)(&zgsz373);
  KILL(ztuple_z8z5izCz0z5iz9)(&zgaz3311);
end_function_141: ;
  goto end_function_196;
end_block_exception_142: ;
  goto end_function_196;
end_function_196: ;
}

static bool sailgen_inCapBounds(struct zCapability, uint64_t, sail_int);

static bool sailgen_inCapBounds(struct zCapability zcap, uint64_t zaddr, sail_int zsizze)
{
  __label__ case_145, finish_match_144, end_function_146, end_block_exception_147;

  bool zcbz334;
  struct ztuple_z8z5izCz0z5iz9 zgaz3317;
  CREATE(ztuple_z8z5izCz0z5iz9)(&zgaz3317);
  sailgen_getCapBounds(&zgaz3317, zcap);
  bool zgsz377;
  {
    sail_int zbase;
    CREATE(sail_int)(&zbase);
    COPY(sail_int)(&zbase, zgaz3317.ztup0);
    sail_int ztop;
    CREATE(sail_int)(&ztop);
    COPY(sail_int)(&ztop, zgaz3317.ztup1);
    sail_int za;
    CREATE(sail_int)(&za);
    {
      lbits zgsz3659;
      CREATE(lbits)(&zgsz3659);
      CONVERT_OF(lbits, fbits)(&zgsz3659, zaddr, UINT64_C(64) , true);
      sail_unsigned(&za, zgsz3659);
      KILL(lbits)(&zgsz3659);
    }
    bool zgaz3319;
    zgaz3319 = gteq(za, zbase);
    bool zgsz378;
    if (zgaz3319) {
      sail_int zgaz3318;
      CREATE(sail_int)(&zgaz3318);
      add_int(&zgaz3318, za, zsizze);
      zgsz378 = lteq(zgaz3318, ztop);
      KILL(sail_int)(&zgaz3318);
    } else {  zgsz378 = false;  }
    zgsz377 = zgsz378;
    KILL(sail_int)(&za);
    KILL(sail_int)(&ztop);
    KILL(sail_int)(&zbase);
    goto finish_match_144;
  }
case_145: ;
  sail_match_failure("inCapBounds");
finish_match_144: ;
  zcbz334 = zgsz377;

  KILL(ztuple_z8z5izCz0z5iz9)(&zgaz3317);
end_function_146: ;
  return zcbz334;
end_block_exception_147: ;

  return false;
}

static struct zCapability sailgen_clearTagIf(struct zCapability, bool);

static struct zCapability sailgen_clearTagIf(struct zCapability zcap, bool zcond)
{
  __label__ end_function_149, end_block_exception_150;

  struct zCapability zcbz335;
  bool zgaz3321;
  {
    bool zgaz3320;
    zgaz3320 = zcap.ztag;
    bool zgsz380;
    if (zgaz3320) {  zgsz380 = not(zcond);  } else {  zgsz380 = false;  }
    zgaz3321 = zgsz380;
  }
  struct zCapability zgsz381;
  zgsz381 = zcap;
  zgsz381.ztag = zgaz3321;
  zcbz335 = zgsz381;


end_function_149: ;
  return zcbz335;
end_block_exception_150: ;
  struct zCapability zcbz355 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz355;
}

static struct zCapability sailgen_clearTagIfSealed(struct zCapability);

static struct zCapability sailgen_clearTagIfSealed(struct zCapability zcap)
{
  __label__ end_function_152, end_block_exception_153;

  struct zCapability zcbz336;
  bool zgaz3322;
  zgaz3322 = sailgen_isCapSealed(zcap);
  zcbz336 = sailgen_clearTagIf(zcap, zgaz3322);

end_function_152: ;
  return zcbz336;
end_block_exception_153: ;
  struct zCapability zcbz356 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz356;
}

static bool sailgen_capBoundsEqual(struct zCapability, struct zCapability);

static bool sailgen_capBoundsEqual(struct zCapability zc1, struct zCapability zc2)
{
  __label__ case_156, finish_match_155, end_function_159, end_block_exception_160;

  bool zcbz337;
  struct ztuple_z8z5izCz0z5iz9 zgaz3323;
  CREATE(ztuple_z8z5izCz0z5iz9)(&zgaz3323);
  sailgen_getCapBounds(&zgaz3323, zc1);
  bool zgsz382;
  {
    __label__ case_158, finish_match_157;

    sail_int zbase1;
    CREATE(sail_int)(&zbase1);
    COPY(sail_int)(&zbase1, zgaz3323.ztup0);
    sail_int ztop1;
    CREATE(sail_int)(&ztop1);
    COPY(sail_int)(&ztop1, zgaz3323.ztup1);
    struct ztuple_z8z5izCz0z5iz9 zgaz3324;
    CREATE(ztuple_z8z5izCz0z5iz9)(&zgaz3324);
    sailgen_getCapBounds(&zgaz3324, zc2);
    bool zgsz383;
    {
      sail_int zbase2;
      CREATE(sail_int)(&zbase2);
      COPY(sail_int)(&zbase2, zgaz3324.ztup0);
      sail_int ztop2;
      CREATE(sail_int)(&ztop2);
      COPY(sail_int)(&ztop2, zgaz3324.ztup1);
      bool zgaz3325;
      zgaz3325 = eq_int(zbase1, zbase2);
      bool zgsz384;
      if (zgaz3325) {  zgsz384 = eq_int(ztop1, ztop2);  } else {  zgsz384 = false;  }
      zgsz383 = zgsz384;
      KILL(sail_int)(&ztop2);
      KILL(sail_int)(&zbase2);
      goto finish_match_157;
    }
  case_158: ;
    sail_match_failure("capBoundsEqual");
  finish_match_157: ;
    zgsz382 = zgsz383;
    KILL(ztuple_z8z5izCz0z5iz9)(&zgaz3324);
    KILL(sail_int)(&ztop1);
    KILL(sail_int)(&zbase1);
    goto finish_match_155;
  }
case_156: ;
  sail_match_failure("capBoundsEqual");
finish_match_155: ;
  zcbz337 = zgsz382;

  KILL(ztuple_z8z5izCz0z5iz9)(&zgaz3323);
end_function_159: ;
  return zcbz337;
end_block_exception_160: ;

  return false;
}

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_setCapAddr(struct zCapability, uint64_t);

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_setCapAddr(struct zCapability zc, uint64_t zaddr)
{
  __label__ end_function_162, end_block_exception_163;

  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz338;
  struct zCapability znewCap;
  {
    struct zCapability zgsz387;
    zgsz387 = zc;
    zgsz387.zaddress = zaddr;
    znewCap = zgsz387;
  }
  bool zrepresentable;
  zrepresentable = sailgen_capBoundsEqual(zc, znewCap);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz388;
  zgsz388.ztup0 = zrepresentable;
  zgsz388.ztup1 = znewCap;
  zcbz338 = zgsz388;



end_function_162: ;
  return zcbz338;
end_block_exception_163: ;
  struct zCapability zcbz358 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz357 = { .ztup1 = zcbz358, .ztup0 = false };
  return zcbz357;
}

static bool sailgen_fastRepCheck(struct zCapability, uint64_t);

static bool sailgen_fastRepCheck(struct zCapability zc, uint64_t zi)
{
  __label__ end_function_165, end_block_exception_166;

  bool zcbz339;
  int64_t zE;
  {
    uint64_t zgaz3339;
    zgaz3339 = zc.zE;
    {
      lbits zgsz3660;
      CREATE(lbits)(&zgsz3660);
      CONVERT_OF(lbits, fbits)(&zgsz3660, zgaz3339, UINT64_C(6) , true);
      sail_int zgsz3661;
      CREATE(sail_int)(&zgsz3661);
      sail_unsigned(&zgsz3661, zgsz3660);
      zE = CONVERT_OF(mach_int, sail_int)(zgsz3661);
      KILL(sail_int)(&zgsz3661);
      KILL(lbits)(&zgsz3660);
    }
  }
  bool zgaz3327;
  {
    int64_t zgaz3326;
    {
      sail_int zgsz3664;
      CREATE(sail_int)(&zgsz3664);
      CONVERT_OF(sail_int, mach_int)(&zgsz3664, zcap_max_E);
      sail_int zgsz3665;
      CREATE(sail_int)(&zgsz3665);
      CONVERT_OF(sail_int, mach_int)(&zgsz3665, INT64_C(2));
      sail_int zgsz3666;
      CREATE(sail_int)(&zgsz3666);
      sub_int(&zgsz3666, zgsz3664, zgsz3665);
      zgaz3326 = CONVERT_OF(mach_int, sail_int)(zgsz3666);
      KILL(sail_int)(&zgsz3666);
      KILL(sail_int)(&zgsz3665);
      KILL(sail_int)(&zgsz3664);
    }
    {
      sail_int zgsz3662;
      CREATE(sail_int)(&zgsz3662);
      CONVERT_OF(sail_int, mach_int)(&zgsz3662, zE);
      sail_int zgsz3663;
      CREATE(sail_int)(&zgsz3663);
      CONVERT_OF(sail_int, mach_int)(&zgsz3663, zgaz3326);
      zgaz3327 = gteq(zgsz3662, zgsz3663);
      KILL(sail_int)(&zgsz3663);
      KILL(sail_int)(&zgsz3662);
    }
  }
  if (zgaz3327) {  zcbz339 = true;  } else {
    int64_t zi_top;
    {
      uint64_t zgaz3338;
      {
        int64_t zgaz3337;
        {
          sail_int zgsz3670;
          CREATE(sail_int)(&zgsz3670);
          CONVERT_OF(sail_int, mach_int)(&zgsz3670, zE);
          sail_int zgsz3671;
          CREATE(sail_int)(&zgsz3671);
          CONVERT_OF(sail_int, mach_int)(&zgsz3671, zcap_mantissa_width);
          sail_int zgsz3672;
          CREATE(sail_int)(&zgsz3672);
          add_int(&zgsz3672, zgsz3670, zgsz3671);
          zgaz3337 = CONVERT_OF(mach_int, sail_int)(zgsz3672);
          KILL(sail_int)(&zgsz3672);
          KILL(sail_int)(&zgsz3671);
          KILL(sail_int)(&zgsz3670);
        }
        {
          lbits zgsz3667;
          CREATE(lbits)(&zgsz3667);
          CONVERT_OF(lbits, fbits)(&zgsz3667, zi, UINT64_C(64) , true);
          sail_int zgsz3668;
          CREATE(sail_int)(&zgsz3668);
          CONVERT_OF(sail_int, mach_int)(&zgsz3668, zgaz3337);
          lbits zgsz3669;
          CREATE(lbits)(&zgsz3669);
          arith_shiftr(&zgsz3669, zgsz3667, zgsz3668);
          zgaz3338 = CONVERT_OF(fbits, lbits)(zgsz3669, true);
          KILL(lbits)(&zgsz3669);
          KILL(sail_int)(&zgsz3668);
          KILL(lbits)(&zgsz3667);
        }
      }
      {
        lbits zgsz3673;
        CREATE(lbits)(&zgsz3673);
        CONVERT_OF(lbits, fbits)(&zgsz3673, zgaz3338, UINT64_C(64) , true);
        sail_int zgsz3674;
        CREATE(sail_int)(&zgsz3674);
        sail_signed(&zgsz3674, zgsz3673);
        zi_top = CONVERT_OF(mach_int, sail_int)(zgsz3674);
        KILL(sail_int)(&zgsz3674);
        KILL(lbits)(&zgsz3673);
      }
    }
    uint64_t zi_mid;
    {
      uint64_t zgaz3336;
      {
        lbits zgsz3678;
        CREATE(lbits)(&zgsz3678);
        CONVERT_OF(lbits, fbits)(&zgsz3678, zi, UINT64_C(64) , true);
        sail_int zgsz3679;
        CREATE(sail_int)(&zgsz3679);
        CONVERT_OF(sail_int, mach_int)(&zgsz3679, zE);
        lbits zgsz3680;
        CREATE(lbits)(&zgsz3680);
        shiftr(&zgsz3680, zgsz3678, zgsz3679);
        zgaz3336 = CONVERT_OF(fbits, lbits)(zgsz3680, true);
        KILL(lbits)(&zgsz3680);
        KILL(sail_int)(&zgsz3679);
        KILL(lbits)(&zgsz3678);
      }
      {
        lbits zgsz3675;
        CREATE(lbits)(&zgsz3675);
        CONVERT_OF(lbits, fbits)(&zgsz3675, zgaz3336, UINT64_C(64) , true);
        sail_int zgsz3676;
        CREATE(sail_int)(&zgsz3676);
        CONVERT_OF(sail_int, mach_int)(&zgsz3676, zcap_mantissa_width);
        lbits zgsz3677;
        CREATE(lbits)(&zgsz3677);
        sail_truncate(&zgsz3677, zgsz3675, zgsz3676);
        zi_mid = CONVERT_OF(fbits, lbits)(zgsz3677, true);
        KILL(lbits)(&zgsz3677);
        KILL(sail_int)(&zgsz3676);
        KILL(lbits)(&zgsz3675);
      }
    }
    uint64_t za_mid;
    {
      uint64_t zgaz3335;
      {
        uint64_t zgaz3334;
        zgaz3334 = zc.zaddress;
        {
          lbits zgsz3681;
          CREATE(lbits)(&zgsz3681);
          CONVERT_OF(lbits, fbits)(&zgsz3681, zgaz3334, UINT64_C(64) , true);
          sail_int zgsz3682;
          CREATE(sail_int)(&zgsz3682);
          CONVERT_OF(sail_int, mach_int)(&zgsz3682, zE);
          lbits zgsz3683;
          CREATE(lbits)(&zgsz3683);
          shiftr(&zgsz3683, zgsz3681, zgsz3682);
          zgaz3335 = CONVERT_OF(fbits, lbits)(zgsz3683, true);
          KILL(lbits)(&zgsz3683);
          KILL(sail_int)(&zgsz3682);
          KILL(lbits)(&zgsz3681);
        }
      }
      {
        lbits zgsz3684;
        CREATE(lbits)(&zgsz3684);
        CONVERT_OF(lbits, fbits)(&zgsz3684, zgaz3335, UINT64_C(64) , true);
        sail_int zgsz3685;
        CREATE(sail_int)(&zgsz3685);
        CONVERT_OF(sail_int, mach_int)(&zgsz3685, zcap_mantissa_width);
        lbits zgsz3686;
        CREATE(lbits)(&zgsz3686);
        sail_truncate(&zgsz3686, zgsz3684, zgsz3685);
        za_mid = CONVERT_OF(fbits, lbits)(zgsz3686, true);
        KILL(lbits)(&zgsz3686);
        KILL(sail_int)(&zgsz3685);
        KILL(lbits)(&zgsz3684);
      }
    }
    uint64_t zB3;
    {
      uint64_t zgaz3333;
      zgaz3333 = zc.zB;
      {
        lbits zgsz3687;
        CREATE(lbits)(&zgsz3687);
        CONVERT_OF(lbits, fbits)(&zgsz3687, zgaz3333, UINT64_C(14) , true);
        sail_int zgsz3688;
        CREATE(sail_int)(&zgsz3688);
        CONVERT_OF(sail_int, mach_int)(&zgsz3688, INT64_C(3));
        lbits zgsz3689;
        CREATE(lbits)(&zgsz3689);
        sail_truncateLSB(&zgsz3689, zgsz3687, zgsz3688);
        zB3 = CONVERT_OF(fbits, lbits)(zgsz3689, true);
        KILL(lbits)(&zgsz3689);
        KILL(sail_int)(&zgsz3688);
        KILL(lbits)(&zgsz3687);
      }
    }
    uint64_t zR3;
    {
      lbits zgsz3714;
      CREATE(lbits)(&zgsz3714);
      CONVERT_OF(lbits, fbits)(&zgsz3714, zB3, UINT64_C(3) , true);
      lbits zgsz3715;
      CREATE(lbits)(&zgsz3715);
      CONVERT_OF(lbits, fbits)(&zgsz3715, UINT64_C(0b001), UINT64_C(3) , true);
      lbits zgsz3716;
      CREATE(lbits)(&zgsz3716);
      sub_bits(&zgsz3716, zgsz3714, zgsz3715);
      zR3 = CONVERT_OF(fbits, lbits)(zgsz3716, true);
      KILL(lbits)(&zgsz3716);
      KILL(lbits)(&zgsz3715);
      KILL(lbits)(&zgsz3714);
    }
    uint64_t zR;
    {
      uint64_t zgaz3332;
      {
        int64_t zgaz3331;
        {
          sail_int zgsz3692;
          CREATE(sail_int)(&zgsz3692);
          CONVERT_OF(sail_int, mach_int)(&zgsz3692, zcap_mantissa_width);
          sail_int zgsz3693;
          CREATE(sail_int)(&zgsz3693);
          CONVERT_OF(sail_int, mach_int)(&zgsz3693, INT64_C(3));
          sail_int zgsz3694;
          CREATE(sail_int)(&zgsz3694);
          sub_int(&zgsz3694, zgsz3692, zgsz3693);
          zgaz3331 = CONVERT_OF(mach_int, sail_int)(zgsz3694);
          KILL(sail_int)(&zgsz3694);
          KILL(sail_int)(&zgsz3693);
          KILL(sail_int)(&zgsz3692);
        }
        {
          sail_int zgsz3690;
          CREATE(sail_int)(&zgsz3690);
          CONVERT_OF(sail_int, mach_int)(&zgsz3690, zgaz3331);
          lbits zgsz3691;
          CREATE(lbits)(&zgsz3691);
          sailgen_zzeros_implicit(&zgsz3691, zgsz3690);
          zgaz3332 = CONVERT_OF(fbits, lbits)(zgsz3691, true);
          KILL(lbits)(&zgsz3691);
          KILL(sail_int)(&zgsz3690);
        }
      }
      {
        lbits zgsz3695;
        CREATE(lbits)(&zgsz3695);
        CONVERT_OF(lbits, fbits)(&zgsz3695, zR3, UINT64_C(3) , true);
        lbits zgsz3696;
        CREATE(lbits)(&zgsz3696);
        CONVERT_OF(lbits, fbits)(&zgsz3696, zgaz3332, UINT64_C(11) , true);
        lbits zgsz3697;
        CREATE(lbits)(&zgsz3697);
        append(&zgsz3697, zgsz3695, zgsz3696);
        zR = CONVERT_OF(fbits, lbits)(zgsz3697, true);
        KILL(lbits)(&zgsz3697);
        KILL(lbits)(&zgsz3696);
        KILL(lbits)(&zgsz3695);
      }
    }
    uint64_t zdiff;
    {
      lbits zgsz3711;
      CREATE(lbits)(&zgsz3711);
      CONVERT_OF(lbits, fbits)(&zgsz3711, zR, UINT64_C(14) , true);
      lbits zgsz3712;
      CREATE(lbits)(&zgsz3712);
      CONVERT_OF(lbits, fbits)(&zgsz3712, za_mid, UINT64_C(14) , true);
      lbits zgsz3713;
      CREATE(lbits)(&zgsz3713);
      sub_bits(&zgsz3713, zgsz3711, zgsz3712);
      zdiff = CONVERT_OF(fbits, lbits)(zgsz3713, true);
      KILL(lbits)(&zgsz3713);
      KILL(lbits)(&zgsz3712);
      KILL(lbits)(&zgsz3711);
    }
    uint64_t zdiff1;
    {
      lbits zgsz3708;
      CREATE(lbits)(&zgsz3708);
      CONVERT_OF(lbits, fbits)(&zgsz3708, zdiff, UINT64_C(14) , true);
      sail_int zgsz3709;
      CREATE(sail_int)(&zgsz3709);
      CONVERT_OF(sail_int, mach_int)(&zgsz3709, INT64_C(1));
      lbits zgsz3710;
      CREATE(lbits)(&zgsz3710);
      sub_bits_int(&zgsz3710, zgsz3708, zgsz3709);
      zdiff1 = CONVERT_OF(fbits, lbits)(zgsz3710, true);
      KILL(lbits)(&zgsz3710);
      KILL(sail_int)(&zgsz3709);
      KILL(lbits)(&zgsz3708);
    }
    bool zgaz3328;
    {
      sail_int zgsz3706;
      CREATE(sail_int)(&zgsz3706);
      CONVERT_OF(sail_int, mach_int)(&zgsz3706, zi_top);
      sail_int zgsz3707;
      CREATE(sail_int)(&zgsz3707);
      CONVERT_OF(sail_int, mach_int)(&zgsz3707, INT64_C(0));
      zgaz3328 = eq_int(zgsz3706, zgsz3707);
      KILL(sail_int)(&zgsz3707);
      KILL(sail_int)(&zgsz3706);
    }
    if (zgaz3328) {
      {
        lbits zgsz3704;
        CREATE(lbits)(&zgsz3704);
        CONVERT_OF(lbits, fbits)(&zgsz3704, zi_mid, UINT64_C(14) , true);
        lbits zgsz3705;
        CREATE(lbits)(&zgsz3705);
        CONVERT_OF(lbits, fbits)(&zgsz3705, zdiff1, UINT64_C(14) , true);
        zcbz339 = sailgen_z8operatorz0zI_uz9(zgsz3704, zgsz3705);
        KILL(lbits)(&zgsz3705);
        KILL(lbits)(&zgsz3704);
      }
    } else {
      bool zgaz3329;
      {
        sail_int zgsz3702;
        CREATE(sail_int)(&zgsz3702);
        CONVERT_OF(sail_int, mach_int)(&zgsz3702, zi_top);
        sail_int zgsz3703;
        CREATE(sail_int)(&zgsz3703);
        CONVERT_OF(sail_int, mach_int)(&zgsz3703, INT64_C(-1));
        zgaz3329 = eq_int(zgsz3702, zgsz3703);
        KILL(sail_int)(&zgsz3703);
        KILL(sail_int)(&zgsz3702);
      }
      if (zgaz3329) {
        bool zgaz3330;
        {
          lbits zgsz3700;
          CREATE(lbits)(&zgsz3700);
          CONVERT_OF(lbits, fbits)(&zgsz3700, zi_mid, UINT64_C(14) , true);
          lbits zgsz3701;
          CREATE(lbits)(&zgsz3701);
          CONVERT_OF(lbits, fbits)(&zgsz3701, zdiff, UINT64_C(14) , true);
          zgaz3330 = sailgen_z8operatorz0zKzJ_uz9(zgsz3700, zgsz3701);
          KILL(lbits)(&zgsz3701);
          KILL(lbits)(&zgsz3700);
        }
        bool zgsz389;
        if (zgaz3330) {
          {
            lbits zgsz3698;
            CREATE(lbits)(&zgsz3698);
            CONVERT_OF(lbits, fbits)(&zgsz3698, zR, UINT64_C(14) , true);
            lbits zgsz3699;
            CREATE(lbits)(&zgsz3699);
            CONVERT_OF(lbits, fbits)(&zgsz3699, za_mid, UINT64_C(14) , true);
            zgsz389 = neq_bits(zgsz3698, zgsz3699);
            KILL(lbits)(&zgsz3699);
            KILL(lbits)(&zgsz3698);
          }
        } else {  zgsz389 = false;  }
        zcbz339 = zgsz389;
      } else {  zcbz339 = false;  }
    }
  }


end_function_165: ;
  return zcbz339;
end_block_exception_166: ;

  return false;
}

static void sailgen_capToString(sail_string *rop, struct zCapability);

static void sailgen_capToString(sail_string *zcbz340, struct zCapability zcap)
{
  __label__ end_function_168, end_block_exception_169, end_function_195;

  sail_int zlen;
  CREATE(sail_int)(&zlen);
  sailgen_getCapLength(&zlen, zcap);
  sail_string zlen_str;
  CREATE(sail_string)(&zlen_str);
  {
    lbits zgaz3368;
    CREATE(lbits)(&zgaz3368);
    {
      int64_t zgaz3367;
      {
        sail_int zgsz3718;
        CREATE(sail_int)(&zgsz3718);
        CONVERT_OF(sail_int, mach_int)(&zgsz3718, zcap_len_width);
        sail_int zgsz3719;
        CREATE(sail_int)(&zgsz3719);
        CONVERT_OF(sail_int, mach_int)(&zgsz3719, INT64_C(3));
        sail_int zgsz3720;
        CREATE(sail_int)(&zgsz3720);
        add_int(&zgsz3720, zgsz3718, zgsz3719);
        zgaz3367 = CONVERT_OF(mach_int, sail_int)(zgsz3720);
        KILL(sail_int)(&zgsz3720);
        KILL(sail_int)(&zgsz3719);
        KILL(sail_int)(&zgsz3718);
      }
      {
        sail_int zgsz3717;
        CREATE(sail_int)(&zgsz3717);
        CONVERT_OF(sail_int, mach_int)(&zgsz3717, zgaz3367);
        sailgen_to_bits(&zgaz3368, zgsz3717, zlen);
        KILL(sail_int)(&zgsz3717);
      }
    }
    string_of_lbits(&zlen_str, zgaz3368);
    KILL(lbits)(&zgaz3368);
  }
  uint64_t zotype64;
  {
    bool zgaz3364;
    zgaz3364 = sailgen_hasReservedOType(zcap);
    if (zgaz3364) {
      uint64_t zgaz3365;
      zgaz3365 = zcap.zotype;
      {
        sail_int zgsz3724;
        CREATE(sail_int)(&zgsz3724);
        CONVERT_OF(sail_int, mach_int)(&zgsz3724, INT64_C(64));
        lbits zgsz3725;
        CREATE(lbits)(&zgsz3725);
        CONVERT_OF(lbits, fbits)(&zgsz3725, zgaz3365, UINT64_C(18) , true);
        lbits zgsz3726;
        CREATE(lbits)(&zgsz3726);
        sailgen_EXTS(&zgsz3726, zgsz3724, zgsz3725);
        zotype64 = CONVERT_OF(fbits, lbits)(zgsz3726, true);
        KILL(lbits)(&zgsz3726);
        KILL(lbits)(&zgsz3725);
        KILL(sail_int)(&zgsz3724);
      }
    } else {
      uint64_t zgaz3366;
      zgaz3366 = zcap.zotype;
      {
        sail_int zgsz3721;
        CREATE(sail_int)(&zgsz3721);
        CONVERT_OF(sail_int, mach_int)(&zgsz3721, INT64_C(64));
        lbits zgsz3722;
        CREATE(lbits)(&zgsz3722);
        CONVERT_OF(lbits, fbits)(&zgsz3722, zgaz3366, UINT64_C(18) , true);
        lbits zgsz3723;
        CREATE(lbits)(&zgsz3723);
        sailgen_EXTZ(&zgsz3723, zgsz3721, zgsz3722);
        zotype64 = CONVERT_OF(fbits, lbits)(zgsz3723, true);
        KILL(lbits)(&zgsz3723);
        KILL(lbits)(&zgsz3722);
        KILL(sail_int)(&zgsz3721);
      }
    }
  }
  sail_string zgaz3363;
  CREATE(sail_string)(&zgaz3363);
  {
    sail_string zgaz3361;
    CREATE(sail_string)(&zgaz3361);
    {
      bool zgaz3340;
      zgaz3340 = zcap.ztag;
      if (zgaz3340) {  COPY(sail_string)(&zgaz3361, "1");  } else {  COPY(sail_string)(&zgaz3361, "0");  }
    }
    sail_string zgaz3362;
    CREATE(sail_string)(&zgaz3362);
    {
      sail_string zgaz3360;
      CREATE(sail_string)(&zgaz3360);
      {
        sail_string zgaz3358;
        CREATE(sail_string)(&zgaz3358);
        {
          bool zgaz3341;
          zgaz3341 = sailgen_isCapSealed(zcap);
          if (zgaz3341) {  COPY(sail_string)(&zgaz3358, "1");  } else {  COPY(sail_string)(&zgaz3358, "0");  }
        }
        sail_string zgaz3359;
        CREATE(sail_string)(&zgaz3359);
        {
          sail_string zgaz3357;
          CREATE(sail_string)(&zgaz3357);
          {
            sail_string zgaz3355;
            CREATE(sail_string)(&zgaz3355);
            {
              uint64_t zgaz3343;
              {
                uint64_t zgaz3342;
                zgaz3342 = sailgen_getCapPerms(zcap);
                {
                  lbits zgsz3727;
                  CREATE(lbits)(&zgsz3727);
                  CONVERT_OF(lbits, fbits)(&zgsz3727, UINT64_C(0b0), UINT64_C(1) , true);
                  lbits zgsz3728;
                  CREATE(lbits)(&zgsz3728);
                  CONVERT_OF(lbits, fbits)(&zgsz3728, zgaz3342, UINT64_C(19) , true);
                  lbits zgsz3729;
                  CREATE(lbits)(&zgsz3729);
                  append(&zgsz3729, zgsz3727, zgsz3728);
                  zgaz3343 = CONVERT_OF(fbits, lbits)(zgsz3729, true);
                  KILL(lbits)(&zgsz3729);
                  KILL(lbits)(&zgsz3728);
                  KILL(lbits)(&zgsz3727);
                }
              }
              {
                lbits zgsz3730;
                CREATE(lbits)(&zgsz3730);
                CONVERT_OF(lbits, fbits)(&zgsz3730, zgaz3343, UINT64_C(20) , true);
                string_of_lbits(&zgaz3355, zgsz3730);
                KILL(lbits)(&zgsz3730);
              }
            }
            sail_string zgaz3356;
            CREATE(sail_string)(&zgaz3356);
            {
              sail_string zgaz3354;
              CREATE(sail_string)(&zgaz3354);
              {
                sail_string zgaz3352;
                CREATE(sail_string)(&zgaz3352);
                {
                  lbits zgsz3733;
                  CREATE(lbits)(&zgsz3733);
                  CONVERT_OF(lbits, fbits)(&zgsz3733, zotype64, UINT64_C(64) , true);
                  string_of_lbits(&zgaz3352, zgsz3733);
                  KILL(lbits)(&zgsz3733);
                }
                sail_string zgaz3353;
                CREATE(sail_string)(&zgaz3353);
                {
                  sail_string zgaz3351;
                  CREATE(sail_string)(&zgaz3351);
                  {
                    sail_string zgaz3349;
                    CREATE(sail_string)(&zgaz3349);
                    {
                      uint64_t zgaz3344;
                      zgaz3344 = zcap.zaddress;
                      {
                        lbits zgsz3731;
                        CREATE(lbits)(&zgsz3731);
                        CONVERT_OF(lbits, fbits)(&zgsz3731, zgaz3344, UINT64_C(64) , true);
                        string_of_lbits(&zgaz3349, zgsz3731);
                        KILL(lbits)(&zgsz3731);
                      }
                    }
                    sail_string zgaz3350;
                    CREATE(sail_string)(&zgaz3350);
                    {
                      sail_string zgaz3348;
                      CREATE(sail_string)(&zgaz3348);
                      {
                        sail_string zgaz3346;
                        CREATE(sail_string)(&zgaz3346);
                        {
                          uint64_t zgaz3345;
                          zgaz3345 = sailgen_getCapBaseBits(zcap);
                          {
                            lbits zgsz3732;
                            CREATE(lbits)(&zgsz3732);
                            CONVERT_OF(lbits, fbits)(&zgsz3732, zgaz3345, UINT64_C(64) , true);
                            string_of_lbits(&zgaz3346, zgsz3732);
                            KILL(lbits)(&zgsz3732);
                          }
                        }
                        sail_string zgaz3347;
                        CREATE(sail_string)(&zgaz3347);
                        concat_str(&zgaz3347, " length:", zlen_str);
                        concat_str(&zgaz3348, zgaz3346, zgaz3347);
                        KILL(sail_string)(&zgaz3347);
                        KILL(sail_string)(&zgaz3346);
                      }
                      concat_str(&zgaz3350, " base:", zgaz3348);
                      KILL(sail_string)(&zgaz3348);
                    }
                    concat_str(&zgaz3351, zgaz3349, zgaz3350);
                    KILL(sail_string)(&zgaz3350);
                    KILL(sail_string)(&zgaz3349);
                  }
                  concat_str(&zgaz3353, " address:", zgaz3351);
                  KILL(sail_string)(&zgaz3351);
                }
                concat_str(&zgaz3354, zgaz3352, zgaz3353);
                KILL(sail_string)(&zgaz3353);
                KILL(sail_string)(&zgaz3352);
              }
              concat_str(&zgaz3356, " type:", zgaz3354);
              KILL(sail_string)(&zgaz3354);
            }
            concat_str(&zgaz3357, zgaz3355, zgaz3356);
            KILL(sail_string)(&zgaz3356);
            KILL(sail_string)(&zgaz3355);
          }
          concat_str(&zgaz3359, " perms:", zgaz3357);
          KILL(sail_string)(&zgaz3357);
        }
        concat_str(&zgaz3360, zgaz3358, zgaz3359);
        KILL(sail_string)(&zgaz3359);
        KILL(sail_string)(&zgaz3358);
      }
      concat_str(&zgaz3362, " s:", zgaz3360);
      KILL(sail_string)(&zgaz3360);
    }
    concat_str(&zgaz3363, zgaz3361, zgaz3362);
    KILL(sail_string)(&zgaz3362);
    KILL(sail_string)(&zgaz3361);
  }
  concat_str((*(&zcbz340)), " t:", zgaz3363);
  KILL(sail_string)(&zgaz3363);

  KILL(sail_string)(&zlen_str);
  KILL(sail_int)(&zlen);
end_function_168: ;
  goto end_function_195;
end_block_exception_169: ;
  goto end_function_195;
end_function_195: ;
}

static uint64_t sailgen_getRepresentableAlignmentMask(uint64_t);

static uint64_t sailgen_getRepresentableAlignmentMask(uint64_t zlen)
{
  __label__ case_172, finish_match_171, end_function_173, end_block_exception_174;

  uint64_t zcbz341;
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3371;
  {
    uint64_t zgaz3369;
    {
      sail_int zgsz3736;
      CREATE(sail_int)(&zgsz3736);
      CONVERT_OF(sail_int, mach_int)(&zgsz3736, INT64_C(64));
      sail_int zgsz3737;
      CREATE(sail_int)(&zgsz3737);
      CONVERT_OF(sail_int, mach_int)(&zgsz3737, INT64_C(0));
      lbits zgsz3738;
      CREATE(lbits)(&zgsz3738);
      sailgen_to_bits(&zgsz3738, zgsz3736, zgsz3737);
      zgaz3369 = CONVERT_OF(fbits, lbits)(zgsz3738, true);
      KILL(lbits)(&zgsz3738);
      KILL(sail_int)(&zgsz3737);
      KILL(sail_int)(&zgsz3736);
    }
    lbits zgaz3370;
    CREATE(lbits)(&zgaz3370);
    {
      lbits zgsz3734;
      CREATE(lbits)(&zgsz3734);
      CONVERT_OF(lbits, fbits)(&zgsz3734, UINT64_C(0b0), UINT64_C(1) , true);
      lbits zgsz3735;
      CREATE(lbits)(&zgsz3735);
      CONVERT_OF(lbits, fbits)(&zgsz3735, zlen, UINT64_C(64) , true);
      append(&zgaz3370, zgsz3734, zgsz3735);
      KILL(lbits)(&zgsz3735);
      KILL(lbits)(&zgsz3734);
    }
    zgaz3371 = sailgen_setCapBounds(zdefault_cap, zgaz3369, zgaz3370);
    KILL(lbits)(&zgaz3370);
  }
  uint64_t zgsz390;
  {
    bool zexact;
    zexact = zgaz3371.ztup0;
    struct zCapability zc;
    zc = zgaz3371.ztup1;
    int64_t ze;
    {
      int64_t zgaz3377;
      {
        uint64_t zgaz3376;
        zgaz3376 = zc.zE;
        {
          lbits zgsz3739;
          CREATE(lbits)(&zgsz3739);
          CONVERT_OF(lbits, fbits)(&zgsz3739, zgaz3376, UINT64_C(6) , true);
          sail_int zgsz3740;
          CREATE(sail_int)(&zgsz3740);
          sail_unsigned(&zgsz3740, zgsz3739);
          zgaz3377 = CONVERT_OF(mach_int, sail_int)(zgsz3740);
          KILL(sail_int)(&zgsz3740);
          KILL(lbits)(&zgsz3739);
        }
      }
      {
        sail_int zgsz3741;
        CREATE(sail_int)(&zgsz3741);
        CONVERT_OF(sail_int, mach_int)(&zgsz3741, zgaz3377);
        sail_int zgsz3742;
        CREATE(sail_int)(&zgsz3742);
        CONVERT_OF(sail_int, mach_int)(&zgsz3742, zcap_max_E);
        sail_int zgsz3743;
        CREATE(sail_int)(&zgsz3743);
        min_int(&zgsz3743, zgsz3741, zgsz3742);
        ze = CONVERT_OF(mach_int, sail_int)(zgsz3743);
        KILL(sail_int)(&zgsz3743);
        KILL(sail_int)(&zgsz3742);
        KILL(sail_int)(&zgsz3741);
      }
    }
    int64_t zez7;
    {
      bool zgaz3375;
      zgaz3375 = zc.zinternal_E;
      if (zgaz3375) {
        {
          sail_int zgsz3744;
          CREATE(sail_int)(&zgsz3744);
          CONVERT_OF(sail_int, mach_int)(&zgsz3744, ze);
          sail_int zgsz3745;
          CREATE(sail_int)(&zgsz3745);
          CONVERT_OF(sail_int, mach_int)(&zgsz3745, zinternal_E_take_bits);
          sail_int zgsz3746;
          CREATE(sail_int)(&zgsz3746);
          add_int(&zgsz3746, zgsz3744, zgsz3745);
          zez7 = CONVERT_OF(mach_int, sail_int)(zgsz3746);
          KILL(sail_int)(&zgsz3746);
          KILL(sail_int)(&zgsz3745);
          KILL(sail_int)(&zgsz3744);
        }
      } else {  zez7 = INT64_C(0);  }
    }
    sbits zgaz3373;
    {
      int64_t zgaz3372;
      {
        sail_int zgsz3749;
        CREATE(sail_int)(&zgsz3749);
        CONVERT_OF(sail_int, mach_int)(&zgsz3749, INT64_C(64));
        sail_int zgsz3750;
        CREATE(sail_int)(&zgsz3750);
        CONVERT_OF(sail_int, mach_int)(&zgsz3750, zez7);
        sail_int zgsz3751;
        CREATE(sail_int)(&zgsz3751);
        sub_int(&zgsz3751, zgsz3749, zgsz3750);
        zgaz3372 = CONVERT_OF(mach_int, sail_int)(zgsz3751);
        KILL(sail_int)(&zgsz3751);
        KILL(sail_int)(&zgsz3750);
        KILL(sail_int)(&zgsz3749);
      }
      {
        sail_int zgsz3747;
        CREATE(sail_int)(&zgsz3747);
        CONVERT_OF(sail_int, mach_int)(&zgsz3747, zgaz3372);
        lbits zgsz3748;
        CREATE(lbits)(&zgsz3748);
        sailgen_ones(&zgsz3748, zgsz3747);
        zgaz3373 = CONVERT_OF(sbits, lbits)(zgsz3748, true);
        KILL(lbits)(&zgsz3748);
        KILL(sail_int)(&zgsz3747);
      }
    }
    sbits zgaz3374;
    {
      sail_int zgsz3755;
      CREATE(sail_int)(&zgsz3755);
      CONVERT_OF(sail_int, mach_int)(&zgsz3755, zez7);
      lbits zgsz3756;
      CREATE(lbits)(&zgsz3756);
      sailgen_zzeros_implicit(&zgsz3756, zgsz3755);
      zgaz3374 = CONVERT_OF(sbits, lbits)(zgsz3756, true);
      KILL(lbits)(&zgsz3756);
      KILL(sail_int)(&zgsz3755);
    }
    {
      lbits zgsz3752;
      CREATE(lbits)(&zgsz3752);
      CONVERT_OF(lbits, sbits)(&zgsz3752, zgaz3373, true);
      lbits zgsz3753;
      CREATE(lbits)(&zgsz3753);
      CONVERT_OF(lbits, sbits)(&zgsz3753, zgaz3374, true);
      lbits zgsz3754;
      CREATE(lbits)(&zgsz3754);
      append(&zgsz3754, zgsz3752, zgsz3753);
      zgsz390 = CONVERT_OF(fbits, lbits)(zgsz3754, true);
      KILL(lbits)(&zgsz3754);
      KILL(lbits)(&zgsz3753);
      KILL(lbits)(&zgsz3752);
    }
    goto finish_match_171;
  }
case_172: ;
  sail_match_failure("getRepresentableAlignmentMask");
finish_match_171: ;
  zcbz341 = zgsz390;


end_function_173: ;
  return zcbz341;
end_block_exception_174: ;

  return UINT64_C(0xdeadc0de);
}

static uint64_t sailgen_getRepresentableLength(uint64_t);

static uint64_t sailgen_getRepresentableLength(uint64_t zlen)
{
  __label__ end_function_176, end_block_exception_177;

  uint64_t zcbz342;
  uint64_t zm;
  zm = sailgen_getRepresentableAlignmentMask(zlen);
  uint64_t zgaz3379;
  {
    uint64_t zgaz3378;
    {
      lbits zgsz3760;
      CREATE(lbits)(&zgsz3760);
      CONVERT_OF(lbits, fbits)(&zgsz3760, zm, UINT64_C(64) , true);
      lbits zgsz3761;
      CREATE(lbits)(&zgsz3761);
      not_bits(&zgsz3761, zgsz3760);
      zgaz3378 = CONVERT_OF(fbits, lbits)(zgsz3761, true);
      KILL(lbits)(&zgsz3761);
      KILL(lbits)(&zgsz3760);
    }
    {
      lbits zgsz3757;
      CREATE(lbits)(&zgsz3757);
      CONVERT_OF(lbits, fbits)(&zgsz3757, zlen, UINT64_C(64) , true);
      lbits zgsz3758;
      CREATE(lbits)(&zgsz3758);
      CONVERT_OF(lbits, fbits)(&zgsz3758, zgaz3378, UINT64_C(64) , true);
      lbits zgsz3759;
      CREATE(lbits)(&zgsz3759);
      add_bits(&zgsz3759, zgsz3757, zgsz3758);
      zgaz3379 = CONVERT_OF(fbits, lbits)(zgsz3759, true);
      KILL(lbits)(&zgsz3759);
      KILL(lbits)(&zgsz3758);
      KILL(lbits)(&zgsz3757);
    }
  }
  {
    lbits zgsz3762;
    CREATE(lbits)(&zgsz3762);
    CONVERT_OF(lbits, fbits)(&zgsz3762, zgaz3379, UINT64_C(64) , true);
    lbits zgsz3763;
    CREATE(lbits)(&zgsz3763);
    CONVERT_OF(lbits, fbits)(&zgsz3763, zm, UINT64_C(64) , true);
    lbits zgsz3764;
    CREATE(lbits)(&zgsz3764);
    and_bits(&zgsz3764, zgsz3762, zgsz3763);
    zcbz342 = CONVERT_OF(fbits, lbits)(zgsz3764, true);
    KILL(lbits)(&zgsz3764);
    KILL(lbits)(&zgsz3763);
    KILL(lbits)(&zgsz3762);
  }


end_function_176: ;
  return zcbz342;
end_block_exception_177: ;

  return UINT64_C(0xdeadc0de);
}

// register TestCap
static struct zCapability zTestCap;

// register TestAddr
static uint64_t zTestAddr;

// register TestLen
static lbits zTestLen;

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_doCSetBounds(struct zCapability, lbits);

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_doCSetBounds(struct zCapability zinput, lbits zreq_len)
{
  __label__ case_180, finish_match_179, end_function_181, end_block_exception_182;

  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz343;
  uint64_t znewBase;
  znewBase = zinput.zaddress;
  lbits znewTop;
  CREATE(lbits)(&znewTop);
  {
    lbits zgaz3385;
    CREATE(lbits)(&zgaz3385);
    {
      int64_t zgaz3384;
      {
        sail_int zgsz3767;
        CREATE(sail_int)(&zgsz3767);
        CONVERT_OF(sail_int, mach_int)(&zgsz3767, INT64_C(64));
        sail_int zgsz3768;
        CREATE(sail_int)(&zgsz3768);
        CONVERT_OF(sail_int, mach_int)(&zgsz3768, INT64_C(1));
        sail_int zgsz3769;
        CREATE(sail_int)(&zgsz3769);
        add_int(&zgsz3769, zgsz3767, zgsz3768);
        zgaz3384 = CONVERT_OF(mach_int, sail_int)(zgsz3769);
        KILL(sail_int)(&zgsz3769);
        KILL(sail_int)(&zgsz3768);
        KILL(sail_int)(&zgsz3767);
      }
      {
        sail_int zgsz3765;
        CREATE(sail_int)(&zgsz3765);
        CONVERT_OF(sail_int, mach_int)(&zgsz3765, zgaz3384);
        lbits zgsz3766;
        CREATE(lbits)(&zgsz3766);
        CONVERT_OF(lbits, fbits)(&zgsz3766, znewBase, UINT64_C(64) , true);
        sailgen_EXTZ(&zgaz3385, zgsz3765, zgsz3766);
        KILL(lbits)(&zgsz3766);
        KILL(sail_int)(&zgsz3765);
      }
    }
    add_bits(&znewTop, zgaz3385, zreq_len);
    KILL(lbits)(&zgaz3385);
  }
  bool zinBounds;
  {
    sail_int zgaz3383;
    CREATE(sail_int)(&zgaz3383);
    sail_unsigned(&zgaz3383, zreq_len);
    zinBounds = sailgen_inCapBounds(zinput, znewBase, zgaz3383);
    KILL(sail_int)(&zgaz3383);
  }
  struct zCapability zinCap;
  zinCap = sailgen_clearTagIfSealed(zinput);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3380;
  zgaz3380 = sailgen_setCapBounds(zinCap, znewBase, znewTop);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz392;
  {
    bool zexact;
    zexact = zgaz3380.ztup0;
    struct zCapability znewCap;
    znewCap = zgaz3380.ztup1;
    struct zCapability zgaz3382;
    {
      bool zgaz3381;
      zgaz3381 = not(zinBounds);
      zgaz3382 = sailgen_clearTagIf(znewCap, zgaz3381);
    }
    struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz393;
    zgsz393.ztup0 = zexact;
    zgsz393.ztup1 = zgaz3382;
    zgsz392 = zgsz393;
    goto finish_match_179;
  }
case_180: ;
  sail_match_failure("doCSetBounds");
finish_match_179: ;
  zcbz343 = zgsz392;




  KILL(lbits)(&znewTop);

end_function_181: ;
  return zcbz343;
end_block_exception_182: ;
  struct zCapability zcbz360 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz359 = { .ztup1 = zcbz360, .ztup0 = false };
  return zcbz359;
}

static unit sailgen_main(unit);


static unit sailgen_main(unit zgsz395)
{
  __label__ case_185, finish_match_184, end_function_190, end_block_exception_191;

  unit zcbz344;
  {
    zTestCap = znull_cap;
    unit zgsz396;
    zgsz396 = UNIT;
  }
  lbits zbits1;
  CREATE(lbits)(&zbits1);
  sailgen_capToBits(&zbits1, zTestCap);
  lbits zbits2;
  CREATE(lbits)(&zbits2);
  sailgen_capToMemBits(&zbits2, zTestCap);
  struct zCapability zcap1;
  {
    bool zgaz3392;
    zgaz3392 = zTestCap.ztag;
    zcap1 = sailgen_capBitsToCapability(zgaz3392, zbits1);
  }
  struct zCapability zcap2;
  {
    bool zgaz3391;
    zgaz3391 = zTestCap.ztag;
    zcap2 = sailgen_memBitsToCapability(zgaz3391, zbits2);
  }
  bool zrep;
  zrep = sailgen_fastRepCheck(zTestCap, zTestAddr);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3386;
  zgaz3386 = sailgen_doCSetBounds(zTestCap, zTestLen);
  unit zgsz397;
  {
    __label__ case_187, finish_match_186;

    bool zexact;
    zexact = zgaz3386.ztup0;
    struct zCapability zcap;
    zcap = zgaz3386.ztup1;
    struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3387;
    zgaz3387 = sailgen_setCapAddr(zTestCap, zTestAddr);
    unit zgsz398;
    {
      __label__ case_189, finish_match_188;

      bool zrepresentable;
      zrepresentable = zgaz3387.ztup0;
      struct zCapability zcapshadowz30;
      zcapshadowz30 = zgaz3387.ztup1;
      struct ztuple_z8z5izCz0z5iz9 zgaz3388;
      CREATE(ztuple_z8z5izCz0z5iz9)(&zgaz3388);
      sailgen_getCapBounds(&zgaz3388, zTestCap);
      unit zgsz399;
      {
        sail_int zbase;
        CREATE(sail_int)(&zbase);
        COPY(sail_int)(&zbase, zgaz3388.ztup0);
        sail_int ztop;
        CREATE(sail_int)(&ztop);
        COPY(sail_int)(&ztop, zgaz3388.ztup1);
        uint64_t zflags;
        zflags = sailgen_getCapFlags(zTestCap);
        struct zCapability znew_cap;
        {
          uint64_t zgaz3390;
          {
            int64_t zgaz3389;
            {
              sail_int zgsz3772;
              CREATE(sail_int)(&zgsz3772);
              CONVERT_OF(sail_int, mach_int)(&zgsz3772, INT64_C(15));
              sail_int zgsz3773;
              CREATE(sail_int)(&zgsz3773);
              CONVERT_OF(sail_int, mach_int)(&zgsz3773, INT64_C(4));
              sail_int zgsz3774;
              CREATE(sail_int)(&zgsz3774);
              add_int(&zgsz3774, zgsz3772, zgsz3773);
              zgaz3389 = CONVERT_OF(mach_int, sail_int)(zgsz3774);
              KILL(sail_int)(&zgsz3774);
              KILL(sail_int)(&zgsz3773);
              KILL(sail_int)(&zgsz3772);
            }
            {
              sail_int zgsz3770;
              CREATE(sail_int)(&zgsz3770);
              CONVERT_OF(sail_int, mach_int)(&zgsz3770, zgaz3389);
              lbits zgsz3771;
              CREATE(lbits)(&zgsz3771);
              sailgen_ones(&zgsz3771, zgsz3770);
              zgaz3390 = CONVERT_OF(fbits, lbits)(zgsz3771, true);
              KILL(lbits)(&zgsz3771);
              KILL(sail_int)(&zgsz3770);
            }
          }
          znew_cap = sailgen_setCapPerms(zTestCap, zgaz3390);
        }
        sail_string zcap_str;
        CREATE(sail_string)(&zcap_str);
        sailgen_capToString(&zcap_str, zTestCap);
        uint64_t zlen;
        zlen = sailgen_getRepresentableLength(zTestAddr);
        uint64_t zmask;
        zmask = sailgen_getRepresentableAlignmentMask(zTestAddr);
        zgsz399 = UNIT;
        KILL(sail_string)(&zcap_str);
        KILL(sail_int)(&ztop);
        KILL(sail_int)(&zbase);
        goto finish_match_188;
      }
    case_189: ;
      sail_match_failure("main");
    finish_match_188: ;
      zgsz398 = zgsz399;
      KILL(ztuple_z8z5izCz0z5iz9)(&zgaz3388);
      goto finish_match_186;
    }
  case_187: ;
    sail_match_failure("main");
  finish_match_186: ;
    zgsz397 = zgsz398;
    goto finish_match_184;
  }
case_185: ;
  sail_match_failure("main");
finish_match_184: ;
  zcbz344 = zgsz397;





  KILL(lbits)(&zbits2);
  KILL(lbits)(&zbits1);
end_function_190: ;
  return zcbz344;
end_block_exception_191: ;

  return UNIT;
}

static unit sailgen_initializze_registers(unit);

static unit sailgen_initializze_registers(unit zgsz3103)
{
  __label__ end_function_193, end_block_exception_194;

  unit zcbz345;
  {
    zTestCap = sailgen_undefined_Capability(UNIT);
    unit zgsz3105;
    zgsz3105 = UNIT;
  }
  {
    {
      sail_int zgsz3775;
      CREATE(sail_int)(&zgsz3775);
      CONVERT_OF(sail_int, mach_int)(&zgsz3775, INT64_C(64));
      lbits zgsz3776;
      CREATE(lbits)(&zgsz3776);
      UNDEFINED(lbits)(&zgsz3776, zgsz3775);
      zTestAddr = CONVERT_OF(fbits, lbits)(zgsz3776, true);
      KILL(lbits)(&zgsz3776);
      KILL(sail_int)(&zgsz3775);
    }
    unit zgsz3104;
    zgsz3104 = UNIT;
  }
  int64_t zgaz3393;
  {
    sail_int zgsz3778;
    CREATE(sail_int)(&zgsz3778);
    CONVERT_OF(sail_int, mach_int)(&zgsz3778, INT64_C(64));
    sail_int zgsz3779;
    CREATE(sail_int)(&zgsz3779);
    CONVERT_OF(sail_int, mach_int)(&zgsz3779, INT64_C(1));
    sail_int zgsz3780;
    CREATE(sail_int)(&zgsz3780);
    add_int(&zgsz3780, zgsz3778, zgsz3779);
    zgaz3393 = CONVERT_OF(mach_int, sail_int)(zgsz3780);
    KILL(sail_int)(&zgsz3780);
    KILL(sail_int)(&zgsz3779);
    KILL(sail_int)(&zgsz3778);
  }
  {
    sail_int zgsz3777;
    CREATE(sail_int)(&zgsz3777);
    CONVERT_OF(sail_int, mach_int)(&zgsz3777, zgaz3393);
    UNDEFINED(lbits)(&zTestLen, zgsz3777);
    KILL(sail_int)(&zgsz3777);
  }
  zcbz345 = UNIT;

end_function_193: ;
  return zcbz345;
end_block_exception_194: ;

  return UNIT;
}

static void model_init(void)
{
  setup_rts();
  current_exception = sail_malloc(sizeof(struct zexception));
  CREATE(zexception)(current_exception);
  throw_location = sail_malloc(sizeof(sail_string));
  CREATE(sail_string)(throw_location);
  create_letbind_0();
  create_letbind_1();
  create_letbind_2();
  create_letbind_3();
  create_letbind_4();
  create_letbind_5();
  create_letbind_6();
  create_letbind_7();
  create_letbind_8();
  create_letbind_9();
  create_letbind_10();
  create_letbind_11();
  create_letbind_12();
  create_letbind_13();
  create_letbind_14();
  create_letbind_15();
  create_letbind_16();
  create_letbind_17();
  create_letbind_18();
  create_letbind_19();
  create_letbind_20();
  create_letbind_21();
  create_letbind_22();
  create_letbind_23();
  create_letbind_24();
  create_letbind_25();
  create_letbind_26();
  create_letbind_27();
  CREATE(lbits)(&zTestLen);
  sailgen_initializze_registers(UNIT);
}

static void model_fini(void)
{
  kill_letbind_27();
  kill_letbind_26();
  kill_letbind_25();
  kill_letbind_24();
  kill_letbind_23();
  kill_letbind_22();
  kill_letbind_21();
  kill_letbind_20();
  kill_letbind_19();
  kill_letbind_18();
  kill_letbind_17();
  kill_letbind_16();
  kill_letbind_15();
  kill_letbind_14();
  kill_letbind_13();
  kill_letbind_12();
  kill_letbind_11();
  kill_letbind_10();
  kill_letbind_9();
  kill_letbind_8();
  kill_letbind_7();
  kill_letbind_6();
  kill_letbind_5();
  kill_letbind_4();
  kill_letbind_3();
  kill_letbind_2();
  kill_letbind_1();
  kill_letbind_0();
  KILL(lbits)(&zTestLen);
  cleanup_rts();
  if (have_exception) {fprintf(stderr, "Exiting due to uncaught exception: %s\n", *throw_location);}
  KILL(zexception)(current_exception);
  sail_free(current_exception);
  KILL(sail_string)(throw_location);
  sail_free(throw_location);
  if (have_exception) {exit(EXIT_FAILURE);}
}

void model_pre_exit()
{
}

static int model_main(int argc, char *argv[])
{
  model_init();
  if (process_arguments(argc, argv)) exit(EXIT_FAILURE);
  sailgen_main(UNIT);
  model_fini();
  model_pre_exit();
  return EXIT_SUCCESS;
}


