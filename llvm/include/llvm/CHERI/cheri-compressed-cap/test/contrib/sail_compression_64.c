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
  __label__ end_function_7, end_block_exception_8, end_function_202;

  lbits zgaz32;
  CREATE(lbits)(&zgaz32);
  zeros(&zgaz32, zn);
  not_bits((*(&zcbz32)), zgaz32);
  KILL(lbits)(&zgaz32);
end_function_7: ;
  goto end_function_202;
end_block_exception_8: ;
  goto end_function_202;
end_function_202: ;
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
  __label__ end_function_13, end_block_exception_14, end_function_201;

  sign_extend((*(&zcbz34)), zv, zm);
end_function_13: ;
  goto end_function_201;
end_block_exception_14: ;
  goto end_function_201;
end_function_201: ;
}

static void sailgen_EXTZ(lbits *zcbz35, sail_int zm, lbits zv)
{
  __label__ end_function_16, end_block_exception_17, end_function_200;

  zero_extend((*(&zcbz35)), zv, zm);
end_function_16: ;
  goto end_function_200;
end_block_exception_17: ;
  goto end_function_200;
end_function_200: ;
}

static void sailgen_zzeros_implicit(lbits *rop, sail_int);

static void sailgen_zzeros_implicit(lbits *zcbz36, sail_int zn)
{
  __label__ end_function_19, end_block_exception_20, end_function_199;

  zeros((*(&zcbz36)), zn);
end_function_19: ;
  goto end_function_199;
end_block_exception_20: ;
  goto end_function_199;
end_function_199: ;
}

static void sailgen_ones(lbits *rop, sail_int);

static void sailgen_ones(lbits *zcbz37, sail_int zn)
{
  __label__ end_function_22, end_block_exception_23, end_function_198;

  sailgen_sail_ones((*(&zcbz37)), zn);
end_function_22: ;
  goto end_function_198;
end_block_exception_23: ;
  goto end_function_198;
end_function_198: ;
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
  __label__ end_function_34, end_block_exception_35, end_function_197;

  {
    sail_int zgsz3106;
    CREATE(sail_int)(&zgsz3106);
    CONVERT_OF(sail_int, mach_int)(&zgsz3106, INT64_C(0));
    get_slice_int((*(&zcbz310)), zl, zn, zgsz3106);
    KILL(sail_int)(&zgsz3106);
  }
end_function_34: ;
  goto end_function_197;
end_block_exception_35: ;
  goto end_function_197;
end_function_197: ;
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
  __label__ end_function_43, end_block_exception_44, end_function_196;

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
  goto end_function_196;
end_block_exception_44: ;
  goto end_function_196;
end_function_196: ;
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
  zgsz38 = INT64_C(8);
  zcap_sizze = zgsz38;

let_end_50: ;
}
static void kill_letbind_5(void) {
}

static int64_t zlog2_cap_sizze;
static void create_letbind_6(void) {


  int64_t zgsz39;
  zgsz39 = INT64_C(3);
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
  zgsz311 = INT64_C(0);
  zcap_uperms_width = zgsz311;

let_end_53: ;
}
static void kill_letbind_8(void) {
}

static int64_t zcap_otype_width;
static void create_letbind_9(void) {


  int64_t zgsz312;
  zgsz312 = INT64_C(4);
  zcap_otype_width = zgsz312;

let_end_54: ;
}
static void kill_letbind_9(void) {
}

static int64_t zcap_reserved_width;
static void create_letbind_10(void) {


  int64_t zgsz313;
  zgsz313 = INT64_C(0);
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
  zgsz315 = INT64_C(8);
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
  zgsz317 = INT64_C(32);
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
    CONVERT_OF(sail_int, mach_int)(&zgsz3108, INT64_C(32));
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
  zgsz319 = INT64_C(8);
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

static struct zEncCapability sailgen_capBitsToEncCapability(uint64_t);

static struct zEncCapability sailgen_capBitsToEncCapability(uint64_t zc)
{
  __label__ end_function_64, end_block_exception_65;

  struct zEncCapability zcbz314;
  uint64_t zgaz39;
  {
    lbits zgsz3133;
    CREATE(lbits)(&zgsz3133);
    CONVERT_OF(lbits, fbits)(&zgsz3133, zc, UINT64_C(64) , true);
    sail_int zgsz3134;
    CREATE(sail_int)(&zgsz3134);
    CONVERT_OF(sail_int, mach_int)(&zgsz3134, INT64_C(63));
    sail_int zgsz3135;
    CREATE(sail_int)(&zgsz3135);
    CONVERT_OF(sail_int, mach_int)(&zgsz3135, INT64_C(52));
    lbits zgsz3136;
    CREATE(lbits)(&zgsz3136);
    vector_subrange_lbits(&zgsz3136, zgsz3133, zgsz3134, zgsz3135);
    zgaz39 = CONVERT_OF(fbits, lbits)(zgsz3136, true);
    KILL(lbits)(&zgsz3136);
    KILL(sail_int)(&zgsz3135);
    KILL(sail_int)(&zgsz3134);
    KILL(lbits)(&zgsz3133);
  }
  uint64_t zgaz310;
  {
    lbits zgsz3129;
    CREATE(lbits)(&zgsz3129);
    CONVERT_OF(lbits, fbits)(&zgsz3129, zc, UINT64_C(64) , true);
    sail_int zgsz3130;
    CREATE(sail_int)(&zgsz3130);
    CONVERT_OF(sail_int, mach_int)(&zgsz3130, INT64_C(51));
    sail_int zgsz3131;
    CREATE(sail_int)(&zgsz3131);
    CONVERT_OF(sail_int, mach_int)(&zgsz3131, INT64_C(51));
    lbits zgsz3132;
    CREATE(lbits)(&zgsz3132);
    vector_subrange_lbits(&zgsz3132, zgsz3129, zgsz3130, zgsz3131);
    zgaz310 = CONVERT_OF(fbits, lbits)(zgsz3132, true);
    KILL(lbits)(&zgsz3132);
    KILL(sail_int)(&zgsz3131);
    KILL(sail_int)(&zgsz3130);
    KILL(lbits)(&zgsz3129);
  }
  uint64_t zgaz311;
  {
    lbits zgsz3125;
    CREATE(lbits)(&zgsz3125);
    CONVERT_OF(lbits, fbits)(&zgsz3125, zc, UINT64_C(64) , true);
    sail_int zgsz3126;
    CREATE(sail_int)(&zgsz3126);
    CONVERT_OF(sail_int, mach_int)(&zgsz3126, INT64_C(50));
    sail_int zgsz3127;
    CREATE(sail_int)(&zgsz3127);
    CONVERT_OF(sail_int, mach_int)(&zgsz3127, INT64_C(47));
    lbits zgsz3128;
    CREATE(lbits)(&zgsz3128);
    vector_subrange_lbits(&zgsz3128, zgsz3125, zgsz3126, zgsz3127);
    zgaz311 = CONVERT_OF(fbits, lbits)(zgsz3128, true);
    KILL(lbits)(&zgsz3128);
    KILL(sail_int)(&zgsz3127);
    KILL(sail_int)(&zgsz3126);
    KILL(lbits)(&zgsz3125);
  }
  fbits zgaz312;
  {
    lbits zgsz3123;
    CREATE(lbits)(&zgsz3123);
    CONVERT_OF(lbits, fbits)(&zgsz3123, zc, UINT64_C(64) , true);
    sail_int zgsz3124;
    CREATE(sail_int)(&zgsz3124);
    CONVERT_OF(sail_int, mach_int)(&zgsz3124, INT64_C(46));
    zgaz312 = bitvector_access(zgsz3123, zgsz3124);
    KILL(sail_int)(&zgsz3124);
    KILL(lbits)(&zgsz3123);
  }
  uint64_t zgaz313;
  {
    lbits zgsz3119;
    CREATE(lbits)(&zgsz3119);
    CONVERT_OF(lbits, fbits)(&zgsz3119, zc, UINT64_C(64) , true);
    sail_int zgsz3120;
    CREATE(sail_int)(&zgsz3120);
    CONVERT_OF(sail_int, mach_int)(&zgsz3120, INT64_C(45));
    sail_int zgsz3121;
    CREATE(sail_int)(&zgsz3121);
    CONVERT_OF(sail_int, mach_int)(&zgsz3121, INT64_C(40));
    lbits zgsz3122;
    CREATE(lbits)(&zgsz3122);
    vector_subrange_lbits(&zgsz3122, zgsz3119, zgsz3120, zgsz3121);
    zgaz313 = CONVERT_OF(fbits, lbits)(zgsz3122, true);
    KILL(lbits)(&zgsz3122);
    KILL(sail_int)(&zgsz3121);
    KILL(sail_int)(&zgsz3120);
    KILL(lbits)(&zgsz3119);
  }
  uint64_t zgaz314;
  {
    lbits zgsz3115;
    CREATE(lbits)(&zgsz3115);
    CONVERT_OF(lbits, fbits)(&zgsz3115, zc, UINT64_C(64) , true);
    sail_int zgsz3116;
    CREATE(sail_int)(&zgsz3116);
    CONVERT_OF(sail_int, mach_int)(&zgsz3116, INT64_C(39));
    sail_int zgsz3117;
    CREATE(sail_int)(&zgsz3117);
    CONVERT_OF(sail_int, mach_int)(&zgsz3117, INT64_C(32));
    lbits zgsz3118;
    CREATE(lbits)(&zgsz3118);
    vector_subrange_lbits(&zgsz3118, zgsz3115, zgsz3116, zgsz3117);
    zgaz314 = CONVERT_OF(fbits, lbits)(zgsz3118, true);
    KILL(lbits)(&zgsz3118);
    KILL(sail_int)(&zgsz3117);
    KILL(sail_int)(&zgsz3116);
    KILL(lbits)(&zgsz3115);
  }
  uint64_t zgaz315;
  {
    lbits zgsz3111;
    CREATE(lbits)(&zgsz3111);
    CONVERT_OF(lbits, fbits)(&zgsz3111, zc, UINT64_C(64) , true);
    sail_int zgsz3112;
    CREATE(sail_int)(&zgsz3112);
    CONVERT_OF(sail_int, mach_int)(&zgsz3112, INT64_C(31));
    sail_int zgsz3113;
    CREATE(sail_int)(&zgsz3113);
    CONVERT_OF(sail_int, mach_int)(&zgsz3113, INT64_C(0));
    lbits zgsz3114;
    CREATE(lbits)(&zgsz3114);
    vector_subrange_lbits(&zgsz3114, zgsz3111, zgsz3112, zgsz3113);
    zgaz315 = CONVERT_OF(fbits, lbits)(zgsz3114, true);
    KILL(lbits)(&zgsz3114);
    KILL(sail_int)(&zgsz3113);
    KILL(sail_int)(&zgsz3112);
    KILL(lbits)(&zgsz3111);
  }
  struct zEncCapability zgsz321;
  zgsz321.zB = zgaz314;
  zgsz321.zT = zgaz313;
  zgsz321.zaddress = zgaz315;
  zgsz321.zflags = zgaz310;
  zgsz321.zinternal_E = zgaz312;
  zgsz321.zotype = zgaz311;
  zgsz321.zperms = zgaz39;
  zgsz321.zreserved = UINT64_C(0);
  zcbz314 = zgsz321;








end_function_64: ;
  return zcbz314;
end_block_exception_65: ;
  struct zEncCapability zcbz346 = { .zreserved = UINT64_C(0xdeadc0de), .zperms = UINT64_C(0xdeadc0de), .zotype = UINT64_C(0xdeadc0de), .zinternal_E = UINT64_C(0), .zflags = UINT64_C(0xdeadc0de), .zaddress = UINT64_C(0xdeadc0de), .zT = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz346;
}

static uint64_t sailgen_encCapToBits(struct zEncCapability);

static uint64_t sailgen_encCapToBits(struct zEncCapability zcap)
{
  __label__ end_function_67, end_block_exception_68;

  uint64_t zcbz315;
  uint64_t zgaz329;
  zgaz329 = zcap.zperms;
  uint64_t zgaz330;
  {
    uint64_t zgaz327;
    zgaz327 = zcap.zreserved;
    uint64_t zgaz328;
    {
      uint64_t zgaz325;
      zgaz325 = zcap.zflags;
      uint64_t zgaz326;
      {
        uint64_t zgaz323;
        zgaz323 = zcap.zotype;
        uint64_t zgaz324;
        {
          uint64_t zgaz321;
          {
            fbits zgaz316;
            zgaz316 = zcap.zinternal_E;
            uint64_t zgsz322;
            zgsz322 = UINT64_C(0b0);
            zgsz322 = update_fbits(zgsz322, INT64_C(0), zgaz316);
            zgaz321 = zgsz322;
          }
          uint64_t zgaz322;
          {
            uint64_t zgaz319;
            zgaz319 = zcap.zT;
            uint64_t zgaz320;
            {
              uint64_t zgaz317;
              zgaz317 = zcap.zB;
              uint64_t zgaz318;
              zgaz318 = zcap.zaddress;
              {
                lbits zgsz3137;
                CREATE(lbits)(&zgsz3137);
                CONVERT_OF(lbits, fbits)(&zgsz3137, zgaz317, UINT64_C(8) , true);
                lbits zgsz3138;
                CREATE(lbits)(&zgsz3138);
                CONVERT_OF(lbits, fbits)(&zgsz3138, zgaz318, UINT64_C(32) , true);
                lbits zgsz3139;
                CREATE(lbits)(&zgsz3139);
                append(&zgsz3139, zgsz3137, zgsz3138);
                zgaz320 = CONVERT_OF(fbits, lbits)(zgsz3139, true);
                KILL(lbits)(&zgsz3139);
                KILL(lbits)(&zgsz3138);
                KILL(lbits)(&zgsz3137);
              }
            }
            {
              lbits zgsz3140;
              CREATE(lbits)(&zgsz3140);
              CONVERT_OF(lbits, fbits)(&zgsz3140, zgaz319, UINT64_C(6) , true);
              lbits zgsz3141;
              CREATE(lbits)(&zgsz3141);
              CONVERT_OF(lbits, fbits)(&zgsz3141, zgaz320, UINT64_C(40) , true);
              lbits zgsz3142;
              CREATE(lbits)(&zgsz3142);
              append(&zgsz3142, zgsz3140, zgsz3141);
              zgaz322 = CONVERT_OF(fbits, lbits)(zgsz3142, true);
              KILL(lbits)(&zgsz3142);
              KILL(lbits)(&zgsz3141);
              KILL(lbits)(&zgsz3140);
            }
          }
          {
            lbits zgsz3143;
            CREATE(lbits)(&zgsz3143);
            CONVERT_OF(lbits, fbits)(&zgsz3143, zgaz321, UINT64_C(1) , true);
            lbits zgsz3144;
            CREATE(lbits)(&zgsz3144);
            CONVERT_OF(lbits, fbits)(&zgsz3144, zgaz322, UINT64_C(46) , true);
            lbits zgsz3145;
            CREATE(lbits)(&zgsz3145);
            append(&zgsz3145, zgsz3143, zgsz3144);
            zgaz324 = CONVERT_OF(fbits, lbits)(zgsz3145, true);
            KILL(lbits)(&zgsz3145);
            KILL(lbits)(&zgsz3144);
            KILL(lbits)(&zgsz3143);
          }
        }
        {
          lbits zgsz3146;
          CREATE(lbits)(&zgsz3146);
          CONVERT_OF(lbits, fbits)(&zgsz3146, zgaz323, UINT64_C(4) , true);
          lbits zgsz3147;
          CREATE(lbits)(&zgsz3147);
          CONVERT_OF(lbits, fbits)(&zgsz3147, zgaz324, UINT64_C(47) , true);
          lbits zgsz3148;
          CREATE(lbits)(&zgsz3148);
          append(&zgsz3148, zgsz3146, zgsz3147);
          zgaz326 = CONVERT_OF(fbits, lbits)(zgsz3148, true);
          KILL(lbits)(&zgsz3148);
          KILL(lbits)(&zgsz3147);
          KILL(lbits)(&zgsz3146);
        }
      }
      {
        lbits zgsz3149;
        CREATE(lbits)(&zgsz3149);
        CONVERT_OF(lbits, fbits)(&zgsz3149, zgaz325, UINT64_C(1) , true);
        lbits zgsz3150;
        CREATE(lbits)(&zgsz3150);
        CONVERT_OF(lbits, fbits)(&zgsz3150, zgaz326, UINT64_C(51) , true);
        lbits zgsz3151;
        CREATE(lbits)(&zgsz3151);
        append(&zgsz3151, zgsz3149, zgsz3150);
        zgaz328 = CONVERT_OF(fbits, lbits)(zgsz3151, true);
        KILL(lbits)(&zgsz3151);
        KILL(lbits)(&zgsz3150);
        KILL(lbits)(&zgsz3149);
      }
    }
    {
      lbits zgsz3152;
      CREATE(lbits)(&zgsz3152);
      CONVERT_OF(lbits, fbits)(&zgsz3152, zgaz327, UINT64_C(0) , true);
      lbits zgsz3153;
      CREATE(lbits)(&zgsz3153);
      CONVERT_OF(lbits, fbits)(&zgsz3153, zgaz328, UINT64_C(52) , true);
      lbits zgsz3154;
      CREATE(lbits)(&zgsz3154);
      append(&zgsz3154, zgsz3152, zgsz3153);
      zgaz330 = CONVERT_OF(fbits, lbits)(zgsz3154, true);
      KILL(lbits)(&zgsz3154);
      KILL(lbits)(&zgsz3153);
      KILL(lbits)(&zgsz3152);
    }
  }
  {
    lbits zgsz3155;
    CREATE(lbits)(&zgsz3155);
    CONVERT_OF(lbits, fbits)(&zgsz3155, zgaz329, UINT64_C(12) , true);
    lbits zgsz3156;
    CREATE(lbits)(&zgsz3156);
    CONVERT_OF(lbits, fbits)(&zgsz3156, zgaz330, UINT64_C(52) , true);
    lbits zgsz3157;
    CREATE(lbits)(&zgsz3157);
    append(&zgsz3157, zgsz3155, zgsz3156);
    zcbz315 = CONVERT_OF(fbits, lbits)(zgsz3157, true);
    KILL(lbits)(&zgsz3157);
    KILL(lbits)(&zgsz3156);
    KILL(lbits)(&zgsz3155);
  }


end_function_67: ;
  return zcbz315;
end_block_exception_68: ;

  return UINT64_C(0xdeadc0de);
}

static int64_t zcap_max_addr;
static void create_letbind_18(void) {


  int64_t zgsz323;
  {
    sail_int zgsz3158;
    CREATE(sail_int)(&zgsz3158);
    CONVERT_OF(sail_int, mach_int)(&zgsz3158, zcap_addr_width);
    sail_int zgsz3159;
    CREATE(sail_int)(&zgsz3159);
    sailgen_MAX(&zgsz3159, zgsz3158);
    zgsz323 = CONVERT_OF(mach_int, sail_int)(zgsz3159);
    KILL(sail_int)(&zgsz3159);
    KILL(sail_int)(&zgsz3158);
  }
  zcap_max_addr = zgsz323;

let_end_69: ;
}
static void kill_letbind_18(void) {
}

static int64_t zcap_max_otype;
static void create_letbind_19(void) {


  int64_t zgsz324;
  int64_t zgaz331;
  {
    sail_int zgsz3163;
    CREATE(sail_int)(&zgsz3163);
    CONVERT_OF(sail_int, mach_int)(&zgsz3163, zcap_otype_width);
    sail_int zgsz3164;
    CREATE(sail_int)(&zgsz3164);
    sailgen_MAX(&zgsz3164, zgsz3163);
    zgaz331 = CONVERT_OF(mach_int, sail_int)(zgsz3164);
    KILL(sail_int)(&zgsz3164);
    KILL(sail_int)(&zgsz3163);
  }
  {
    sail_int zgsz3160;
    CREATE(sail_int)(&zgsz3160);
    CONVERT_OF(sail_int, mach_int)(&zgsz3160, zgaz331);
    sail_int zgsz3161;
    CREATE(sail_int)(&zgsz3161);
    CONVERT_OF(sail_int, mach_int)(&zgsz3161, zreserved_otypes);
    sail_int zgsz3162;
    CREATE(sail_int)(&zgsz3162);
    sub_int(&zgsz3162, zgsz3160, zgsz3161);
    zgsz324 = CONVERT_OF(mach_int, sail_int)(zgsz3162);
    KILL(sail_int)(&zgsz3162);
    KILL(sail_int)(&zgsz3161);
    KILL(sail_int)(&zgsz3160);
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
    sail_int zgsz3165;
    CREATE(sail_int)(&zgsz3165);
    CONVERT_OF(sail_int, mach_int)(&zgsz3165, INT64_C(15));
    sail_int zgsz3166;
    CREATE(sail_int)(&zgsz3166);
    CONVERT_OF(sail_int, mach_int)(&zgsz3166, INT64_C(0));
    sail_int zgsz3167;
    CREATE(sail_int)(&zgsz3167);
    add_int(&zgsz3167, zgsz3165, zgsz3166);
    zgsz326 = CONVERT_OF(mach_int, sail_int)(zgsz3167);
    KILL(sail_int)(&zgsz3167);
    KILL(sail_int)(&zgsz3166);
    KILL(sail_int)(&zgsz3165);
  }
  zcap_perms_width = zgsz326;

let_end_72: ;
}
static void kill_letbind_21(void) {
}

static int64_t zcap_max_E;
static void create_letbind_22(void) {


  int64_t zgsz327;
  int64_t zgaz332;
  {
    sail_int zgsz3171;
    CREATE(sail_int)(&zgsz3171);
    CONVERT_OF(sail_int, mach_int)(&zgsz3171, zcap_len_width);
    sail_int zgsz3172;
    CREATE(sail_int)(&zgsz3172);
    CONVERT_OF(sail_int, mach_int)(&zgsz3172, zcap_mantissa_width);
    sail_int zgsz3173;
    CREATE(sail_int)(&zgsz3173);
    sub_int(&zgsz3173, zgsz3171, zgsz3172);
    zgaz332 = CONVERT_OF(mach_int, sail_int)(zgsz3173);
    KILL(sail_int)(&zgsz3173);
    KILL(sail_int)(&zgsz3172);
    KILL(sail_int)(&zgsz3171);
  }
  {
    sail_int zgsz3168;
    CREATE(sail_int)(&zgsz3168);
    CONVERT_OF(sail_int, mach_int)(&zgsz3168, zgaz332);
    sail_int zgsz3169;
    CREATE(sail_int)(&zgsz3169);
    CONVERT_OF(sail_int, mach_int)(&zgsz3169, INT64_C(1));
    sail_int zgsz3170;
    CREATE(sail_int)(&zgsz3170);
    add_int(&zgsz3170, zgsz3168, zgsz3169);
    zgsz327 = CONVERT_OF(mach_int, sail_int)(zgsz3170);
    KILL(sail_int)(&zgsz3170);
    KILL(sail_int)(&zgsz3169);
    KILL(sail_int)(&zgsz3168);
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
    sail_int zgsz3174;
    CREATE(sail_int)(&zgsz3174);
    CONVERT_OF(sail_int, mach_int)(&zgsz3174, zcap_E_width);
    sail_int zgsz3175;
    CREATE(sail_int)(&zgsz3175);
    CONVERT_OF(sail_int, mach_int)(&zgsz3175, zcap_max_E);
    lbits zgsz3176;
    CREATE(lbits)(&zgsz3176);
    sailgen_to_bits(&zgsz3176, zgsz3174, zgsz3175);
    zgsz328 = CONVERT_OF(fbits, lbits)(zgsz3176, true);
    KILL(lbits)(&zgsz3176);
    KILL(sail_int)(&zgsz3175);
    KILL(sail_int)(&zgsz3174);
  }
  zcap_reset_E = zgsz328;

let_end_74: ;
}
static void kill_letbind_23(void) {
}

static uint64_t zcap_reset_T;
static void create_letbind_24(void) {


  uint64_t zgsz329;
  uint64_t zgaz334;
  {
    int64_t zgaz333;
    {
      sail_int zgsz3179;
      CREATE(sail_int)(&zgsz3179);
      CONVERT_OF(sail_int, mach_int)(&zgsz3179, zcap_mantissa_width);
      sail_int zgsz3180;
      CREATE(sail_int)(&zgsz3180);
      CONVERT_OF(sail_int, mach_int)(&zgsz3180, INT64_C(2));
      sail_int zgsz3181;
      CREATE(sail_int)(&zgsz3181);
      sub_int(&zgsz3181, zgsz3179, zgsz3180);
      zgaz333 = CONVERT_OF(mach_int, sail_int)(zgsz3181);
      KILL(sail_int)(&zgsz3181);
      KILL(sail_int)(&zgsz3180);
      KILL(sail_int)(&zgsz3179);
    }
    {
      sail_int zgsz3177;
      CREATE(sail_int)(&zgsz3177);
      CONVERT_OF(sail_int, mach_int)(&zgsz3177, zgaz333);
      lbits zgsz3178;
      CREATE(lbits)(&zgsz3178);
      sailgen_zzeros_implicit(&zgsz3178, zgsz3177);
      zgaz334 = CONVERT_OF(fbits, lbits)(zgsz3178, true);
      KILL(lbits)(&zgsz3178);
      KILL(sail_int)(&zgsz3177);
    }
  }
  {
    lbits zgsz3182;
    CREATE(lbits)(&zgsz3182);
    CONVERT_OF(lbits, fbits)(&zgsz3182, UINT64_C(0b01), UINT64_C(2) , true);
    lbits zgsz3183;
    CREATE(lbits)(&zgsz3183);
    CONVERT_OF(lbits, fbits)(&zgsz3183, zgaz334, UINT64_C(6) , true);
    lbits zgsz3184;
    CREATE(lbits)(&zgsz3184);
    append(&zgsz3184, zgsz3182, zgsz3183);
    zgsz329 = CONVERT_OF(fbits, lbits)(zgsz3184, true);
    KILL(lbits)(&zgsz3184);
    KILL(lbits)(&zgsz3183);
    KILL(lbits)(&zgsz3182);
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
  bool zgaz335;
  zgaz335 = undefined_bool(UNIT);
  uint64_t zgaz336;
  {
    sail_int zgsz3197;
    CREATE(sail_int)(&zgsz3197);
    CONVERT_OF(sail_int, mach_int)(&zgsz3197, INT64_C(0));
    lbits zgsz3198;
    CREATE(lbits)(&zgsz3198);
    UNDEFINED(lbits)(&zgsz3198, zgsz3197);
    zgaz336 = CONVERT_OF(fbits, lbits)(zgsz3198, true);
    KILL(lbits)(&zgsz3198);
    KILL(sail_int)(&zgsz3197);
  }
  bool zgaz337;
  zgaz337 = undefined_bool(UNIT);
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
  uint64_t zgaz349;
  {
    sail_int zgsz3195;
    CREATE(sail_int)(&zgsz3195);
    CONVERT_OF(sail_int, mach_int)(&zgsz3195, INT64_C(0));
    lbits zgsz3196;
    CREATE(lbits)(&zgsz3196);
    UNDEFINED(lbits)(&zgsz3196, zgsz3195);
    zgaz349 = CONVERT_OF(fbits, lbits)(zgsz3196, true);
    KILL(lbits)(&zgsz3196);
    KILL(sail_int)(&zgsz3195);
  }
  bool zgaz350;
  zgaz350 = undefined_bool(UNIT);
  bool zgaz351;
  zgaz351 = undefined_bool(UNIT);
  uint64_t zgaz352;
  {
    sail_int zgsz3193;
    CREATE(sail_int)(&zgsz3193);
    CONVERT_OF(sail_int, mach_int)(&zgsz3193, INT64_C(6));
    lbits zgsz3194;
    CREATE(lbits)(&zgsz3194);
    UNDEFINED(lbits)(&zgsz3194, zgsz3193);
    zgaz352 = CONVERT_OF(fbits, lbits)(zgsz3194, true);
    KILL(lbits)(&zgsz3194);
    KILL(sail_int)(&zgsz3193);
  }
  uint64_t zgaz353;
  {
    sail_int zgsz3191;
    CREATE(sail_int)(&zgsz3191);
    CONVERT_OF(sail_int, mach_int)(&zgsz3191, INT64_C(8));
    lbits zgsz3192;
    CREATE(lbits)(&zgsz3192);
    UNDEFINED(lbits)(&zgsz3192, zgsz3191);
    zgaz353 = CONVERT_OF(fbits, lbits)(zgsz3192, true);
    KILL(lbits)(&zgsz3192);
    KILL(sail_int)(&zgsz3191);
  }
  uint64_t zgaz354;
  {
    sail_int zgsz3189;
    CREATE(sail_int)(&zgsz3189);
    CONVERT_OF(sail_int, mach_int)(&zgsz3189, INT64_C(8));
    lbits zgsz3190;
    CREATE(lbits)(&zgsz3190);
    UNDEFINED(lbits)(&zgsz3190, zgsz3189);
    zgaz354 = CONVERT_OF(fbits, lbits)(zgsz3190, true);
    KILL(lbits)(&zgsz3190);
    KILL(sail_int)(&zgsz3189);
  }
  uint64_t zgaz355;
  {
    sail_int zgsz3187;
    CREATE(sail_int)(&zgsz3187);
    CONVERT_OF(sail_int, mach_int)(&zgsz3187, INT64_C(4));
    lbits zgsz3188;
    CREATE(lbits)(&zgsz3188);
    UNDEFINED(lbits)(&zgsz3188, zgsz3187);
    zgaz355 = CONVERT_OF(fbits, lbits)(zgsz3188, true);
    KILL(lbits)(&zgsz3188);
    KILL(sail_int)(&zgsz3187);
  }
  uint64_t zgaz356;
  {
    sail_int zgsz3185;
    CREATE(sail_int)(&zgsz3185);
    CONVERT_OF(sail_int, mach_int)(&zgsz3185, INT64_C(32));
    lbits zgsz3186;
    CREATE(lbits)(&zgsz3186);
    UNDEFINED(lbits)(&zgsz3186, zgsz3185);
    zgaz356 = CONVERT_OF(fbits, lbits)(zgsz3186, true);
    KILL(lbits)(&zgsz3186);
    KILL(sail_int)(&zgsz3185);
  }
  struct zCapability zgsz331;
  zgsz331.zB = zgaz353;
  zgsz331.zE = zgaz352;
  zgsz331.zT = zgaz354;
  zgsz331.zaccess_system_regs = zgaz338;
  zgsz331.zaddress = zgaz356;
  zgsz331.zflag_cap_mode = zgaz350;
  zgsz331.zglobal = zgaz348;
  zgsz331.zinternal_E = zgaz351;
  zgsz331.zotype = zgaz355;
  zgsz331.zpermit_cinvoke = zgaz340;
  zgsz331.zpermit_execute = zgaz347;
  zgsz331.zpermit_load = zgaz346;
  zgsz331.zpermit_load_cap = zgaz344;
  zgsz331.zpermit_seal = zgaz341;
  zgsz331.zpermit_set_CID = zgaz337;
  zgsz331.zpermit_store = zgaz345;
  zgsz331.zpermit_store_cap = zgaz343;
  zgsz331.zpermit_store_local_cap = zgaz342;
  zgsz331.zpermit_unseal = zgaz339;
  zgsz331.zreserved = zgaz349;
  zgsz331.ztag = zgaz335;
  zgsz331.zuperms = zgaz336;
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
  uint64_t zgaz357;
  {
    sail_int zgsz3208;
    CREATE(sail_int)(&zgsz3208);
    CONVERT_OF(sail_int, mach_int)(&zgsz3208, INT64_C(0));
    lbits zgsz3209;
    CREATE(lbits)(&zgsz3209);
    sailgen_zzeros_implicit(&zgsz3209, zgsz3208);
    zgaz357 = CONVERT_OF(fbits, lbits)(zgsz3209, true);
    KILL(lbits)(&zgsz3209);
    KILL(sail_int)(&zgsz3208);
  }
  uint64_t zgaz358;
  {
    sail_int zgsz3206;
    CREATE(sail_int)(&zgsz3206);
    CONVERT_OF(sail_int, mach_int)(&zgsz3206, INT64_C(0));
    lbits zgsz3207;
    CREATE(lbits)(&zgsz3207);
    sailgen_zzeros_implicit(&zgsz3207, zgsz3206);
    zgaz358 = CONVERT_OF(fbits, lbits)(zgsz3207, true);
    KILL(lbits)(&zgsz3207);
    KILL(sail_int)(&zgsz3206);
  }
  uint64_t zgaz359;
  {
    sail_int zgsz3204;
    CREATE(sail_int)(&zgsz3204);
    CONVERT_OF(sail_int, mach_int)(&zgsz3204, INT64_C(8));
    lbits zgsz3205;
    CREATE(lbits)(&zgsz3205);
    sailgen_zzeros_implicit(&zgsz3205, zgsz3204);
    zgaz359 = CONVERT_OF(fbits, lbits)(zgsz3205, true);
    KILL(lbits)(&zgsz3205);
    KILL(sail_int)(&zgsz3204);
  }
  uint64_t zgaz360;
  {
    sail_int zgsz3201;
    CREATE(sail_int)(&zgsz3201);
    CONVERT_OF(sail_int, mach_int)(&zgsz3201, zcap_otype_width);
    sail_int zgsz3202;
    CREATE(sail_int)(&zgsz3202);
    CONVERT_OF(sail_int, mach_int)(&zgsz3202, zotype_unsealed);
    lbits zgsz3203;
    CREATE(lbits)(&zgsz3203);
    sailgen_to_bits(&zgsz3203, zgsz3201, zgsz3202);
    zgaz360 = CONVERT_OF(fbits, lbits)(zgsz3203, true);
    KILL(lbits)(&zgsz3203);
    KILL(sail_int)(&zgsz3202);
    KILL(sail_int)(&zgsz3201);
  }
  uint64_t zgaz361;
  {
    sail_int zgsz3199;
    CREATE(sail_int)(&zgsz3199);
    CONVERT_OF(sail_int, mach_int)(&zgsz3199, INT64_C(32));
    lbits zgsz3200;
    CREATE(lbits)(&zgsz3200);
    sailgen_zzeros_implicit(&zgsz3200, zgsz3199);
    zgaz361 = CONVERT_OF(fbits, lbits)(zgsz3200, true);
    KILL(lbits)(&zgsz3200);
    KILL(sail_int)(&zgsz3199);
  }
  struct zCapability zgsz332;
  zgsz332.zB = zgaz359;
  zgsz332.zE = zcap_reset_E;
  zgsz332.zT = zcap_reset_T;
  zgsz332.zaccess_system_regs = false;
  zgsz332.zaddress = zgaz361;
  zgsz332.zflag_cap_mode = false;
  zgsz332.zglobal = false;
  zgsz332.zinternal_E = true;
  zgsz332.zotype = zgaz360;
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
  zgsz332.zreserved = zgaz358;
  zgsz332.ztag = false;
  zgsz332.zuperms = zgaz357;
  zgsz333 = zgsz332;






  znull_cap = zgsz333;

let_end_79: ;
}
static void kill_letbind_25(void) {
}

static struct zCapability zdefault_cap;
static void create_letbind_26(void) {


  struct zCapability zgsz335;
  uint64_t zgaz362;
  {
    sail_int zgsz3219;
    CREATE(sail_int)(&zgsz3219);
    CONVERT_OF(sail_int, mach_int)(&zgsz3219, INT64_C(0));
    lbits zgsz3220;
    CREATE(lbits)(&zgsz3220);
    sailgen_ones(&zgsz3220, zgsz3219);
    zgaz362 = CONVERT_OF(fbits, lbits)(zgsz3220, true);
    KILL(lbits)(&zgsz3220);
    KILL(sail_int)(&zgsz3219);
  }
  uint64_t zgaz363;
  {
    sail_int zgsz3217;
    CREATE(sail_int)(&zgsz3217);
    CONVERT_OF(sail_int, mach_int)(&zgsz3217, INT64_C(0));
    lbits zgsz3218;
    CREATE(lbits)(&zgsz3218);
    sailgen_zzeros_implicit(&zgsz3218, zgsz3217);
    zgaz363 = CONVERT_OF(fbits, lbits)(zgsz3218, true);
    KILL(lbits)(&zgsz3218);
    KILL(sail_int)(&zgsz3217);
  }
  uint64_t zgaz364;
  {
    sail_int zgsz3215;
    CREATE(sail_int)(&zgsz3215);
    CONVERT_OF(sail_int, mach_int)(&zgsz3215, INT64_C(8));
    lbits zgsz3216;
    CREATE(lbits)(&zgsz3216);
    sailgen_zzeros_implicit(&zgsz3216, zgsz3215);
    zgaz364 = CONVERT_OF(fbits, lbits)(zgsz3216, true);
    KILL(lbits)(&zgsz3216);
    KILL(sail_int)(&zgsz3215);
  }
  uint64_t zgaz365;
  {
    sail_int zgsz3212;
    CREATE(sail_int)(&zgsz3212);
    CONVERT_OF(sail_int, mach_int)(&zgsz3212, zcap_otype_width);
    sail_int zgsz3213;
    CREATE(sail_int)(&zgsz3213);
    CONVERT_OF(sail_int, mach_int)(&zgsz3213, zotype_unsealed);
    lbits zgsz3214;
    CREATE(lbits)(&zgsz3214);
    sailgen_to_bits(&zgsz3214, zgsz3212, zgsz3213);
    zgaz365 = CONVERT_OF(fbits, lbits)(zgsz3214, true);
    KILL(lbits)(&zgsz3214);
    KILL(sail_int)(&zgsz3213);
    KILL(sail_int)(&zgsz3212);
  }
  uint64_t zgaz366;
  {
    sail_int zgsz3210;
    CREATE(sail_int)(&zgsz3210);
    CONVERT_OF(sail_int, mach_int)(&zgsz3210, INT64_C(32));
    lbits zgsz3211;
    CREATE(lbits)(&zgsz3211);
    sailgen_zzeros_implicit(&zgsz3211, zgsz3210);
    zgaz366 = CONVERT_OF(fbits, lbits)(zgsz3211, true);
    KILL(lbits)(&zgsz3211);
    KILL(sail_int)(&zgsz3210);
  }
  struct zCapability zgsz334;
  zgsz334.zB = zgaz364;
  zgsz334.zE = zcap_reset_E;
  zgsz334.zT = zcap_reset_T;
  zgsz334.zaccess_system_regs = true;
  zgsz334.zaddress = zgaz366;
  zgsz334.zflag_cap_mode = false;
  zgsz334.zglobal = true;
  zgsz334.zinternal_E = true;
  zgsz334.zotype = zgaz365;
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
  zgsz334.zreserved = zgaz363;
  zgsz334.ztag = true;
  zgsz334.zuperms = zgaz362;
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
  uint64_t zgaz399;
  {
    bool zgaz367;
    zgaz367 = zcap.zpermit_set_CID;
    zgaz399 = sailgen_bool_to_bits(zgaz367);
  }
  uint64_t zgaz3100;
  {
    uint64_t zgaz397;
    {
      bool zgaz368;
      zgaz368 = zcap.zaccess_system_regs;
      zgaz397 = sailgen_bool_to_bits(zgaz368);
    }
    uint64_t zgaz398;
    {
      uint64_t zgaz395;
      {
        bool zgaz369;
        zgaz369 = zcap.zpermit_unseal;
        zgaz395 = sailgen_bool_to_bits(zgaz369);
      }
      uint64_t zgaz396;
      {
        uint64_t zgaz393;
        {
          bool zgaz370;
          zgaz370 = zcap.zpermit_cinvoke;
          zgaz393 = sailgen_bool_to_bits(zgaz370);
        }
        uint64_t zgaz394;
        {
          uint64_t zgaz391;
          {
            bool zgaz371;
            zgaz371 = zcap.zpermit_seal;
            zgaz391 = sailgen_bool_to_bits(zgaz371);
          }
          uint64_t zgaz392;
          {
            uint64_t zgaz389;
            {
              bool zgaz372;
              zgaz372 = zcap.zpermit_store_local_cap;
              zgaz389 = sailgen_bool_to_bits(zgaz372);
            }
            uint64_t zgaz390;
            {
              uint64_t zgaz387;
              {
                bool zgaz373;
                zgaz373 = zcap.zpermit_store_cap;
                zgaz387 = sailgen_bool_to_bits(zgaz373);
              }
              uint64_t zgaz388;
              {
                uint64_t zgaz385;
                {
                  bool zgaz374;
                  zgaz374 = zcap.zpermit_load_cap;
                  zgaz385 = sailgen_bool_to_bits(zgaz374);
                }
                uint64_t zgaz386;
                {
                  uint64_t zgaz383;
                  {
                    bool zgaz375;
                    zgaz375 = zcap.zpermit_store;
                    zgaz383 = sailgen_bool_to_bits(zgaz375);
                  }
                  uint64_t zgaz384;
                  {
                    uint64_t zgaz381;
                    {
                      bool zgaz376;
                      zgaz376 = zcap.zpermit_load;
                      zgaz381 = sailgen_bool_to_bits(zgaz376);
                    }
                    uint64_t zgaz382;
                    {
                      uint64_t zgaz379;
                      {
                        bool zgaz377;
                        zgaz377 = zcap.zpermit_execute;
                        zgaz379 = sailgen_bool_to_bits(zgaz377);
                      }
                      uint64_t zgaz380;
                      {
                        bool zgaz378;
                        zgaz378 = zcap.zglobal;
                        zgaz380 = sailgen_bool_to_bits(zgaz378);
                      }
                      {
                        lbits zgsz3221;
                        CREATE(lbits)(&zgsz3221);
                        CONVERT_OF(lbits, fbits)(&zgsz3221, zgaz379, UINT64_C(1) , true);
                        lbits zgsz3222;
                        CREATE(lbits)(&zgsz3222);
                        CONVERT_OF(lbits, fbits)(&zgsz3222, zgaz380, UINT64_C(1) , true);
                        lbits zgsz3223;
                        CREATE(lbits)(&zgsz3223);
                        append(&zgsz3223, zgsz3221, zgsz3222);
                        zgaz382 = CONVERT_OF(fbits, lbits)(zgsz3223, true);
                        KILL(lbits)(&zgsz3223);
                        KILL(lbits)(&zgsz3222);
                        KILL(lbits)(&zgsz3221);
                      }
                    }
                    {
                      lbits zgsz3224;
                      CREATE(lbits)(&zgsz3224);
                      CONVERT_OF(lbits, fbits)(&zgsz3224, zgaz381, UINT64_C(1) , true);
                      lbits zgsz3225;
                      CREATE(lbits)(&zgsz3225);
                      CONVERT_OF(lbits, fbits)(&zgsz3225, zgaz382, UINT64_C(2) , true);
                      lbits zgsz3226;
                      CREATE(lbits)(&zgsz3226);
                      append(&zgsz3226, zgsz3224, zgsz3225);
                      zgaz384 = CONVERT_OF(fbits, lbits)(zgsz3226, true);
                      KILL(lbits)(&zgsz3226);
                      KILL(lbits)(&zgsz3225);
                      KILL(lbits)(&zgsz3224);
                    }
                  }
                  {
                    lbits zgsz3227;
                    CREATE(lbits)(&zgsz3227);
                    CONVERT_OF(lbits, fbits)(&zgsz3227, zgaz383, UINT64_C(1) , true);
                    lbits zgsz3228;
                    CREATE(lbits)(&zgsz3228);
                    CONVERT_OF(lbits, fbits)(&zgsz3228, zgaz384, UINT64_C(3) , true);
                    lbits zgsz3229;
                    CREATE(lbits)(&zgsz3229);
                    append(&zgsz3229, zgsz3227, zgsz3228);
                    zgaz386 = CONVERT_OF(fbits, lbits)(zgsz3229, true);
                    KILL(lbits)(&zgsz3229);
                    KILL(lbits)(&zgsz3228);
                    KILL(lbits)(&zgsz3227);
                  }
                }
                {
                  lbits zgsz3230;
                  CREATE(lbits)(&zgsz3230);
                  CONVERT_OF(lbits, fbits)(&zgsz3230, zgaz385, UINT64_C(1) , true);
                  lbits zgsz3231;
                  CREATE(lbits)(&zgsz3231);
                  CONVERT_OF(lbits, fbits)(&zgsz3231, zgaz386, UINT64_C(4) , true);
                  lbits zgsz3232;
                  CREATE(lbits)(&zgsz3232);
                  append(&zgsz3232, zgsz3230, zgsz3231);
                  zgaz388 = CONVERT_OF(fbits, lbits)(zgsz3232, true);
                  KILL(lbits)(&zgsz3232);
                  KILL(lbits)(&zgsz3231);
                  KILL(lbits)(&zgsz3230);
                }
              }
              {
                lbits zgsz3233;
                CREATE(lbits)(&zgsz3233);
                CONVERT_OF(lbits, fbits)(&zgsz3233, zgaz387, UINT64_C(1) , true);
                lbits zgsz3234;
                CREATE(lbits)(&zgsz3234);
                CONVERT_OF(lbits, fbits)(&zgsz3234, zgaz388, UINT64_C(5) , true);
                lbits zgsz3235;
                CREATE(lbits)(&zgsz3235);
                append(&zgsz3235, zgsz3233, zgsz3234);
                zgaz390 = CONVERT_OF(fbits, lbits)(zgsz3235, true);
                KILL(lbits)(&zgsz3235);
                KILL(lbits)(&zgsz3234);
                KILL(lbits)(&zgsz3233);
              }
            }
            {
              lbits zgsz3236;
              CREATE(lbits)(&zgsz3236);
              CONVERT_OF(lbits, fbits)(&zgsz3236, zgaz389, UINT64_C(1) , true);
              lbits zgsz3237;
              CREATE(lbits)(&zgsz3237);
              CONVERT_OF(lbits, fbits)(&zgsz3237, zgaz390, UINT64_C(6) , true);
              lbits zgsz3238;
              CREATE(lbits)(&zgsz3238);
              append(&zgsz3238, zgsz3236, zgsz3237);
              zgaz392 = CONVERT_OF(fbits, lbits)(zgsz3238, true);
              KILL(lbits)(&zgsz3238);
              KILL(lbits)(&zgsz3237);
              KILL(lbits)(&zgsz3236);
            }
          }
          {
            lbits zgsz3239;
            CREATE(lbits)(&zgsz3239);
            CONVERT_OF(lbits, fbits)(&zgsz3239, zgaz391, UINT64_C(1) , true);
            lbits zgsz3240;
            CREATE(lbits)(&zgsz3240);
            CONVERT_OF(lbits, fbits)(&zgsz3240, zgaz392, UINT64_C(7) , true);
            lbits zgsz3241;
            CREATE(lbits)(&zgsz3241);
            append(&zgsz3241, zgsz3239, zgsz3240);
            zgaz394 = CONVERT_OF(fbits, lbits)(zgsz3241, true);
            KILL(lbits)(&zgsz3241);
            KILL(lbits)(&zgsz3240);
            KILL(lbits)(&zgsz3239);
          }
        }
        {
          lbits zgsz3242;
          CREATE(lbits)(&zgsz3242);
          CONVERT_OF(lbits, fbits)(&zgsz3242, zgaz393, UINT64_C(1) , true);
          lbits zgsz3243;
          CREATE(lbits)(&zgsz3243);
          CONVERT_OF(lbits, fbits)(&zgsz3243, zgaz394, UINT64_C(8) , true);
          lbits zgsz3244;
          CREATE(lbits)(&zgsz3244);
          append(&zgsz3244, zgsz3242, zgsz3243);
          zgaz396 = CONVERT_OF(fbits, lbits)(zgsz3244, true);
          KILL(lbits)(&zgsz3244);
          KILL(lbits)(&zgsz3243);
          KILL(lbits)(&zgsz3242);
        }
      }
      {
        lbits zgsz3245;
        CREATE(lbits)(&zgsz3245);
        CONVERT_OF(lbits, fbits)(&zgsz3245, zgaz395, UINT64_C(1) , true);
        lbits zgsz3246;
        CREATE(lbits)(&zgsz3246);
        CONVERT_OF(lbits, fbits)(&zgsz3246, zgaz396, UINT64_C(9) , true);
        lbits zgsz3247;
        CREATE(lbits)(&zgsz3247);
        append(&zgsz3247, zgsz3245, zgsz3246);
        zgaz398 = CONVERT_OF(fbits, lbits)(zgsz3247, true);
        KILL(lbits)(&zgsz3247);
        KILL(lbits)(&zgsz3246);
        KILL(lbits)(&zgsz3245);
      }
    }
    {
      lbits zgsz3248;
      CREATE(lbits)(&zgsz3248);
      CONVERT_OF(lbits, fbits)(&zgsz3248, zgaz397, UINT64_C(1) , true);
      lbits zgsz3249;
      CREATE(lbits)(&zgsz3249);
      CONVERT_OF(lbits, fbits)(&zgsz3249, zgaz398, UINT64_C(10) , true);
      lbits zgsz3250;
      CREATE(lbits)(&zgsz3250);
      append(&zgsz3250, zgsz3248, zgsz3249);
      zgaz3100 = CONVERT_OF(fbits, lbits)(zgsz3250, true);
      KILL(lbits)(&zgsz3250);
      KILL(lbits)(&zgsz3249);
      KILL(lbits)(&zgsz3248);
    }
  }
  {
    lbits zgsz3251;
    CREATE(lbits)(&zgsz3251);
    CONVERT_OF(lbits, fbits)(&zgsz3251, zgaz399, UINT64_C(1) , true);
    lbits zgsz3252;
    CREATE(lbits)(&zgsz3252);
    CONVERT_OF(lbits, fbits)(&zgsz3252, zgaz3100, UINT64_C(11) , true);
    lbits zgsz3253;
    CREATE(lbits)(&zgsz3253);
    append(&zgsz3253, zgsz3251, zgsz3252);
    zcbz317 = CONVERT_OF(fbits, lbits)(zgsz3253, true);
    KILL(lbits)(&zgsz3253);
    KILL(lbits)(&zgsz3252);
    KILL(lbits)(&zgsz3251);
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
    fbits zgaz3172;
    zgaz3172 = zc.zinternal_E;
    zinternal_E = sailgen_bit_to_bool(zgaz3172);
  }
  uint64_t zE;
  {
    sail_int zgsz3360;
    CREATE(sail_int)(&zgsz3360);
    CONVERT_OF(sail_int, mach_int)(&zgsz3360, INT64_C(6));
    lbits zgsz3361;
    CREATE(lbits)(&zgsz3361);
    sailgen_zzeros_implicit(&zgsz3361, zgsz3360);
    zE = CONVERT_OF(fbits, lbits)(zgsz3361, true);
    KILL(lbits)(&zgsz3361);
    KILL(sail_int)(&zgsz3360);
  }
  uint64_t zBs;
  {
    sail_int zgsz3358;
    CREATE(sail_int)(&zgsz3358);
    CONVERT_OF(sail_int, mach_int)(&zgsz3358, INT64_C(8));
    lbits zgsz3359;
    CREATE(lbits)(&zgsz3359);
    sailgen_zzeros_implicit(&zgsz3359, zgsz3358);
    zBs = CONVERT_OF(fbits, lbits)(zgsz3359, true);
    KILL(lbits)(&zgsz3359);
    KILL(sail_int)(&zgsz3358);
  }
  uint64_t zT;
  {
    int64_t zgaz3171;
    {
      sail_int zgsz3256;
      CREATE(sail_int)(&zgsz3256);
      CONVERT_OF(sail_int, mach_int)(&zgsz3256, INT64_C(8));
      sail_int zgsz3257;
      CREATE(sail_int)(&zgsz3257);
      CONVERT_OF(sail_int, mach_int)(&zgsz3257, INT64_C(2));
      sail_int zgsz3258;
      CREATE(sail_int)(&zgsz3258);
      sub_int(&zgsz3258, zgsz3256, zgsz3257);
      zgaz3171 = CONVERT_OF(mach_int, sail_int)(zgsz3258);
      KILL(sail_int)(&zgsz3258);
      KILL(sail_int)(&zgsz3257);
      KILL(sail_int)(&zgsz3256);
    }
    {
      sail_int zgsz3254;
      CREATE(sail_int)(&zgsz3254);
      CONVERT_OF(sail_int, mach_int)(&zgsz3254, zgaz3171);
      lbits zgsz3255;
      CREATE(lbits)(&zgsz3255);
      sailgen_zzeros_implicit(&zgsz3255, zgsz3254);
      zT = CONVERT_OF(fbits, lbits)(zgsz3255, true);
      KILL(lbits)(&zgsz3255);
      KILL(sail_int)(&zgsz3254);
    }
  }
  uint64_t zlenMSBs;
  {
    sail_int zgsz3356;
    CREATE(sail_int)(&zgsz3356);
    CONVERT_OF(sail_int, mach_int)(&zgsz3356, INT64_C(2));
    lbits zgsz3357;
    CREATE(lbits)(&zgsz3357);
    sailgen_zzeros_implicit(&zgsz3357, zgsz3356);
    zlenMSBs = CONVERT_OF(fbits, lbits)(zgsz3357, true);
    KILL(lbits)(&zgsz3357);
    KILL(sail_int)(&zgsz3356);
  }
  {
    unit zgsz336;
    if (zinternal_E) {
      {
        uint64_t zgaz3105;
        {
          uint64_t zgaz3101;
          zgaz3101 = zc.zT;
          int64_t zgaz3102;
          {
            sail_int zgsz3263;
            CREATE(sail_int)(&zgsz3263);
            CONVERT_OF(sail_int, mach_int)(&zgsz3263, zinternal_E_take_bits);
            sail_int zgsz3264;
            CREATE(sail_int)(&zgsz3264);
            CONVERT_OF(sail_int, mach_int)(&zgsz3264, INT64_C(1));
            sail_int zgsz3265;
            CREATE(sail_int)(&zgsz3265);
            sub_int(&zgsz3265, zgsz3263, zgsz3264);
            zgaz3102 = CONVERT_OF(mach_int, sail_int)(zgsz3265);
            KILL(sail_int)(&zgsz3265);
            KILL(sail_int)(&zgsz3264);
            KILL(sail_int)(&zgsz3263);
          }
          {
            lbits zgsz3259;
            CREATE(lbits)(&zgsz3259);
            CONVERT_OF(lbits, fbits)(&zgsz3259, zgaz3101, UINT64_C(6) , true);
            sail_int zgsz3260;
            CREATE(sail_int)(&zgsz3260);
            CONVERT_OF(sail_int, mach_int)(&zgsz3260, zgaz3102);
            sail_int zgsz3261;
            CREATE(sail_int)(&zgsz3261);
            CONVERT_OF(sail_int, mach_int)(&zgsz3261, INT64_C(0));
            lbits zgsz3262;
            CREATE(lbits)(&zgsz3262);
            vector_subrange_lbits(&zgsz3262, zgsz3259, zgsz3260, zgsz3261);
            zgaz3105 = CONVERT_OF(fbits, lbits)(zgsz3262, true);
            KILL(lbits)(&zgsz3262);
            KILL(sail_int)(&zgsz3261);
            KILL(sail_int)(&zgsz3260);
            KILL(lbits)(&zgsz3259);
          }
        }
        uint64_t zgaz3106;
        {
          uint64_t zgaz3103;
          zgaz3103 = zc.zB;
          int64_t zgaz3104;
          {
            sail_int zgsz3270;
            CREATE(sail_int)(&zgsz3270);
            CONVERT_OF(sail_int, mach_int)(&zgsz3270, zinternal_E_take_bits);
            sail_int zgsz3271;
            CREATE(sail_int)(&zgsz3271);
            CONVERT_OF(sail_int, mach_int)(&zgsz3271, INT64_C(1));
            sail_int zgsz3272;
            CREATE(sail_int)(&zgsz3272);
            sub_int(&zgsz3272, zgsz3270, zgsz3271);
            zgaz3104 = CONVERT_OF(mach_int, sail_int)(zgsz3272);
            KILL(sail_int)(&zgsz3272);
            KILL(sail_int)(&zgsz3271);
            KILL(sail_int)(&zgsz3270);
          }
          {
            lbits zgsz3266;
            CREATE(lbits)(&zgsz3266);
            CONVERT_OF(lbits, fbits)(&zgsz3266, zgaz3103, UINT64_C(8) , true);
            sail_int zgsz3267;
            CREATE(sail_int)(&zgsz3267);
            CONVERT_OF(sail_int, mach_int)(&zgsz3267, zgaz3104);
            sail_int zgsz3268;
            CREATE(sail_int)(&zgsz3268);
            CONVERT_OF(sail_int, mach_int)(&zgsz3268, INT64_C(0));
            lbits zgsz3269;
            CREATE(lbits)(&zgsz3269);
            vector_subrange_lbits(&zgsz3269, zgsz3266, zgsz3267, zgsz3268);
            zgaz3106 = CONVERT_OF(fbits, lbits)(zgsz3269, true);
            KILL(lbits)(&zgsz3269);
            KILL(sail_int)(&zgsz3268);
            KILL(sail_int)(&zgsz3267);
            KILL(lbits)(&zgsz3266);
          }
        }
        {
          lbits zgsz3273;
          CREATE(lbits)(&zgsz3273);
          CONVERT_OF(lbits, fbits)(&zgsz3273, zgaz3105, UINT64_C(3) , true);
          lbits zgsz3274;
          CREATE(lbits)(&zgsz3274);
          CONVERT_OF(lbits, fbits)(&zgsz3274, zgaz3106, UINT64_C(3) , true);
          lbits zgsz3275;
          CREATE(lbits)(&zgsz3275);
          append(&zgsz3275, zgsz3273, zgsz3274);
          zE = CONVERT_OF(fbits, lbits)(zgsz3275, true);
          KILL(lbits)(&zgsz3275);
          KILL(lbits)(&zgsz3274);
          KILL(lbits)(&zgsz3273);
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
        uint64_t zgaz3109;
        {
          uint64_t zgaz3107;
          zgaz3107 = zc.zT;
          int64_t zgaz3108;
          {
            sail_int zgsz3280;
            CREATE(sail_int)(&zgsz3280);
            CONVERT_OF(sail_int, mach_int)(&zgsz3280, zcap_mantissa_width);
            sail_int zgsz3281;
            CREATE(sail_int)(&zgsz3281);
            CONVERT_OF(sail_int, mach_int)(&zgsz3281, INT64_C(3));
            sail_int zgsz3282;
            CREATE(sail_int)(&zgsz3282);
            sub_int(&zgsz3282, zgsz3280, zgsz3281);
            zgaz3108 = CONVERT_OF(mach_int, sail_int)(zgsz3282);
            KILL(sail_int)(&zgsz3282);
            KILL(sail_int)(&zgsz3281);
            KILL(sail_int)(&zgsz3280);
          }
          {
            lbits zgsz3276;
            CREATE(lbits)(&zgsz3276);
            CONVERT_OF(lbits, fbits)(&zgsz3276, zgaz3107, UINT64_C(6) , true);
            sail_int zgsz3277;
            CREATE(sail_int)(&zgsz3277);
            CONVERT_OF(sail_int, mach_int)(&zgsz3277, zgaz3108);
            sail_int zgsz3278;
            CREATE(sail_int)(&zgsz3278);
            CONVERT_OF(sail_int, mach_int)(&zgsz3278, INT64_C(3));
            lbits zgsz3279;
            CREATE(lbits)(&zgsz3279);
            vector_subrange_lbits(&zgsz3279, zgsz3276, zgsz3277, zgsz3278);
            zgaz3109 = CONVERT_OF(fbits, lbits)(zgsz3279, true);
            KILL(lbits)(&zgsz3279);
            KILL(sail_int)(&zgsz3278);
            KILL(sail_int)(&zgsz3277);
            KILL(lbits)(&zgsz3276);
          }
        }
        uint64_t zgaz3110;
        {
          sail_int zgsz3286;
          CREATE(sail_int)(&zgsz3286);
          CONVERT_OF(sail_int, mach_int)(&zgsz3286, zinternal_E_take_bits);
          lbits zgsz3287;
          CREATE(lbits)(&zgsz3287);
          sailgen_zzeros_implicit(&zgsz3287, zgsz3286);
          zgaz3110 = CONVERT_OF(fbits, lbits)(zgsz3287, true);
          KILL(lbits)(&zgsz3287);
          KILL(sail_int)(&zgsz3286);
        }
        {
          lbits zgsz3283;
          CREATE(lbits)(&zgsz3283);
          CONVERT_OF(lbits, fbits)(&zgsz3283, zgaz3109, UINT64_C(3) , true);
          lbits zgsz3284;
          CREATE(lbits)(&zgsz3284);
          CONVERT_OF(lbits, fbits)(&zgsz3284, zgaz3110, UINT64_C(3) , true);
          lbits zgsz3285;
          CREATE(lbits)(&zgsz3285);
          append(&zgsz3285, zgsz3283, zgsz3284);
          zT = CONVERT_OF(fbits, lbits)(zgsz3285, true);
          KILL(lbits)(&zgsz3285);
          KILL(lbits)(&zgsz3284);
          KILL(lbits)(&zgsz3283);
        }
        unit zgsz339;
        zgsz339 = UNIT;
      }
      uint64_t zgaz3113;
      {
        uint64_t zgaz3111;
        zgaz3111 = zc.zB;
        int64_t zgaz3112;
        {
          sail_int zgsz3292;
          CREATE(sail_int)(&zgsz3292);
          CONVERT_OF(sail_int, mach_int)(&zgsz3292, zcap_mantissa_width);
          sail_int zgsz3293;
          CREATE(sail_int)(&zgsz3293);
          CONVERT_OF(sail_int, mach_int)(&zgsz3293, INT64_C(1));
          sail_int zgsz3294;
          CREATE(sail_int)(&zgsz3294);
          sub_int(&zgsz3294, zgsz3292, zgsz3293);
          zgaz3112 = CONVERT_OF(mach_int, sail_int)(zgsz3294);
          KILL(sail_int)(&zgsz3294);
          KILL(sail_int)(&zgsz3293);
          KILL(sail_int)(&zgsz3292);
        }
        {
          lbits zgsz3288;
          CREATE(lbits)(&zgsz3288);
          CONVERT_OF(lbits, fbits)(&zgsz3288, zgaz3111, UINT64_C(8) , true);
          sail_int zgsz3289;
          CREATE(sail_int)(&zgsz3289);
          CONVERT_OF(sail_int, mach_int)(&zgsz3289, zgaz3112);
          sail_int zgsz3290;
          CREATE(sail_int)(&zgsz3290);
          CONVERT_OF(sail_int, mach_int)(&zgsz3290, INT64_C(3));
          lbits zgsz3291;
          CREATE(lbits)(&zgsz3291);
          vector_subrange_lbits(&zgsz3291, zgsz3288, zgsz3289, zgsz3290);
          zgaz3113 = CONVERT_OF(fbits, lbits)(zgsz3291, true);
          KILL(lbits)(&zgsz3291);
          KILL(sail_int)(&zgsz3290);
          KILL(sail_int)(&zgsz3289);
          KILL(lbits)(&zgsz3288);
        }
      }
      uint64_t zgaz3114;
      {
        sail_int zgsz3298;
        CREATE(sail_int)(&zgsz3298);
        CONVERT_OF(sail_int, mach_int)(&zgsz3298, zinternal_E_take_bits);
        lbits zgsz3299;
        CREATE(lbits)(&zgsz3299);
        sailgen_zzeros_implicit(&zgsz3299, zgsz3298);
        zgaz3114 = CONVERT_OF(fbits, lbits)(zgsz3299, true);
        KILL(lbits)(&zgsz3299);
        KILL(sail_int)(&zgsz3298);
      }
      {
        lbits zgsz3295;
        CREATE(lbits)(&zgsz3295);
        CONVERT_OF(lbits, fbits)(&zgsz3295, zgaz3113, UINT64_C(5) , true);
        lbits zgsz3296;
        CREATE(lbits)(&zgsz3296);
        CONVERT_OF(lbits, fbits)(&zgsz3296, zgaz3114, UINT64_C(3) , true);
        lbits zgsz3297;
        CREATE(lbits)(&zgsz3297);
        append(&zgsz3297, zgsz3295, zgsz3296);
        zBs = CONVERT_OF(fbits, lbits)(zgsz3297, true);
        KILL(lbits)(&zgsz3297);
        KILL(lbits)(&zgsz3296);
        KILL(lbits)(&zgsz3295);
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
    bool zgaz3170;
    {
      uint64_t zgaz3169;
      {
        int64_t zgaz3168;
        {
          sail_int zgsz3304;
          CREATE(sail_int)(&zgsz3304);
          CONVERT_OF(sail_int, mach_int)(&zgsz3304, zcap_mantissa_width);
          sail_int zgsz3305;
          CREATE(sail_int)(&zgsz3305);
          CONVERT_OF(sail_int, mach_int)(&zgsz3305, INT64_C(3));
          sail_int zgsz3306;
          CREATE(sail_int)(&zgsz3306);
          sub_int(&zgsz3306, zgsz3304, zgsz3305);
          zgaz3168 = CONVERT_OF(mach_int, sail_int)(zgsz3306);
          KILL(sail_int)(&zgsz3306);
          KILL(sail_int)(&zgsz3305);
          KILL(sail_int)(&zgsz3304);
        }
        {
          lbits zgsz3300;
          CREATE(lbits)(&zgsz3300);
          CONVERT_OF(lbits, fbits)(&zgsz3300, zBs, UINT64_C(8) , true);
          sail_int zgsz3301;
          CREATE(sail_int)(&zgsz3301);
          CONVERT_OF(sail_int, mach_int)(&zgsz3301, zgaz3168);
          sail_int zgsz3302;
          CREATE(sail_int)(&zgsz3302);
          CONVERT_OF(sail_int, mach_int)(&zgsz3302, INT64_C(0));
          lbits zgsz3303;
          CREATE(lbits)(&zgsz3303);
          vector_subrange_lbits(&zgsz3303, zgsz3300, zgsz3301, zgsz3302);
          zgaz3169 = CONVERT_OF(fbits, lbits)(zgsz3303, true);
          KILL(lbits)(&zgsz3303);
          KILL(sail_int)(&zgsz3302);
          KILL(sail_int)(&zgsz3301);
          KILL(lbits)(&zgsz3300);
        }
      }
      {
        lbits zgsz3307;
        CREATE(lbits)(&zgsz3307);
        CONVERT_OF(lbits, fbits)(&zgsz3307, zT, UINT64_C(6) , true);
        lbits zgsz3308;
        CREATE(lbits)(&zgsz3308);
        CONVERT_OF(lbits, fbits)(&zgsz3308, zgaz3169, UINT64_C(6) , true);
        zgaz3170 = sailgen_z8operatorz0zI_uz9(zgsz3307, zgsz3308);
        KILL(lbits)(&zgsz3308);
        KILL(lbits)(&zgsz3307);
      }
    }
    if (zgaz3170) {  zcarry_out = UINT64_C(0b01);  } else {  zcarry_out = UINT64_C(0b00);  }
  }
  uint64_t zTtop2;
  {
    uint64_t zgaz3167;
    {
      uint64_t zgaz3166;
      {
        int64_t zgaz3164;
        {
          sail_int zgsz3316;
          CREATE(sail_int)(&zgsz3316);
          CONVERT_OF(sail_int, mach_int)(&zgsz3316, zcap_mantissa_width);
          sail_int zgsz3317;
          CREATE(sail_int)(&zgsz3317);
          CONVERT_OF(sail_int, mach_int)(&zgsz3317, INT64_C(1));
          sail_int zgsz3318;
          CREATE(sail_int)(&zgsz3318);
          sub_int(&zgsz3318, zgsz3316, zgsz3317);
          zgaz3164 = CONVERT_OF(mach_int, sail_int)(zgsz3318);
          KILL(sail_int)(&zgsz3318);
          KILL(sail_int)(&zgsz3317);
          KILL(sail_int)(&zgsz3316);
        }
        int64_t zgaz3165;
        {
          sail_int zgsz3313;
          CREATE(sail_int)(&zgsz3313);
          CONVERT_OF(sail_int, mach_int)(&zgsz3313, zcap_mantissa_width);
          sail_int zgsz3314;
          CREATE(sail_int)(&zgsz3314);
          CONVERT_OF(sail_int, mach_int)(&zgsz3314, INT64_C(2));
          sail_int zgsz3315;
          CREATE(sail_int)(&zgsz3315);
          sub_int(&zgsz3315, zgsz3313, zgsz3314);
          zgaz3165 = CONVERT_OF(mach_int, sail_int)(zgsz3315);
          KILL(sail_int)(&zgsz3315);
          KILL(sail_int)(&zgsz3314);
          KILL(sail_int)(&zgsz3313);
        }
        {
          lbits zgsz3309;
          CREATE(lbits)(&zgsz3309);
          CONVERT_OF(lbits, fbits)(&zgsz3309, zBs, UINT64_C(8) , true);
          sail_int zgsz3310;
          CREATE(sail_int)(&zgsz3310);
          CONVERT_OF(sail_int, mach_int)(&zgsz3310, zgaz3164);
          sail_int zgsz3311;
          CREATE(sail_int)(&zgsz3311);
          CONVERT_OF(sail_int, mach_int)(&zgsz3311, zgaz3165);
          lbits zgsz3312;
          CREATE(lbits)(&zgsz3312);
          vector_subrange_lbits(&zgsz3312, zgsz3309, zgsz3310, zgsz3311);
          zgaz3166 = CONVERT_OF(fbits, lbits)(zgsz3312, true);
          KILL(lbits)(&zgsz3312);
          KILL(sail_int)(&zgsz3311);
          KILL(sail_int)(&zgsz3310);
          KILL(lbits)(&zgsz3309);
        }
      }
      {
        lbits zgsz3319;
        CREATE(lbits)(&zgsz3319);
        CONVERT_OF(lbits, fbits)(&zgsz3319, zgaz3166, UINT64_C(2) , true);
        lbits zgsz3320;
        CREATE(lbits)(&zgsz3320);
        CONVERT_OF(lbits, fbits)(&zgsz3320, zlenMSBs, UINT64_C(2) , true);
        lbits zgsz3321;
        CREATE(lbits)(&zgsz3321);
        add_bits(&zgsz3321, zgsz3319, zgsz3320);
        zgaz3167 = CONVERT_OF(fbits, lbits)(zgsz3321, true);
        KILL(lbits)(&zgsz3321);
        KILL(lbits)(&zgsz3320);
        KILL(lbits)(&zgsz3319);
      }
    }
    {
      lbits zgsz3322;
      CREATE(lbits)(&zgsz3322);
      CONVERT_OF(lbits, fbits)(&zgsz3322, zgaz3167, UINT64_C(2) , true);
      lbits zgsz3323;
      CREATE(lbits)(&zgsz3323);
      CONVERT_OF(lbits, fbits)(&zgsz3323, zcarry_out, UINT64_C(2) , true);
      lbits zgsz3324;
      CREATE(lbits)(&zgsz3324);
      add_bits(&zgsz3324, zgsz3322, zgsz3323);
      zTtop2 = CONVERT_OF(fbits, lbits)(zgsz3324, true);
      KILL(lbits)(&zgsz3324);
      KILL(lbits)(&zgsz3323);
      KILL(lbits)(&zgsz3322);
    }
  }
  struct zCapability zgaz3163;
  {
    uint64_t zgaz3119;
    {
      bool zgaz3115;
      {
        sail_int zgsz3325;
        CREATE(sail_int)(&zgsz3325);
        CONVERT_OF(sail_int, mach_int)(&zgsz3325, zcap_uperms_width);
        sail_int zgsz3326;
        CREATE(sail_int)(&zgsz3326);
        CONVERT_OF(sail_int, mach_int)(&zgsz3326, INT64_C(0));
        zgaz3115 = gt(zgsz3325, zgsz3326);
        KILL(sail_int)(&zgsz3326);
        KILL(sail_int)(&zgsz3325);
      }
      zgaz3119 = UINT64_C(0);
    }
    bool zgaz3122;
    {
      fbits zgaz3121;
      {
        uint64_t zgaz3120;
        zgaz3120 = zc.zperms;
        {
          lbits zgsz3327;
          CREATE(lbits)(&zgsz3327);
          CONVERT_OF(lbits, fbits)(&zgsz3327, zgaz3120, UINT64_C(12) , true);
          sail_int zgsz3328;
          CREATE(sail_int)(&zgsz3328);
          CONVERT_OF(sail_int, mach_int)(&zgsz3328, INT64_C(11));
          zgaz3121 = bitvector_access(zgsz3327, zgsz3328);
          KILL(sail_int)(&zgsz3328);
          KILL(lbits)(&zgsz3327);
        }
      }
      zgaz3122 = sailgen_bit_to_bool(zgaz3121);
    }
    bool zgaz3125;
    {
      fbits zgaz3124;
      {
        uint64_t zgaz3123;
        zgaz3123 = zc.zperms;
        {
          lbits zgsz3329;
          CREATE(lbits)(&zgsz3329);
          CONVERT_OF(lbits, fbits)(&zgsz3329, zgaz3123, UINT64_C(12) , true);
          sail_int zgsz3330;
          CREATE(sail_int)(&zgsz3330);
          CONVERT_OF(sail_int, mach_int)(&zgsz3330, INT64_C(10));
          zgaz3124 = bitvector_access(zgsz3329, zgsz3330);
          KILL(sail_int)(&zgsz3330);
          KILL(lbits)(&zgsz3329);
        }
      }
      zgaz3125 = sailgen_bit_to_bool(zgaz3124);
    }
    bool zgaz3128;
    {
      fbits zgaz3127;
      {
        uint64_t zgaz3126;
        zgaz3126 = zc.zperms;
        {
          lbits zgsz3331;
          CREATE(lbits)(&zgsz3331);
          CONVERT_OF(lbits, fbits)(&zgsz3331, zgaz3126, UINT64_C(12) , true);
          sail_int zgsz3332;
          CREATE(sail_int)(&zgsz3332);
          CONVERT_OF(sail_int, mach_int)(&zgsz3332, INT64_C(9));
          zgaz3127 = bitvector_access(zgsz3331, zgsz3332);
          KILL(sail_int)(&zgsz3332);
          KILL(lbits)(&zgsz3331);
        }
      }
      zgaz3128 = sailgen_bit_to_bool(zgaz3127);
    }
    bool zgaz3131;
    {
      fbits zgaz3130;
      {
        uint64_t zgaz3129;
        zgaz3129 = zc.zperms;
        {
          lbits zgsz3333;
          CREATE(lbits)(&zgsz3333);
          CONVERT_OF(lbits, fbits)(&zgsz3333, zgaz3129, UINT64_C(12) , true);
          sail_int zgsz3334;
          CREATE(sail_int)(&zgsz3334);
          CONVERT_OF(sail_int, mach_int)(&zgsz3334, INT64_C(8));
          zgaz3130 = bitvector_access(zgsz3333, zgsz3334);
          KILL(sail_int)(&zgsz3334);
          KILL(lbits)(&zgsz3333);
        }
      }
      zgaz3131 = sailgen_bit_to_bool(zgaz3130);
    }
    bool zgaz3134;
    {
      fbits zgaz3133;
      {
        uint64_t zgaz3132;
        zgaz3132 = zc.zperms;
        {
          lbits zgsz3335;
          CREATE(lbits)(&zgsz3335);
          CONVERT_OF(lbits, fbits)(&zgsz3335, zgaz3132, UINT64_C(12) , true);
          sail_int zgsz3336;
          CREATE(sail_int)(&zgsz3336);
          CONVERT_OF(sail_int, mach_int)(&zgsz3336, INT64_C(7));
          zgaz3133 = bitvector_access(zgsz3335, zgsz3336);
          KILL(sail_int)(&zgsz3336);
          KILL(lbits)(&zgsz3335);
        }
      }
      zgaz3134 = sailgen_bit_to_bool(zgaz3133);
    }
    bool zgaz3137;
    {
      fbits zgaz3136;
      {
        uint64_t zgaz3135;
        zgaz3135 = zc.zperms;
        {
          lbits zgsz3337;
          CREATE(lbits)(&zgsz3337);
          CONVERT_OF(lbits, fbits)(&zgsz3337, zgaz3135, UINT64_C(12) , true);
          sail_int zgsz3338;
          CREATE(sail_int)(&zgsz3338);
          CONVERT_OF(sail_int, mach_int)(&zgsz3338, INT64_C(6));
          zgaz3136 = bitvector_access(zgsz3337, zgsz3338);
          KILL(sail_int)(&zgsz3338);
          KILL(lbits)(&zgsz3337);
        }
      }
      zgaz3137 = sailgen_bit_to_bool(zgaz3136);
    }
    bool zgaz3140;
    {
      fbits zgaz3139;
      {
        uint64_t zgaz3138;
        zgaz3138 = zc.zperms;
        {
          lbits zgsz3339;
          CREATE(lbits)(&zgsz3339);
          CONVERT_OF(lbits, fbits)(&zgsz3339, zgaz3138, UINT64_C(12) , true);
          sail_int zgsz3340;
          CREATE(sail_int)(&zgsz3340);
          CONVERT_OF(sail_int, mach_int)(&zgsz3340, INT64_C(5));
          zgaz3139 = bitvector_access(zgsz3339, zgsz3340);
          KILL(sail_int)(&zgsz3340);
          KILL(lbits)(&zgsz3339);
        }
      }
      zgaz3140 = sailgen_bit_to_bool(zgaz3139);
    }
    bool zgaz3143;
    {
      fbits zgaz3142;
      {
        uint64_t zgaz3141;
        zgaz3141 = zc.zperms;
        {
          lbits zgsz3341;
          CREATE(lbits)(&zgsz3341);
          CONVERT_OF(lbits, fbits)(&zgsz3341, zgaz3141, UINT64_C(12) , true);
          sail_int zgsz3342;
          CREATE(sail_int)(&zgsz3342);
          CONVERT_OF(sail_int, mach_int)(&zgsz3342, INT64_C(4));
          zgaz3142 = bitvector_access(zgsz3341, zgsz3342);
          KILL(sail_int)(&zgsz3342);
          KILL(lbits)(&zgsz3341);
        }
      }
      zgaz3143 = sailgen_bit_to_bool(zgaz3142);
    }
    bool zgaz3146;
    {
      fbits zgaz3145;
      {
        uint64_t zgaz3144;
        zgaz3144 = zc.zperms;
        {
          lbits zgsz3343;
          CREATE(lbits)(&zgsz3343);
          CONVERT_OF(lbits, fbits)(&zgsz3343, zgaz3144, UINT64_C(12) , true);
          sail_int zgsz3344;
          CREATE(sail_int)(&zgsz3344);
          CONVERT_OF(sail_int, mach_int)(&zgsz3344, INT64_C(3));
          zgaz3145 = bitvector_access(zgsz3343, zgsz3344);
          KILL(sail_int)(&zgsz3344);
          KILL(lbits)(&zgsz3343);
        }
      }
      zgaz3146 = sailgen_bit_to_bool(zgaz3145);
    }
    bool zgaz3149;
    {
      fbits zgaz3148;
      {
        uint64_t zgaz3147;
        zgaz3147 = zc.zperms;
        {
          lbits zgsz3345;
          CREATE(lbits)(&zgsz3345);
          CONVERT_OF(lbits, fbits)(&zgsz3345, zgaz3147, UINT64_C(12) , true);
          sail_int zgsz3346;
          CREATE(sail_int)(&zgsz3346);
          CONVERT_OF(sail_int, mach_int)(&zgsz3346, INT64_C(2));
          zgaz3148 = bitvector_access(zgsz3345, zgsz3346);
          KILL(sail_int)(&zgsz3346);
          KILL(lbits)(&zgsz3345);
        }
      }
      zgaz3149 = sailgen_bit_to_bool(zgaz3148);
    }
    bool zgaz3152;
    {
      fbits zgaz3151;
      {
        uint64_t zgaz3150;
        zgaz3150 = zc.zperms;
        {
          lbits zgsz3347;
          CREATE(lbits)(&zgsz3347);
          CONVERT_OF(lbits, fbits)(&zgsz3347, zgaz3150, UINT64_C(12) , true);
          sail_int zgsz3348;
          CREATE(sail_int)(&zgsz3348);
          CONVERT_OF(sail_int, mach_int)(&zgsz3348, INT64_C(1));
          zgaz3151 = bitvector_access(zgsz3347, zgsz3348);
          KILL(sail_int)(&zgsz3348);
          KILL(lbits)(&zgsz3347);
        }
      }
      zgaz3152 = sailgen_bit_to_bool(zgaz3151);
    }
    bool zgaz3155;
    {
      fbits zgaz3154;
      {
        uint64_t zgaz3153;
        zgaz3153 = zc.zperms;
        {
          lbits zgsz3349;
          CREATE(lbits)(&zgsz3349);
          CONVERT_OF(lbits, fbits)(&zgsz3349, zgaz3153, UINT64_C(12) , true);
          sail_int zgsz3350;
          CREATE(sail_int)(&zgsz3350);
          CONVERT_OF(sail_int, mach_int)(&zgsz3350, INT64_C(0));
          zgaz3154 = bitvector_access(zgsz3349, zgsz3350);
          KILL(sail_int)(&zgsz3350);
          KILL(lbits)(&zgsz3349);
        }
      }
      zgaz3155 = sailgen_bit_to_bool(zgaz3154);
    }
    uint64_t zgaz3156;
    zgaz3156 = zc.zreserved;
    bool zgaz3159;
    {
      fbits zgaz3158;
      {
        uint64_t zgaz3157;
        zgaz3157 = zc.zflags;
        {
          lbits zgsz3351;
          CREATE(lbits)(&zgsz3351);
          CONVERT_OF(lbits, fbits)(&zgsz3351, zgaz3157, UINT64_C(1) , true);
          sail_int zgsz3352;
          CREATE(sail_int)(&zgsz3352);
          CONVERT_OF(sail_int, mach_int)(&zgsz3352, INT64_C(0));
          zgaz3158 = bitvector_access(zgsz3351, zgsz3352);
          KILL(sail_int)(&zgsz3352);
          KILL(lbits)(&zgsz3351);
        }
      }
      zgaz3159 = sailgen_bit_to_bool(zgaz3158);
    }
    uint64_t zgaz3160;
    {
      lbits zgsz3353;
      CREATE(lbits)(&zgsz3353);
      CONVERT_OF(lbits, fbits)(&zgsz3353, zTtop2, UINT64_C(2) , true);
      lbits zgsz3354;
      CREATE(lbits)(&zgsz3354);
      CONVERT_OF(lbits, fbits)(&zgsz3354, zT, UINT64_C(6) , true);
      lbits zgsz3355;
      CREATE(lbits)(&zgsz3355);
      append(&zgsz3355, zgsz3353, zgsz3354);
      zgaz3160 = CONVERT_OF(fbits, lbits)(zgsz3355, true);
      KILL(lbits)(&zgsz3355);
      KILL(lbits)(&zgsz3354);
      KILL(lbits)(&zgsz3353);
    }
    uint64_t zgaz3161;
    zgaz3161 = zc.zotype;
    uint64_t zgaz3162;
    zgaz3162 = zc.zaddress;
    struct zCapability zgsz342;
    zgsz342.zB = zBs;
    zgsz342.zE = zE;
    zgsz342.zT = zgaz3160;
    zgsz342.zaccess_system_regs = zgaz3125;
    zgsz342.zaddress = zgaz3162;
    zgsz342.zflag_cap_mode = zgaz3159;
    zgsz342.zglobal = zgaz3155;
    zgsz342.zinternal_E = zinternal_E;
    zgsz342.zotype = zgaz3161;
    zgsz342.zpermit_cinvoke = zgaz3131;
    zgsz342.zpermit_execute = zgaz3152;
    zgsz342.zpermit_load = zgaz3149;
    zgsz342.zpermit_load_cap = zgaz3143;
    zgsz342.zpermit_seal = zgaz3134;
    zgsz342.zpermit_set_CID = zgaz3122;
    zgsz342.zpermit_store = zgaz3146;
    zgsz342.zpermit_store_cap = zgaz3140;
    zgsz342.zpermit_store_local_cap = zgaz3137;
    zgsz342.zpermit_unseal = zgaz3128;
    zgsz342.zreserved = zgaz3156;
    zgsz342.ztag = zt;
    zgsz342.zuperms = zgaz3119;
    zgaz3163 = zgsz342;
  }
  zcbz318 = zgaz3163;
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

static struct zCapability sailgen_capBitsToCapability(bool, uint64_t);

static struct zCapability sailgen_capBitsToCapability(bool zt, uint64_t zc)
{
  __label__ end_function_90, end_block_exception_91;

  struct zCapability zcbz319;
  struct zEncCapability zgaz3173;
  zgaz3173 = sailgen_capBitsToEncCapability(zc);
  zcbz319 = sailgen_encCapabilityToCapability(zt, zgaz3173);

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
    uint64_t zgaz3200;
    zgaz3200 = zcap.zT;
    int64_t zgaz3201;
    {
      sail_int zgsz3366;
      CREATE(sail_int)(&zgsz3366);
      CONVERT_OF(sail_int, mach_int)(&zgsz3366, zcap_mantissa_width);
      sail_int zgsz3367;
      CREATE(sail_int)(&zgsz3367);
      CONVERT_OF(sail_int, mach_int)(&zgsz3367, INT64_C(3));
      sail_int zgsz3368;
      CREATE(sail_int)(&zgsz3368);
      sub_int(&zgsz3368, zgsz3366, zgsz3367);
      zgaz3201 = CONVERT_OF(mach_int, sail_int)(zgsz3368);
      KILL(sail_int)(&zgsz3368);
      KILL(sail_int)(&zgsz3367);
      KILL(sail_int)(&zgsz3366);
    }
    {
      lbits zgsz3362;
      CREATE(lbits)(&zgsz3362);
      CONVERT_OF(lbits, fbits)(&zgsz3362, zgaz3200, UINT64_C(8) , true);
      sail_int zgsz3363;
      CREATE(sail_int)(&zgsz3363);
      CONVERT_OF(sail_int, mach_int)(&zgsz3363, zgaz3201);
      sail_int zgsz3364;
      CREATE(sail_int)(&zgsz3364);
      CONVERT_OF(sail_int, mach_int)(&zgsz3364, zinternal_E_take_bits);
      lbits zgsz3365;
      CREATE(lbits)(&zgsz3365);
      vector_subrange_lbits(&zgsz3365, zgsz3362, zgsz3363, zgsz3364);
      zt_hi = CONVERT_OF(fbits, lbits)(zgsz3365, true);
      KILL(lbits)(&zgsz3365);
      KILL(sail_int)(&zgsz3364);
      KILL(sail_int)(&zgsz3363);
      KILL(lbits)(&zgsz3362);
    }
  }
  uint64_t zt_lo;
  {
    uint64_t zgaz3198;
    zgaz3198 = zcap.zT;
    int64_t zgaz3199;
    {
      sail_int zgsz3373;
      CREATE(sail_int)(&zgsz3373);
      CONVERT_OF(sail_int, mach_int)(&zgsz3373, zinternal_E_take_bits);
      sail_int zgsz3374;
      CREATE(sail_int)(&zgsz3374);
      CONVERT_OF(sail_int, mach_int)(&zgsz3374, INT64_C(1));
      sail_int zgsz3375;
      CREATE(sail_int)(&zgsz3375);
      sub_int(&zgsz3375, zgsz3373, zgsz3374);
      zgaz3199 = CONVERT_OF(mach_int, sail_int)(zgsz3375);
      KILL(sail_int)(&zgsz3375);
      KILL(sail_int)(&zgsz3374);
      KILL(sail_int)(&zgsz3373);
    }
    {
      lbits zgsz3369;
      CREATE(lbits)(&zgsz3369);
      CONVERT_OF(lbits, fbits)(&zgsz3369, zgaz3198, UINT64_C(8) , true);
      sail_int zgsz3370;
      CREATE(sail_int)(&zgsz3370);
      CONVERT_OF(sail_int, mach_int)(&zgsz3370, zgaz3199);
      sail_int zgsz3371;
      CREATE(sail_int)(&zgsz3371);
      CONVERT_OF(sail_int, mach_int)(&zgsz3371, INT64_C(0));
      lbits zgsz3372;
      CREATE(lbits)(&zgsz3372);
      vector_subrange_lbits(&zgsz3372, zgsz3369, zgsz3370, zgsz3371);
      zt_lo = CONVERT_OF(fbits, lbits)(zgsz3372, true);
      KILL(lbits)(&zgsz3372);
      KILL(sail_int)(&zgsz3371);
      KILL(sail_int)(&zgsz3370);
      KILL(lbits)(&zgsz3369);
    }
  }
  uint64_t zb_hi;
  {
    uint64_t zgaz3196;
    zgaz3196 = zcap.zB;
    int64_t zgaz3197;
    {
      sail_int zgsz3380;
      CREATE(sail_int)(&zgsz3380);
      CONVERT_OF(sail_int, mach_int)(&zgsz3380, zcap_mantissa_width);
      sail_int zgsz3381;
      CREATE(sail_int)(&zgsz3381);
      CONVERT_OF(sail_int, mach_int)(&zgsz3381, INT64_C(1));
      sail_int zgsz3382;
      CREATE(sail_int)(&zgsz3382);
      sub_int(&zgsz3382, zgsz3380, zgsz3381);
      zgaz3197 = CONVERT_OF(mach_int, sail_int)(zgsz3382);
      KILL(sail_int)(&zgsz3382);
      KILL(sail_int)(&zgsz3381);
      KILL(sail_int)(&zgsz3380);
    }
    {
      lbits zgsz3376;
      CREATE(lbits)(&zgsz3376);
      CONVERT_OF(lbits, fbits)(&zgsz3376, zgaz3196, UINT64_C(8) , true);
      sail_int zgsz3377;
      CREATE(sail_int)(&zgsz3377);
      CONVERT_OF(sail_int, mach_int)(&zgsz3377, zgaz3197);
      sail_int zgsz3378;
      CREATE(sail_int)(&zgsz3378);
      CONVERT_OF(sail_int, mach_int)(&zgsz3378, zinternal_E_take_bits);
      lbits zgsz3379;
      CREATE(lbits)(&zgsz3379);
      vector_subrange_lbits(&zgsz3379, zgsz3376, zgsz3377, zgsz3378);
      zb_hi = CONVERT_OF(fbits, lbits)(zgsz3379, true);
      KILL(lbits)(&zgsz3379);
      KILL(sail_int)(&zgsz3378);
      KILL(sail_int)(&zgsz3377);
      KILL(lbits)(&zgsz3376);
    }
  }
  uint64_t zb_lo;
  {
    uint64_t zgaz3194;
    zgaz3194 = zcap.zB;
    int64_t zgaz3195;
    {
      sail_int zgsz3387;
      CREATE(sail_int)(&zgsz3387);
      CONVERT_OF(sail_int, mach_int)(&zgsz3387, zinternal_E_take_bits);
      sail_int zgsz3388;
      CREATE(sail_int)(&zgsz3388);
      CONVERT_OF(sail_int, mach_int)(&zgsz3388, INT64_C(1));
      sail_int zgsz3389;
      CREATE(sail_int)(&zgsz3389);
      sub_int(&zgsz3389, zgsz3387, zgsz3388);
      zgaz3195 = CONVERT_OF(mach_int, sail_int)(zgsz3389);
      KILL(sail_int)(&zgsz3389);
      KILL(sail_int)(&zgsz3388);
      KILL(sail_int)(&zgsz3387);
    }
    {
      lbits zgsz3383;
      CREATE(lbits)(&zgsz3383);
      CONVERT_OF(lbits, fbits)(&zgsz3383, zgaz3194, UINT64_C(8) , true);
      sail_int zgsz3384;
      CREATE(sail_int)(&zgsz3384);
      CONVERT_OF(sail_int, mach_int)(&zgsz3384, zgaz3195);
      sail_int zgsz3385;
      CREATE(sail_int)(&zgsz3385);
      CONVERT_OF(sail_int, mach_int)(&zgsz3385, INT64_C(0));
      lbits zgsz3386;
      CREATE(lbits)(&zgsz3386);
      vector_subrange_lbits(&zgsz3386, zgsz3383, zgsz3384, zgsz3385);
      zb_lo = CONVERT_OF(fbits, lbits)(zgsz3386, true);
      KILL(lbits)(&zgsz3386);
      KILL(sail_int)(&zgsz3385);
      KILL(sail_int)(&zgsz3384);
      KILL(lbits)(&zgsz3383);
    }
  }
  {
    bool zgaz3174;
    zgaz3174 = zcap.zinternal_E;
    unit zgsz343;
    if (zgaz3174) {
      {
        uint64_t zgaz3176;
        zgaz3176 = zcap.zE;
        int64_t zgaz3177;
        {
          int64_t zgaz3175;
          {
            sail_int zgsz3393;
            CREATE(sail_int)(&zgsz3393);
            CONVERT_OF(sail_int, mach_int)(&zgsz3393, INT64_C(2));
            sail_int zgsz3394;
            CREATE(sail_int)(&zgsz3394);
            CONVERT_OF(sail_int, mach_int)(&zgsz3394, zinternal_E_take_bits);
            sail_int zgsz3395;
            CREATE(sail_int)(&zgsz3395);
            mult_int(&zgsz3395, zgsz3393, zgsz3394);
            zgaz3175 = CONVERT_OF(mach_int, sail_int)(zgsz3395);
            KILL(sail_int)(&zgsz3395);
            KILL(sail_int)(&zgsz3394);
            KILL(sail_int)(&zgsz3393);
          }
          {
            sail_int zgsz3390;
            CREATE(sail_int)(&zgsz3390);
            CONVERT_OF(sail_int, mach_int)(&zgsz3390, zgaz3175);
            sail_int zgsz3391;
            CREATE(sail_int)(&zgsz3391);
            CONVERT_OF(sail_int, mach_int)(&zgsz3391, INT64_C(1));
            sail_int zgsz3392;
            CREATE(sail_int)(&zgsz3392);
            sub_int(&zgsz3392, zgsz3390, zgsz3391);
            zgaz3177 = CONVERT_OF(mach_int, sail_int)(zgsz3392);
            KILL(sail_int)(&zgsz3392);
            KILL(sail_int)(&zgsz3391);
            KILL(sail_int)(&zgsz3390);
          }
        }
        {
          lbits zgsz3396;
          CREATE(lbits)(&zgsz3396);
          CONVERT_OF(lbits, fbits)(&zgsz3396, zgaz3176, UINT64_C(6) , true);
          sail_int zgsz3397;
          CREATE(sail_int)(&zgsz3397);
          CONVERT_OF(sail_int, mach_int)(&zgsz3397, zgaz3177);
          sail_int zgsz3398;
          CREATE(sail_int)(&zgsz3398);
          CONVERT_OF(sail_int, mach_int)(&zgsz3398, zinternal_E_take_bits);
          lbits zgsz3399;
          CREATE(lbits)(&zgsz3399);
          vector_subrange_lbits(&zgsz3399, zgsz3396, zgsz3397, zgsz3398);
          zt_lo = CONVERT_OF(fbits, lbits)(zgsz3399, true);
          KILL(lbits)(&zgsz3399);
          KILL(sail_int)(&zgsz3398);
          KILL(sail_int)(&zgsz3397);
          KILL(lbits)(&zgsz3396);
        }
        unit zgsz344;
        zgsz344 = UNIT;
      }
      uint64_t zgaz3178;
      zgaz3178 = zcap.zE;
      int64_t zgaz3179;
      {
        sail_int zgsz3404;
        CREATE(sail_int)(&zgsz3404);
        CONVERT_OF(sail_int, mach_int)(&zgsz3404, zinternal_E_take_bits);
        sail_int zgsz3405;
        CREATE(sail_int)(&zgsz3405);
        CONVERT_OF(sail_int, mach_int)(&zgsz3405, INT64_C(1));
        sail_int zgsz3406;
        CREATE(sail_int)(&zgsz3406);
        sub_int(&zgsz3406, zgsz3404, zgsz3405);
        zgaz3179 = CONVERT_OF(mach_int, sail_int)(zgsz3406);
        KILL(sail_int)(&zgsz3406);
        KILL(sail_int)(&zgsz3405);
        KILL(sail_int)(&zgsz3404);
      }
      {
        lbits zgsz3400;
        CREATE(lbits)(&zgsz3400);
        CONVERT_OF(lbits, fbits)(&zgsz3400, zgaz3178, UINT64_C(6) , true);
        sail_int zgsz3401;
        CREATE(sail_int)(&zgsz3401);
        CONVERT_OF(sail_int, mach_int)(&zgsz3401, zgaz3179);
        sail_int zgsz3402;
        CREATE(sail_int)(&zgsz3402);
        CONVERT_OF(sail_int, mach_int)(&zgsz3402, INT64_C(0));
        lbits zgsz3403;
        CREATE(lbits)(&zgsz3403);
        vector_subrange_lbits(&zgsz3403, zgsz3400, zgsz3401, zgsz3402);
        zb_lo = CONVERT_OF(fbits, lbits)(zgsz3403, true);
        KILL(lbits)(&zgsz3403);
        KILL(sail_int)(&zgsz3402);
        KILL(sail_int)(&zgsz3401);
        KILL(lbits)(&zgsz3400);
      }
      zgsz343 = UNIT;
    } else {  zgsz343 = UNIT;  }
  }
  struct zEncCapability zgaz3193;
  {
    uint64_t zgaz3182;
    {
      uint64_t zgaz3180;
      zgaz3180 = zcap.zuperms;
      uint64_t zgaz3181;
      zgaz3181 = sailgen_getCapHardPerms(zcap);
      {
        lbits zgsz3407;
        CREATE(lbits)(&zgsz3407);
        CONVERT_OF(lbits, fbits)(&zgsz3407, zgaz3180, UINT64_C(0) , true);
        lbits zgsz3408;
        CREATE(lbits)(&zgsz3408);
        CONVERT_OF(lbits, fbits)(&zgsz3408, zgaz3181, UINT64_C(12) , true);
        lbits zgsz3409;
        CREATE(lbits)(&zgsz3409);
        append(&zgsz3409, zgsz3407, zgsz3408);
        zgaz3182 = CONVERT_OF(fbits, lbits)(zgsz3409, true);
        KILL(lbits)(&zgsz3409);
        KILL(lbits)(&zgsz3408);
        KILL(lbits)(&zgsz3407);
      }
    }
    uint64_t zgaz3183;
    zgaz3183 = zcap.zreserved;
    uint64_t zgaz3185;
    {
      bool zgaz3184;
      zgaz3184 = zcap.zflag_cap_mode;
      zgaz3185 = sailgen_bool_to_bits(zgaz3184);
    }
    uint64_t zgaz3186;
    zgaz3186 = zcap.zotype;
    fbits zgaz3189;
    {
      uint64_t zgaz3188;
      {
        bool zgaz3187;
        zgaz3187 = zcap.zinternal_E;
        zgaz3188 = sailgen_bool_to_bits(zgaz3187);
      }
      {
        lbits zgsz3410;
        CREATE(lbits)(&zgsz3410);
        CONVERT_OF(lbits, fbits)(&zgsz3410, zgaz3188, UINT64_C(1) , true);
        sail_int zgsz3411;
        CREATE(sail_int)(&zgsz3411);
        CONVERT_OF(sail_int, mach_int)(&zgsz3411, INT64_C(0));
        zgaz3189 = bitvector_access(zgsz3410, zgsz3411);
        KILL(sail_int)(&zgsz3411);
        KILL(lbits)(&zgsz3410);
      }
    }
    uint64_t zgaz3190;
    {
      lbits zgsz3415;
      CREATE(lbits)(&zgsz3415);
      CONVERT_OF(lbits, fbits)(&zgsz3415, zt_hi, UINT64_C(3) , true);
      lbits zgsz3416;
      CREATE(lbits)(&zgsz3416);
      CONVERT_OF(lbits, fbits)(&zgsz3416, zt_lo, UINT64_C(3) , true);
      lbits zgsz3417;
      CREATE(lbits)(&zgsz3417);
      append(&zgsz3417, zgsz3415, zgsz3416);
      zgaz3190 = CONVERT_OF(fbits, lbits)(zgsz3417, true);
      KILL(lbits)(&zgsz3417);
      KILL(lbits)(&zgsz3416);
      KILL(lbits)(&zgsz3415);
    }
    uint64_t zgaz3191;
    {
      lbits zgsz3412;
      CREATE(lbits)(&zgsz3412);
      CONVERT_OF(lbits, fbits)(&zgsz3412, zb_hi, UINT64_C(5) , true);
      lbits zgsz3413;
      CREATE(lbits)(&zgsz3413);
      CONVERT_OF(lbits, fbits)(&zgsz3413, zb_lo, UINT64_C(3) , true);
      lbits zgsz3414;
      CREATE(lbits)(&zgsz3414);
      append(&zgsz3414, zgsz3412, zgsz3413);
      zgaz3191 = CONVERT_OF(fbits, lbits)(zgsz3414, true);
      KILL(lbits)(&zgsz3414);
      KILL(lbits)(&zgsz3413);
      KILL(lbits)(&zgsz3412);
    }
    uint64_t zgaz3192;
    zgaz3192 = zcap.zaddress;
    struct zEncCapability zgsz345;
    zgsz345.zB = zgaz3191;
    zgsz345.zT = zgaz3190;
    zgsz345.zaddress = zgaz3192;
    zgsz345.zflags = zgaz3185;
    zgsz345.zinternal_E = zgaz3189;
    zgsz345.zotype = zgaz3186;
    zgsz345.zperms = zgaz3182;
    zgsz345.zreserved = zgaz3183;
    zgaz3193 = zgsz345;
  }
  zcbz320 = zgaz3193;
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

static uint64_t sailgen_capToBits(struct zCapability);

static uint64_t sailgen_capToBits(struct zCapability zcap)
{
  __label__ end_function_98, end_block_exception_99;

  uint64_t zcbz321;
  struct zEncCapability zgaz3202;
  zgaz3202 = sailgen_capToEncCap(zcap);
  zcbz321 = sailgen_encCapToBits(zgaz3202);

end_function_98: ;
  return zcbz321;
end_block_exception_99: ;

  return UINT64_C(0xdeadc0de);
}

static uint64_t znull_cap_bits;
static void create_letbind_27(void) {


  uint64_t zgsz346;
  zgsz346 = sailgen_capToBits(znull_cap);
  znull_cap_bits = zgsz346;

let_end_100: ;
}
static void kill_letbind_27(void) {
}

static uint64_t sailgen_capToMemBits(struct zCapability);

static uint64_t sailgen_capToMemBits(struct zCapability zcap)
{
  __label__ end_function_102, end_block_exception_103;

  uint64_t zcbz322;
  uint64_t zgaz3203;
  zgaz3203 = sailgen_capToBits(zcap);
  {
    lbits zgsz3418;
    CREATE(lbits)(&zgsz3418);
    CONVERT_OF(lbits, fbits)(&zgsz3418, zgaz3203, UINT64_C(64) , true);
    lbits zgsz3419;
    CREATE(lbits)(&zgsz3419);
    CONVERT_OF(lbits, fbits)(&zgsz3419, znull_cap_bits, UINT64_C(64) , true);
    lbits zgsz3420;
    CREATE(lbits)(&zgsz3420);
    xor_bits(&zgsz3420, zgsz3418, zgsz3419);
    zcbz322 = CONVERT_OF(fbits, lbits)(zgsz3420, true);
    KILL(lbits)(&zgsz3420);
    KILL(lbits)(&zgsz3419);
    KILL(lbits)(&zgsz3418);
  }

end_function_102: ;
  return zcbz322;
end_block_exception_103: ;

  return UINT64_C(0xdeadc0de);
}

static struct zCapability sailgen_memBitsToCapability(bool, uint64_t);

static struct zCapability sailgen_memBitsToCapability(bool ztag, uint64_t zb)
{
  __label__ end_function_105, end_block_exception_106;

  struct zCapability zcbz323;
  uint64_t zgaz3204;
  {
    lbits zgsz3421;
    CREATE(lbits)(&zgsz3421);
    CONVERT_OF(lbits, fbits)(&zgsz3421, zb, UINT64_C(64) , true);
    lbits zgsz3422;
    CREATE(lbits)(&zgsz3422);
    CONVERT_OF(lbits, fbits)(&zgsz3422, znull_cap_bits, UINT64_C(64) , true);
    lbits zgsz3423;
    CREATE(lbits)(&zgsz3423);
    xor_bits(&zgsz3423, zgsz3421, zgsz3422);
    zgaz3204 = CONVERT_OF(fbits, lbits)(zgsz3423, true);
    KILL(lbits)(&zgsz3423);
    KILL(lbits)(&zgsz3422);
    KILL(lbits)(&zgsz3421);
  }
  zcbz323 = sailgen_capBitsToCapability(ztag, zgaz3204);

end_function_105: ;
  return zcbz323;
end_block_exception_106: ;
  struct zCapability zcbz351 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz351;
}

// struct tuple_(%bv32, %bv33)
struct ztuple_z8z5bv32zCz0z5bv33z9 {
  uint64_t ztup0;
  uint64_t ztup1;
};

static void COPY(ztuple_z8z5bv32zCz0z5bv33z9)(struct ztuple_z8z5bv32zCz0z5bv33z9 *rop, const struct ztuple_z8z5bv32zCz0z5bv33z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5bv32zCz0z5bv33z9)(struct ztuple_z8z5bv32zCz0z5bv33z9 op1, struct ztuple_z8z5bv32zCz0z5bv33z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1);
}

static struct ztuple_z8z5bv32zCz0z5bv33z9 sailgen_getCapBoundsBits(struct zCapability);

static struct ztuple_z8z5bv32zCz0z5bv33z9 sailgen_getCapBoundsBits(struct zCapability zc)
{
  __label__ end_function_108, end_block_exception_109;

  struct ztuple_z8z5bv32zCz0z5bv33z9 zcbz324;
  int64_t zE;
  {
    int64_t zgaz3238;
    {
      uint64_t zgaz3237;
      zgaz3237 = zc.zE;
      {
        lbits zgsz3424;
        CREATE(lbits)(&zgsz3424);
        CONVERT_OF(lbits, fbits)(&zgsz3424, zgaz3237, UINT64_C(6) , true);
        sail_int zgsz3425;
        CREATE(sail_int)(&zgsz3425);
        sail_unsigned(&zgsz3425, zgsz3424);
        zgaz3238 = CONVERT_OF(mach_int, sail_int)(zgsz3425);
        KILL(sail_int)(&zgsz3425);
        KILL(lbits)(&zgsz3424);
      }
    }
    {
      sail_int zgsz3426;
      CREATE(sail_int)(&zgsz3426);
      CONVERT_OF(sail_int, mach_int)(&zgsz3426, zcap_max_E);
      sail_int zgsz3427;
      CREATE(sail_int)(&zgsz3427);
      CONVERT_OF(sail_int, mach_int)(&zgsz3427, zgaz3238);
      sail_int zgsz3428;
      CREATE(sail_int)(&zgsz3428);
      min_int(&zgsz3428, zgsz3426, zgsz3427);
      zE = CONVERT_OF(mach_int, sail_int)(zgsz3428);
      KILL(sail_int)(&zgsz3428);
      KILL(sail_int)(&zgsz3427);
      KILL(sail_int)(&zgsz3426);
    }
  }
  uint64_t za;
  za = zc.zaddress;
  uint64_t za3;
  {
    uint64_t zgaz3236;
    {
      int64_t zgaz3235;
      {
        int64_t zgaz3234;
        {
          sail_int zgsz3432;
          CREATE(sail_int)(&zgsz3432);
          CONVERT_OF(sail_int, mach_int)(&zgsz3432, zE);
          sail_int zgsz3433;
          CREATE(sail_int)(&zgsz3433);
          CONVERT_OF(sail_int, mach_int)(&zgsz3433, zcap_mantissa_width);
          sail_int zgsz3434;
          CREATE(sail_int)(&zgsz3434);
          add_int(&zgsz3434, zgsz3432, zgsz3433);
          zgaz3234 = CONVERT_OF(mach_int, sail_int)(zgsz3434);
          KILL(sail_int)(&zgsz3434);
          KILL(sail_int)(&zgsz3433);
          KILL(sail_int)(&zgsz3432);
        }
        {
          sail_int zgsz3429;
          CREATE(sail_int)(&zgsz3429);
          CONVERT_OF(sail_int, mach_int)(&zgsz3429, zgaz3234);
          sail_int zgsz3430;
          CREATE(sail_int)(&zgsz3430);
          CONVERT_OF(sail_int, mach_int)(&zgsz3430, INT64_C(3));
          sail_int zgsz3431;
          CREATE(sail_int)(&zgsz3431);
          sub_int(&zgsz3431, zgsz3429, zgsz3430);
          zgaz3235 = CONVERT_OF(mach_int, sail_int)(zgsz3431);
          KILL(sail_int)(&zgsz3431);
          KILL(sail_int)(&zgsz3430);
          KILL(sail_int)(&zgsz3429);
        }
      }
      {
        lbits zgsz3435;
        CREATE(lbits)(&zgsz3435);
        CONVERT_OF(lbits, fbits)(&zgsz3435, za, UINT64_C(32) , true);
        sail_int zgsz3436;
        CREATE(sail_int)(&zgsz3436);
        CONVERT_OF(sail_int, mach_int)(&zgsz3436, zgaz3235);
        lbits zgsz3437;
        CREATE(lbits)(&zgsz3437);
        shiftr(&zgsz3437, zgsz3435, zgsz3436);
        zgaz3236 = CONVERT_OF(fbits, lbits)(zgsz3437, true);
        KILL(lbits)(&zgsz3437);
        KILL(sail_int)(&zgsz3436);
        KILL(lbits)(&zgsz3435);
      }
    }
    {
      lbits zgsz3438;
      CREATE(lbits)(&zgsz3438);
      CONVERT_OF(lbits, fbits)(&zgsz3438, zgaz3236, UINT64_C(32) , true);
      sail_int zgsz3439;
      CREATE(sail_int)(&zgsz3439);
      CONVERT_OF(sail_int, mach_int)(&zgsz3439, INT64_C(3));
      lbits zgsz3440;
      CREATE(lbits)(&zgsz3440);
      sail_truncate(&zgsz3440, zgsz3438, zgsz3439);
      za3 = CONVERT_OF(fbits, lbits)(zgsz3440, true);
      KILL(lbits)(&zgsz3440);
      KILL(sail_int)(&zgsz3439);
      KILL(lbits)(&zgsz3438);
    }
  }
  uint64_t zB3;
  {
    uint64_t zgaz3233;
    zgaz3233 = zc.zB;
    {
      lbits zgsz3441;
      CREATE(lbits)(&zgsz3441);
      CONVERT_OF(lbits, fbits)(&zgsz3441, zgaz3233, UINT64_C(8) , true);
      sail_int zgsz3442;
      CREATE(sail_int)(&zgsz3442);
      CONVERT_OF(sail_int, mach_int)(&zgsz3442, INT64_C(3));
      lbits zgsz3443;
      CREATE(lbits)(&zgsz3443);
      sail_truncateLSB(&zgsz3443, zgsz3441, zgsz3442);
      zB3 = CONVERT_OF(fbits, lbits)(zgsz3443, true);
      KILL(lbits)(&zgsz3443);
      KILL(sail_int)(&zgsz3442);
      KILL(lbits)(&zgsz3441);
    }
  }
  uint64_t zT3;
  {
    uint64_t zgaz3232;
    zgaz3232 = zc.zT;
    {
      lbits zgsz3444;
      CREATE(lbits)(&zgsz3444);
      CONVERT_OF(lbits, fbits)(&zgsz3444, zgaz3232, UINT64_C(8) , true);
      sail_int zgsz3445;
      CREATE(sail_int)(&zgsz3445);
      CONVERT_OF(sail_int, mach_int)(&zgsz3445, INT64_C(3));
      lbits zgsz3446;
      CREATE(lbits)(&zgsz3446);
      sail_truncateLSB(&zgsz3446, zgsz3444, zgsz3445);
      zT3 = CONVERT_OF(fbits, lbits)(zgsz3446, true);
      KILL(lbits)(&zgsz3446);
      KILL(sail_int)(&zgsz3445);
      KILL(lbits)(&zgsz3444);
    }
  }
  uint64_t zR3;
  {
    lbits zgsz3528;
    CREATE(lbits)(&zgsz3528);
    CONVERT_OF(lbits, fbits)(&zgsz3528, zB3, UINT64_C(3) , true);
    lbits zgsz3529;
    CREATE(lbits)(&zgsz3529);
    CONVERT_OF(lbits, fbits)(&zgsz3529, UINT64_C(0b001), UINT64_C(3) , true);
    lbits zgsz3530;
    CREATE(lbits)(&zgsz3530);
    sub_bits(&zgsz3530, zgsz3528, zgsz3529);
    zR3 = CONVERT_OF(fbits, lbits)(zgsz3530, true);
    KILL(lbits)(&zgsz3530);
    KILL(lbits)(&zgsz3529);
    KILL(lbits)(&zgsz3528);
  }
  int64_t zaHi;
  {
    bool zgaz3231;
    {
      lbits zgsz3447;
      CREATE(lbits)(&zgsz3447);
      CONVERT_OF(lbits, fbits)(&zgsz3447, za3, UINT64_C(3) , true);
      lbits zgsz3448;
      CREATE(lbits)(&zgsz3448);
      CONVERT_OF(lbits, fbits)(&zgsz3448, zR3, UINT64_C(3) , true);
      zgaz3231 = sailgen_z8operatorz0zI_uz9(zgsz3447, zgsz3448);
      KILL(lbits)(&zgsz3448);
      KILL(lbits)(&zgsz3447);
    }
    if (zgaz3231) {  zaHi = INT64_C(1);  } else {  zaHi = INT64_C(0);  }
  }
  int64_t zbHi;
  {
    bool zgaz3230;
    {
      lbits zgsz3449;
      CREATE(lbits)(&zgsz3449);
      CONVERT_OF(lbits, fbits)(&zgsz3449, zB3, UINT64_C(3) , true);
      lbits zgsz3450;
      CREATE(lbits)(&zgsz3450);
      CONVERT_OF(lbits, fbits)(&zgsz3450, zR3, UINT64_C(3) , true);
      zgaz3230 = sailgen_z8operatorz0zI_uz9(zgsz3449, zgsz3450);
      KILL(lbits)(&zgsz3450);
      KILL(lbits)(&zgsz3449);
    }
    if (zgaz3230) {  zbHi = INT64_C(1);  } else {  zbHi = INT64_C(0);  }
  }
  int64_t ztHi;
  {
    bool zgaz3229;
    {
      lbits zgsz3451;
      CREATE(lbits)(&zgsz3451);
      CONVERT_OF(lbits, fbits)(&zgsz3451, zT3, UINT64_C(3) , true);
      lbits zgsz3452;
      CREATE(lbits)(&zgsz3452);
      CONVERT_OF(lbits, fbits)(&zgsz3452, zR3, UINT64_C(3) , true);
      zgaz3229 = sailgen_z8operatorz0zI_uz9(zgsz3451, zgsz3452);
      KILL(lbits)(&zgsz3452);
      KILL(lbits)(&zgsz3451);
    }
    if (zgaz3229) {  ztHi = INT64_C(1);  } else {  ztHi = INT64_C(0);  }
  }
  int64_t zcorrection_base;
  {
    sail_int zgsz3525;
    CREATE(sail_int)(&zgsz3525);
    CONVERT_OF(sail_int, mach_int)(&zgsz3525, zbHi);
    sail_int zgsz3526;
    CREATE(sail_int)(&zgsz3526);
    CONVERT_OF(sail_int, mach_int)(&zgsz3526, zaHi);
    sail_int zgsz3527;
    CREATE(sail_int)(&zgsz3527);
    sub_int(&zgsz3527, zgsz3525, zgsz3526);
    zcorrection_base = CONVERT_OF(mach_int, sail_int)(zgsz3527);
    KILL(sail_int)(&zgsz3527);
    KILL(sail_int)(&zgsz3526);
    KILL(sail_int)(&zgsz3525);
  }
  int64_t zcorrection_top;
  {
    sail_int zgsz3522;
    CREATE(sail_int)(&zgsz3522);
    CONVERT_OF(sail_int, mach_int)(&zgsz3522, ztHi);
    sail_int zgsz3523;
    CREATE(sail_int)(&zgsz3523);
    CONVERT_OF(sail_int, mach_int)(&zgsz3523, zaHi);
    sail_int zgsz3524;
    CREATE(sail_int)(&zgsz3524);
    sub_int(&zgsz3524, zgsz3522, zgsz3523);
    zcorrection_top = CONVERT_OF(mach_int, sail_int)(zgsz3524);
    KILL(sail_int)(&zgsz3524);
    KILL(sail_int)(&zgsz3523);
    KILL(sail_int)(&zgsz3522);
  }
  uint64_t za_top;
  {
    int64_t zgaz3228;
    {
      sail_int zgsz3456;
      CREATE(sail_int)(&zgsz3456);
      CONVERT_OF(sail_int, mach_int)(&zgsz3456, zE);
      sail_int zgsz3457;
      CREATE(sail_int)(&zgsz3457);
      CONVERT_OF(sail_int, mach_int)(&zgsz3457, zcap_mantissa_width);
      sail_int zgsz3458;
      CREATE(sail_int)(&zgsz3458);
      add_int(&zgsz3458, zgsz3456, zgsz3457);
      zgaz3228 = CONVERT_OF(mach_int, sail_int)(zgsz3458);
      KILL(sail_int)(&zgsz3458);
      KILL(sail_int)(&zgsz3457);
      KILL(sail_int)(&zgsz3456);
    }
    {
      lbits zgsz3453;
      CREATE(lbits)(&zgsz3453);
      CONVERT_OF(lbits, fbits)(&zgsz3453, za, UINT64_C(32) , true);
      sail_int zgsz3454;
      CREATE(sail_int)(&zgsz3454);
      CONVERT_OF(sail_int, mach_int)(&zgsz3454, zgaz3228);
      lbits zgsz3455;
      CREATE(lbits)(&zgsz3455);
      shiftr(&zgsz3455, zgsz3453, zgsz3454);
      za_top = CONVERT_OF(fbits, lbits)(zgsz3455, true);
      KILL(lbits)(&zgsz3455);
      KILL(sail_int)(&zgsz3454);
      KILL(lbits)(&zgsz3453);
    }
  }
  uint64_t zbase;
  {
    lbits zgaz3227;
    CREATE(lbits)(&zgaz3227);
    {
      uint64_t zgaz3225;
      {
        lbits zgsz3466;
        CREATE(lbits)(&zgsz3466);
        CONVERT_OF(lbits, fbits)(&zgsz3466, za_top, UINT64_C(32) , true);
        sail_int zgsz3467;
        CREATE(sail_int)(&zgsz3467);
        CONVERT_OF(sail_int, mach_int)(&zgsz3467, zcorrection_base);
        lbits zgsz3468;
        CREATE(lbits)(&zgsz3468);
        add_bits_int(&zgsz3468, zgsz3466, zgsz3467);
        zgaz3225 = CONVERT_OF(fbits, lbits)(zgsz3468, true);
        KILL(lbits)(&zgsz3468);
        KILL(sail_int)(&zgsz3467);
        KILL(lbits)(&zgsz3466);
      }
      sbits zgaz3226;
      {
        uint64_t zgaz3223;
        zgaz3223 = zc.zB;
        sbits zgaz3224;
        {
          sail_int zgsz3462;
          CREATE(sail_int)(&zgsz3462);
          CONVERT_OF(sail_int, mach_int)(&zgsz3462, zE);
          lbits zgsz3463;
          CREATE(lbits)(&zgsz3463);
          sailgen_zzeros_implicit(&zgsz3463, zgsz3462);
          zgaz3224 = CONVERT_OF(sbits, lbits)(zgsz3463, true);
          KILL(lbits)(&zgsz3463);
          KILL(sail_int)(&zgsz3462);
        }
        {
          lbits zgsz3459;
          CREATE(lbits)(&zgsz3459);
          CONVERT_OF(lbits, fbits)(&zgsz3459, zgaz3223, UINT64_C(8) , true);
          lbits zgsz3460;
          CREATE(lbits)(&zgsz3460);
          CONVERT_OF(lbits, sbits)(&zgsz3460, zgaz3224, true);
          lbits zgsz3461;
          CREATE(lbits)(&zgsz3461);
          append(&zgsz3461, zgsz3459, zgsz3460);
          zgaz3226 = CONVERT_OF(sbits, lbits)(zgsz3461, true);
          KILL(lbits)(&zgsz3461);
          KILL(lbits)(&zgsz3460);
          KILL(lbits)(&zgsz3459);
        }
      }
      {
        lbits zgsz3464;
        CREATE(lbits)(&zgsz3464);
        CONVERT_OF(lbits, fbits)(&zgsz3464, zgaz3225, UINT64_C(32) , true);
        lbits zgsz3465;
        CREATE(lbits)(&zgsz3465);
        CONVERT_OF(lbits, sbits)(&zgsz3465, zgaz3226, true);
        append(&zgaz3227, zgsz3464, zgsz3465);
        KILL(lbits)(&zgsz3465);
        KILL(lbits)(&zgsz3464);
      }
    }
    {
      sail_int zgsz3469;
      CREATE(sail_int)(&zgsz3469);
      CONVERT_OF(sail_int, mach_int)(&zgsz3469, zcap_len_width);
      lbits zgsz3470;
      CREATE(lbits)(&zgsz3470);
      sail_truncate(&zgsz3470, zgaz3227, zgsz3469);
      zbase = CONVERT_OF(fbits, lbits)(zgsz3470, true);
      KILL(lbits)(&zgsz3470);
      KILL(sail_int)(&zgsz3469);
    }
    KILL(lbits)(&zgaz3227);
  }
  uint64_t ztop;
  {
    lbits zgaz3222;
    CREATE(lbits)(&zgaz3222);
    {
      uint64_t zgaz3220;
      {
        lbits zgsz3478;
        CREATE(lbits)(&zgsz3478);
        CONVERT_OF(lbits, fbits)(&zgsz3478, za_top, UINT64_C(32) , true);
        sail_int zgsz3479;
        CREATE(sail_int)(&zgsz3479);
        CONVERT_OF(sail_int, mach_int)(&zgsz3479, zcorrection_top);
        lbits zgsz3480;
        CREATE(lbits)(&zgsz3480);
        add_bits_int(&zgsz3480, zgsz3478, zgsz3479);
        zgaz3220 = CONVERT_OF(fbits, lbits)(zgsz3480, true);
        KILL(lbits)(&zgsz3480);
        KILL(sail_int)(&zgsz3479);
        KILL(lbits)(&zgsz3478);
      }
      sbits zgaz3221;
      {
        uint64_t zgaz3218;
        zgaz3218 = zc.zT;
        sbits zgaz3219;
        {
          sail_int zgsz3474;
          CREATE(sail_int)(&zgsz3474);
          CONVERT_OF(sail_int, mach_int)(&zgsz3474, zE);
          lbits zgsz3475;
          CREATE(lbits)(&zgsz3475);
          sailgen_zzeros_implicit(&zgsz3475, zgsz3474);
          zgaz3219 = CONVERT_OF(sbits, lbits)(zgsz3475, true);
          KILL(lbits)(&zgsz3475);
          KILL(sail_int)(&zgsz3474);
        }
        {
          lbits zgsz3471;
          CREATE(lbits)(&zgsz3471);
          CONVERT_OF(lbits, fbits)(&zgsz3471, zgaz3218, UINT64_C(8) , true);
          lbits zgsz3472;
          CREATE(lbits)(&zgsz3472);
          CONVERT_OF(lbits, sbits)(&zgsz3472, zgaz3219, true);
          lbits zgsz3473;
          CREATE(lbits)(&zgsz3473);
          append(&zgsz3473, zgsz3471, zgsz3472);
          zgaz3221 = CONVERT_OF(sbits, lbits)(zgsz3473, true);
          KILL(lbits)(&zgsz3473);
          KILL(lbits)(&zgsz3472);
          KILL(lbits)(&zgsz3471);
        }
      }
      {
        lbits zgsz3476;
        CREATE(lbits)(&zgsz3476);
        CONVERT_OF(lbits, fbits)(&zgsz3476, zgaz3220, UINT64_C(32) , true);
        lbits zgsz3477;
        CREATE(lbits)(&zgsz3477);
        CONVERT_OF(lbits, sbits)(&zgsz3477, zgaz3221, true);
        append(&zgaz3222, zgsz3476, zgsz3477);
        KILL(lbits)(&zgsz3477);
        KILL(lbits)(&zgsz3476);
      }
    }
    {
      sail_int zgsz3481;
      CREATE(sail_int)(&zgsz3481);
      CONVERT_OF(sail_int, mach_int)(&zgsz3481, zcap_len_width);
      lbits zgsz3482;
      CREATE(lbits)(&zgsz3482);
      sail_truncate(&zgsz3482, zgaz3222, zgsz3481);
      ztop = CONVERT_OF(fbits, lbits)(zgsz3482, true);
      KILL(lbits)(&zgsz3482);
      KILL(sail_int)(&zgsz3481);
    }
    KILL(lbits)(&zgaz3222);
  }
  uint64_t zbase2;
  {
    uint64_t zgaz3217;
    {
      fbits zgaz3216;
      {
        int64_t zgaz3215;
        {
          sail_int zgsz3485;
          CREATE(sail_int)(&zgsz3485);
          CONVERT_OF(sail_int, mach_int)(&zgsz3485, zcap_addr_width);
          sail_int zgsz3486;
          CREATE(sail_int)(&zgsz3486);
          CONVERT_OF(sail_int, mach_int)(&zgsz3486, INT64_C(1));
          sail_int zgsz3487;
          CREATE(sail_int)(&zgsz3487);
          sub_int(&zgsz3487, zgsz3485, zgsz3486);
          zgaz3215 = CONVERT_OF(mach_int, sail_int)(zgsz3487);
          KILL(sail_int)(&zgsz3487);
          KILL(sail_int)(&zgsz3486);
          KILL(sail_int)(&zgsz3485);
        }
        {
          lbits zgsz3483;
          CREATE(lbits)(&zgsz3483);
          CONVERT_OF(lbits, fbits)(&zgsz3483, zbase, UINT64_C(33) , true);
          sail_int zgsz3484;
          CREATE(sail_int)(&zgsz3484);
          CONVERT_OF(sail_int, mach_int)(&zgsz3484, zgaz3215);
          zgaz3216 = bitvector_access(zgsz3483, zgsz3484);
          KILL(sail_int)(&zgsz3484);
          KILL(lbits)(&zgsz3483);
        }
      }
      uint64_t zgsz347;
      zgsz347 = UINT64_C(0b0);
      zgsz347 = update_fbits(zgsz347, INT64_C(0), zgaz3216);
      zgaz3217 = zgsz347;
    }
    {
      lbits zgsz3488;
      CREATE(lbits)(&zgsz3488);
      CONVERT_OF(lbits, fbits)(&zgsz3488, UINT64_C(0b0), UINT64_C(1) , true);
      lbits zgsz3489;
      CREATE(lbits)(&zgsz3489);
      CONVERT_OF(lbits, fbits)(&zgsz3489, zgaz3217, UINT64_C(1) , true);
      lbits zgsz3490;
      CREATE(lbits)(&zgsz3490);
      append(&zgsz3490, zgsz3488, zgsz3489);
      zbase2 = CONVERT_OF(fbits, lbits)(zgsz3490, true);
      KILL(lbits)(&zgsz3490);
      KILL(lbits)(&zgsz3489);
      KILL(lbits)(&zgsz3488);
    }
  }
  uint64_t ztop2;
  {
    int64_t zgaz3214;
    {
      sail_int zgsz3495;
      CREATE(sail_int)(&zgsz3495);
      CONVERT_OF(sail_int, mach_int)(&zgsz3495, zcap_addr_width);
      sail_int zgsz3496;
      CREATE(sail_int)(&zgsz3496);
      CONVERT_OF(sail_int, mach_int)(&zgsz3496, INT64_C(1));
      sail_int zgsz3497;
      CREATE(sail_int)(&zgsz3497);
      sub_int(&zgsz3497, zgsz3495, zgsz3496);
      zgaz3214 = CONVERT_OF(mach_int, sail_int)(zgsz3497);
      KILL(sail_int)(&zgsz3497);
      KILL(sail_int)(&zgsz3496);
      KILL(sail_int)(&zgsz3495);
    }
    {
      lbits zgsz3491;
      CREATE(lbits)(&zgsz3491);
      CONVERT_OF(lbits, fbits)(&zgsz3491, ztop, UINT64_C(33) , true);
      sail_int zgsz3492;
      CREATE(sail_int)(&zgsz3492);
      CONVERT_OF(sail_int, mach_int)(&zgsz3492, zcap_addr_width);
      sail_int zgsz3493;
      CREATE(sail_int)(&zgsz3493);
      CONVERT_OF(sail_int, mach_int)(&zgsz3493, zgaz3214);
      lbits zgsz3494;
      CREATE(lbits)(&zgsz3494);
      vector_subrange_lbits(&zgsz3494, zgsz3491, zgsz3492, zgsz3493);
      ztop2 = CONVERT_OF(fbits, lbits)(zgsz3494, true);
      KILL(lbits)(&zgsz3494);
      KILL(sail_int)(&zgsz3493);
      KILL(sail_int)(&zgsz3492);
      KILL(lbits)(&zgsz3491);
    }
  }
  {
    bool zgaz3209;
    {
      bool zgaz3208;
      {
        int64_t zgaz3205;
        {
          sail_int zgsz3500;
          CREATE(sail_int)(&zgsz3500);
          CONVERT_OF(sail_int, mach_int)(&zgsz3500, zcap_max_E);
          sail_int zgsz3501;
          CREATE(sail_int)(&zgsz3501);
          CONVERT_OF(sail_int, mach_int)(&zgsz3501, INT64_C(1));
          sail_int zgsz3502;
          CREATE(sail_int)(&zgsz3502);
          sub_int(&zgsz3502, zgsz3500, zgsz3501);
          zgaz3205 = CONVERT_OF(mach_int, sail_int)(zgsz3502);
          KILL(sail_int)(&zgsz3502);
          KILL(sail_int)(&zgsz3501);
          KILL(sail_int)(&zgsz3500);
        }
        {
          sail_int zgsz3498;
          CREATE(sail_int)(&zgsz3498);
          CONVERT_OF(sail_int, mach_int)(&zgsz3498, zE);
          sail_int zgsz3499;
          CREATE(sail_int)(&zgsz3499);
          CONVERT_OF(sail_int, mach_int)(&zgsz3499, zgaz3205);
          zgaz3208 = lt(zgsz3498, zgsz3499);
          KILL(sail_int)(&zgsz3499);
          KILL(sail_int)(&zgsz3498);
        }
      }
      bool zgsz348;
      if (zgaz3208) {
        int64_t zgaz3207;
        {
          uint64_t zgaz3206;
          {
            lbits zgsz3505;
            CREATE(lbits)(&zgsz3505);
            CONVERT_OF(lbits, fbits)(&zgsz3505, ztop2, UINT64_C(2) , true);
            lbits zgsz3506;
            CREATE(lbits)(&zgsz3506);
            CONVERT_OF(lbits, fbits)(&zgsz3506, zbase2, UINT64_C(2) , true);
            lbits zgsz3507;
            CREATE(lbits)(&zgsz3507);
            sub_bits(&zgsz3507, zgsz3505, zgsz3506);
            zgaz3206 = CONVERT_OF(fbits, lbits)(zgsz3507, true);
            KILL(lbits)(&zgsz3507);
            KILL(lbits)(&zgsz3506);
            KILL(lbits)(&zgsz3505);
          }
          {
            lbits zgsz3503;
            CREATE(lbits)(&zgsz3503);
            CONVERT_OF(lbits, fbits)(&zgsz3503, zgaz3206, UINT64_C(2) , true);
            sail_int zgsz3504;
            CREATE(sail_int)(&zgsz3504);
            sail_unsigned(&zgsz3504, zgsz3503);
            zgaz3207 = CONVERT_OF(mach_int, sail_int)(zgsz3504);
            KILL(sail_int)(&zgsz3504);
            KILL(lbits)(&zgsz3503);
          }
        }
        {
          sail_int zgsz3508;
          CREATE(sail_int)(&zgsz3508);
          CONVERT_OF(sail_int, mach_int)(&zgsz3508, zgaz3207);
          sail_int zgsz3509;
          CREATE(sail_int)(&zgsz3509);
          CONVERT_OF(sail_int, mach_int)(&zgsz3509, INT64_C(1));
          zgsz348 = gt(zgsz3508, zgsz3509);
          KILL(sail_int)(&zgsz3509);
          KILL(sail_int)(&zgsz3508);
        }
      } else {  zgsz348 = false;  }
      zgaz3209 = zgsz348;
    }
    unit zgsz349;
    if (zgaz3209) {
      fbits zgaz3211;
      {
        fbits zgaz3210;
        {
          lbits zgsz3510;
          CREATE(lbits)(&zgsz3510);
          CONVERT_OF(lbits, fbits)(&zgsz3510, ztop, UINT64_C(33) , true);
          sail_int zgsz3511;
          CREATE(sail_int)(&zgsz3511);
          CONVERT_OF(sail_int, mach_int)(&zgsz3511, zcap_addr_width);
          zgaz3210 = bitvector_access(zgsz3510, zgsz3511);
          KILL(sail_int)(&zgsz3511);
          KILL(lbits)(&zgsz3510);
        }
        zgaz3211 = sailgen_not_bit(zgaz3210);
      }
      {
        lbits zgsz3512;
        CREATE(lbits)(&zgsz3512);
        CONVERT_OF(lbits, fbits)(&zgsz3512, ztop, UINT64_C(33) , true);
        sail_int zgsz3513;
        CREATE(sail_int)(&zgsz3513);
        CONVERT_OF(sail_int, mach_int)(&zgsz3513, zcap_addr_width);
        lbits zgsz3514;
        CREATE(lbits)(&zgsz3514);
        update_lbits(&zgsz3514, zgsz3512, zgsz3513, zgaz3211);
        ztop = CONVERT_OF(fbits, lbits)(zgsz3514, true);
        KILL(lbits)(&zgsz3514);
        KILL(sail_int)(&zgsz3513);
        KILL(lbits)(&zgsz3512);
      }
      zgsz349 = UNIT;
    } else {  zgsz349 = UNIT;  }
  }
  uint64_t zgaz3213;
  {
    int64_t zgaz3212;
    {
      sail_int zgsz3519;
      CREATE(sail_int)(&zgsz3519);
      CONVERT_OF(sail_int, mach_int)(&zgsz3519, zcap_addr_width);
      sail_int zgsz3520;
      CREATE(sail_int)(&zgsz3520);
      CONVERT_OF(sail_int, mach_int)(&zgsz3520, INT64_C(1));
      sail_int zgsz3521;
      CREATE(sail_int)(&zgsz3521);
      sub_int(&zgsz3521, zgsz3519, zgsz3520);
      zgaz3212 = CONVERT_OF(mach_int, sail_int)(zgsz3521);
      KILL(sail_int)(&zgsz3521);
      KILL(sail_int)(&zgsz3520);
      KILL(sail_int)(&zgsz3519);
    }
    {
      lbits zgsz3515;
      CREATE(lbits)(&zgsz3515);
      CONVERT_OF(lbits, fbits)(&zgsz3515, zbase, UINT64_C(33) , true);
      sail_int zgsz3516;
      CREATE(sail_int)(&zgsz3516);
      CONVERT_OF(sail_int, mach_int)(&zgsz3516, zgaz3212);
      sail_int zgsz3517;
      CREATE(sail_int)(&zgsz3517);
      CONVERT_OF(sail_int, mach_int)(&zgsz3517, INT64_C(0));
      lbits zgsz3518;
      CREATE(lbits)(&zgsz3518);
      vector_subrange_lbits(&zgsz3518, zgsz3515, zgsz3516, zgsz3517);
      zgaz3213 = CONVERT_OF(fbits, lbits)(zgsz3518, true);
      KILL(lbits)(&zgsz3518);
      KILL(sail_int)(&zgsz3517);
      KILL(sail_int)(&zgsz3516);
      KILL(lbits)(&zgsz3515);
    }
  }
  struct ztuple_z8z5bv32zCz0z5bv33z9 zgsz350;
  zgsz350.ztup0 = zgaz3213;
  zgsz350.ztup1 = ztop;
  zcbz324 = zgsz350;


















end_function_108: ;
  return zcbz324;
end_block_exception_109: ;
  struct ztuple_z8z5bv32zCz0z5bv33z9 zcbz352 = { .ztup1 = UINT64_C(0xdeadc0de), .ztup0 = UINT64_C(0xdeadc0de) };
  return zcbz352;
}

// struct tuple_(%i64, %i64)
struct ztuple_z8z5i64zCz0z5i64z9 {
  int64_t ztup0;
  int64_t ztup1;
};

static void COPY(ztuple_z8z5i64zCz0z5i64z9)(struct ztuple_z8z5i64zCz0z5i64z9 *rop, const struct ztuple_z8z5i64zCz0z5i64z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5i64zCz0z5i64z9)(struct ztuple_z8z5i64zCz0z5i64z9 op1, struct ztuple_z8z5i64zCz0z5i64z9 op2) {
  return EQUAL(mach_int)(op1.ztup0, op2.ztup0) && EQUAL(mach_int)(op1.ztup1, op2.ztup1);
}

static struct ztuple_z8z5i64zCz0z5i64z9 sailgen_getCapBounds(struct zCapability);


static struct ztuple_z8z5i64zCz0z5i64z9 sailgen_getCapBounds(struct zCapability zcap)
{
  __label__ case_112, finish_match_111, end_function_113, end_block_exception_114;

  struct ztuple_z8z5i64zCz0z5i64z9 zcbz325;
  struct ztuple_z8z5bv32zCz0z5bv33z9 zgaz3239;
  zgaz3239 = sailgen_getCapBoundsBits(zcap);
  struct ztuple_z8z5i64zCz0z5i64z9 zgsz351;
  {
    uint64_t zbase;
    zbase = zgaz3239.ztup0;
    uint64_t ztop;
    ztop = zgaz3239.ztup1;
    int64_t zgaz3240;
    {
      lbits zgsz3533;
      CREATE(lbits)(&zgsz3533);
      CONVERT_OF(lbits, fbits)(&zgsz3533, zbase, UINT64_C(32) , true);
      sail_int zgsz3534;
      CREATE(sail_int)(&zgsz3534);
      sail_unsigned(&zgsz3534, zgsz3533);
      zgaz3240 = CONVERT_OF(mach_int, sail_int)(zgsz3534);
      KILL(sail_int)(&zgsz3534);
      KILL(lbits)(&zgsz3533);
    }
    int64_t zgaz3241;
    {
      lbits zgsz3531;
      CREATE(lbits)(&zgsz3531);
      CONVERT_OF(lbits, fbits)(&zgsz3531, ztop, UINT64_C(33) , true);
      sail_int zgsz3532;
      CREATE(sail_int)(&zgsz3532);
      sail_unsigned(&zgsz3532, zgsz3531);
      zgaz3241 = CONVERT_OF(mach_int, sail_int)(zgsz3532);
      KILL(sail_int)(&zgsz3532);
      KILL(lbits)(&zgsz3531);
    }
    struct ztuple_z8z5i64zCz0z5i64z9 zgsz352;
    zgsz352.ztup0 = zgaz3240;
    zgsz352.ztup1 = zgaz3241;
    zgsz351 = zgsz352;
    goto finish_match_111;
  }
case_112: ;
  sail_match_failure("getCapBounds");
finish_match_111: ;
  zcbz325 = zgsz351;


end_function_113: ;
  return zcbz325;
end_block_exception_114: ;
  struct ztuple_z8z5i64zCz0z5i64z9 zcbz353 = { .ztup1 = INT64_C(0xdeadc0de), .ztup0 = INT64_C(0xdeadc0de) };
  return zcbz353;
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

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_setCapBounds(struct zCapability, uint64_t, uint64_t);

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_setCapBounds(struct zCapability zcap, uint64_t zbase, uint64_t ztop)
{
  __label__ end_function_116, end_block_exception_117;

  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz326;
  uint64_t zext_base;
  {
    lbits zgsz3666;
    CREATE(lbits)(&zgsz3666);
    CONVERT_OF(lbits, fbits)(&zgsz3666, UINT64_C(0b0), UINT64_C(1) , true);
    lbits zgsz3667;
    CREATE(lbits)(&zgsz3667);
    CONVERT_OF(lbits, fbits)(&zgsz3667, zbase, UINT64_C(32) , true);
    lbits zgsz3668;
    CREATE(lbits)(&zgsz3668);
    append(&zgsz3668, zgsz3666, zgsz3667);
    zext_base = CONVERT_OF(fbits, lbits)(zgsz3668, true);
    KILL(lbits)(&zgsz3668);
    KILL(lbits)(&zgsz3667);
    KILL(lbits)(&zgsz3666);
  }
  uint64_t zlength;
  {
    lbits zgsz3663;
    CREATE(lbits)(&zgsz3663);
    CONVERT_OF(lbits, fbits)(&zgsz3663, ztop, UINT64_C(33) , true);
    lbits zgsz3664;
    CREATE(lbits)(&zgsz3664);
    CONVERT_OF(lbits, fbits)(&zgsz3664, zext_base, UINT64_C(33) , true);
    lbits zgsz3665;
    CREATE(lbits)(&zgsz3665);
    sub_bits(&zgsz3665, zgsz3663, zgsz3664);
    zlength = CONVERT_OF(fbits, lbits)(zgsz3665, true);
    KILL(lbits)(&zgsz3665);
    KILL(lbits)(&zgsz3664);
    KILL(lbits)(&zgsz3663);
  }
  int64_t ze;
  {
    int64_t zgaz3275;
    {
      uint64_t zgaz3274;
      {
        int64_t zgaz3273;
        {
          sail_int zgsz3539;
          CREATE(sail_int)(&zgsz3539);
          CONVERT_OF(sail_int, mach_int)(&zgsz3539, zcap_mantissa_width);
          sail_int zgsz3540;
          CREATE(sail_int)(&zgsz3540);
          CONVERT_OF(sail_int, mach_int)(&zgsz3540, INT64_C(1));
          sail_int zgsz3541;
          CREATE(sail_int)(&zgsz3541);
          sub_int(&zgsz3541, zgsz3539, zgsz3540);
          zgaz3273 = CONVERT_OF(mach_int, sail_int)(zgsz3541);
          KILL(sail_int)(&zgsz3541);
          KILL(sail_int)(&zgsz3540);
          KILL(sail_int)(&zgsz3539);
        }
        {
          lbits zgsz3535;
          CREATE(lbits)(&zgsz3535);
          CONVERT_OF(lbits, fbits)(&zgsz3535, zlength, UINT64_C(33) , true);
          sail_int zgsz3536;
          CREATE(sail_int)(&zgsz3536);
          CONVERT_OF(sail_int, mach_int)(&zgsz3536, zcap_addr_width);
          sail_int zgsz3537;
          CREATE(sail_int)(&zgsz3537);
          CONVERT_OF(sail_int, mach_int)(&zgsz3537, zgaz3273);
          lbits zgsz3538;
          CREATE(lbits)(&zgsz3538);
          vector_subrange_lbits(&zgsz3538, zgsz3535, zgsz3536, zgsz3537);
          zgaz3274 = CONVERT_OF(fbits, lbits)(zgsz3538, true);
          KILL(lbits)(&zgsz3538);
          KILL(sail_int)(&zgsz3537);
          KILL(sail_int)(&zgsz3536);
          KILL(lbits)(&zgsz3535);
        }
      }
      {
        lbits zgsz3542;
        CREATE(lbits)(&zgsz3542);
        CONVERT_OF(lbits, fbits)(&zgsz3542, zgaz3274, UINT64_C(26) , true);
        sail_int zgsz3543;
        CREATE(sail_int)(&zgsz3543);
        count_leading_zeros(&zgsz3543, zgsz3542);
        zgaz3275 = CONVERT_OF(mach_int, sail_int)(zgsz3543);
        KILL(sail_int)(&zgsz3543);
        KILL(lbits)(&zgsz3542);
      }
    }
    {
      sail_int zgsz3544;
      CREATE(sail_int)(&zgsz3544);
      CONVERT_OF(sail_int, mach_int)(&zgsz3544, zcap_max_E);
      sail_int zgsz3545;
      CREATE(sail_int)(&zgsz3545);
      CONVERT_OF(sail_int, mach_int)(&zgsz3545, zgaz3275);
      sail_int zgsz3546;
      CREATE(sail_int)(&zgsz3546);
      sub_int(&zgsz3546, zgsz3544, zgsz3545);
      ze = CONVERT_OF(mach_int, sail_int)(zgsz3546);
      KILL(sail_int)(&zgsz3546);
      KILL(sail_int)(&zgsz3545);
      KILL(sail_int)(&zgsz3544);
    }
  }
  bool zie;
  {
    bool zgaz3272;
    {
      sail_int zgsz3552;
      CREATE(sail_int)(&zgsz3552);
      CONVERT_OF(sail_int, mach_int)(&zgsz3552, ze);
      sail_int zgsz3553;
      CREATE(sail_int)(&zgsz3553);
      CONVERT_OF(sail_int, mach_int)(&zgsz3553, INT64_C(0));
      zgaz3272 = sailgen_neq_int(zgsz3552, zgsz3553);
      KILL(sail_int)(&zgsz3553);
      KILL(sail_int)(&zgsz3552);
    }
    bool zgsz354;
    if (zgaz3272) {  zgsz354 = true;  } else {
      fbits zgaz3271;
      {
        int64_t zgaz3270;
        {
          sail_int zgsz3549;
          CREATE(sail_int)(&zgsz3549);
          CONVERT_OF(sail_int, mach_int)(&zgsz3549, zcap_mantissa_width);
          sail_int zgsz3550;
          CREATE(sail_int)(&zgsz3550);
          CONVERT_OF(sail_int, mach_int)(&zgsz3550, INT64_C(2));
          sail_int zgsz3551;
          CREATE(sail_int)(&zgsz3551);
          sub_int(&zgsz3551, zgsz3549, zgsz3550);
          zgaz3270 = CONVERT_OF(mach_int, sail_int)(zgsz3551);
          KILL(sail_int)(&zgsz3551);
          KILL(sail_int)(&zgsz3550);
          KILL(sail_int)(&zgsz3549);
        }
        {
          lbits zgsz3547;
          CREATE(lbits)(&zgsz3547);
          CONVERT_OF(lbits, fbits)(&zgsz3547, zlength, UINT64_C(33) , true);
          sail_int zgsz3548;
          CREATE(sail_int)(&zgsz3548);
          CONVERT_OF(sail_int, mach_int)(&zgsz3548, zgaz3270);
          zgaz3271 = bitvector_access(zgsz3547, zgsz3548);
          KILL(sail_int)(&zgsz3548);
          KILL(lbits)(&zgsz3547);
        }
      }
      zgsz354 = eq_bit(zgaz3271, UINT64_C(1));
    }
    zie = zgsz354;
  }
  uint64_t zBbits;
  {
    lbits zgsz3660;
    CREATE(lbits)(&zgsz3660);
    CONVERT_OF(lbits, fbits)(&zgsz3660, zbase, UINT64_C(32) , true);
    sail_int zgsz3661;
    CREATE(sail_int)(&zgsz3661);
    CONVERT_OF(sail_int, mach_int)(&zgsz3661, zcap_mantissa_width);
    lbits zgsz3662;
    CREATE(lbits)(&zgsz3662);
    sail_truncate(&zgsz3662, zgsz3660, zgsz3661);
    zBbits = CONVERT_OF(fbits, lbits)(zgsz3662, true);
    KILL(lbits)(&zgsz3662);
    KILL(sail_int)(&zgsz3661);
    KILL(lbits)(&zgsz3660);
  }
  uint64_t zTbits;
  {
    lbits zgsz3657;
    CREATE(lbits)(&zgsz3657);
    CONVERT_OF(lbits, fbits)(&zgsz3657, ztop, UINT64_C(33) , true);
    sail_int zgsz3658;
    CREATE(sail_int)(&zgsz3658);
    CONVERT_OF(sail_int, mach_int)(&zgsz3658, zcap_mantissa_width);
    lbits zgsz3659;
    CREATE(lbits)(&zgsz3659);
    sail_truncate(&zgsz3659, zgsz3657, zgsz3658);
    zTbits = CONVERT_OF(fbits, lbits)(zgsz3659, true);
    KILL(lbits)(&zgsz3659);
    KILL(sail_int)(&zgsz3658);
    KILL(lbits)(&zgsz3657);
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
        uint64_t zgaz3265;
        {
          int64_t zgaz3264;
          {
            sail_int zgsz3557;
            CREATE(sail_int)(&zgsz3557);
            CONVERT_OF(sail_int, mach_int)(&zgsz3557, ze);
            sail_int zgsz3558;
            CREATE(sail_int)(&zgsz3558);
            CONVERT_OF(sail_int, mach_int)(&zgsz3558, INT64_C(3));
            sail_int zgsz3559;
            CREATE(sail_int)(&zgsz3559);
            add_int(&zgsz3559, zgsz3557, zgsz3558);
            zgaz3264 = CONVERT_OF(mach_int, sail_int)(zgsz3559);
            KILL(sail_int)(&zgsz3559);
            KILL(sail_int)(&zgsz3558);
            KILL(sail_int)(&zgsz3557);
          }
          {
            lbits zgsz3554;
            CREATE(lbits)(&zgsz3554);
            CONVERT_OF(lbits, fbits)(&zgsz3554, zbase, UINT64_C(32) , true);
            sail_int zgsz3555;
            CREATE(sail_int)(&zgsz3555);
            CONVERT_OF(sail_int, mach_int)(&zgsz3555, zgaz3264);
            lbits zgsz3556;
            CREATE(lbits)(&zgsz3556);
            shiftr(&zgsz3556, zgsz3554, zgsz3555);
            zgaz3265 = CONVERT_OF(fbits, lbits)(zgsz3556, true);
            KILL(lbits)(&zgsz3556);
            KILL(sail_int)(&zgsz3555);
            KILL(lbits)(&zgsz3554);
          }
        }
        int64_t zgaz3266;
        {
          sail_int zgsz3563;
          CREATE(sail_int)(&zgsz3563);
          CONVERT_OF(sail_int, mach_int)(&zgsz3563, zcap_mantissa_width);
          sail_int zgsz3564;
          CREATE(sail_int)(&zgsz3564);
          CONVERT_OF(sail_int, mach_int)(&zgsz3564, INT64_C(3));
          sail_int zgsz3565;
          CREATE(sail_int)(&zgsz3565);
          sub_int(&zgsz3565, zgsz3563, zgsz3564);
          zgaz3266 = CONVERT_OF(mach_int, sail_int)(zgsz3565);
          KILL(sail_int)(&zgsz3565);
          KILL(sail_int)(&zgsz3564);
          KILL(sail_int)(&zgsz3563);
        }
        {
          lbits zgsz3560;
          CREATE(lbits)(&zgsz3560);
          CONVERT_OF(lbits, fbits)(&zgsz3560, zgaz3265, UINT64_C(32) , true);
          sail_int zgsz3561;
          CREATE(sail_int)(&zgsz3561);
          CONVERT_OF(sail_int, mach_int)(&zgsz3561, zgaz3266);
          lbits zgsz3562;
          CREATE(lbits)(&zgsz3562);
          sail_truncate(&zgsz3562, zgsz3560, zgsz3561);
          zB_ie = CONVERT_OF(fbits, lbits)(zgsz3562, true);
          KILL(lbits)(&zgsz3562);
          KILL(sail_int)(&zgsz3561);
          KILL(lbits)(&zgsz3560);
        }
      }
      uint64_t zT_ie;
      {
        uint64_t zgaz3262;
        {
          int64_t zgaz3261;
          {
            sail_int zgsz3569;
            CREATE(sail_int)(&zgsz3569);
            CONVERT_OF(sail_int, mach_int)(&zgsz3569, ze);
            sail_int zgsz3570;
            CREATE(sail_int)(&zgsz3570);
            CONVERT_OF(sail_int, mach_int)(&zgsz3570, INT64_C(3));
            sail_int zgsz3571;
            CREATE(sail_int)(&zgsz3571);
            add_int(&zgsz3571, zgsz3569, zgsz3570);
            zgaz3261 = CONVERT_OF(mach_int, sail_int)(zgsz3571);
            KILL(sail_int)(&zgsz3571);
            KILL(sail_int)(&zgsz3570);
            KILL(sail_int)(&zgsz3569);
          }
          {
            lbits zgsz3566;
            CREATE(lbits)(&zgsz3566);
            CONVERT_OF(lbits, fbits)(&zgsz3566, ztop, UINT64_C(33) , true);
            sail_int zgsz3567;
            CREATE(sail_int)(&zgsz3567);
            CONVERT_OF(sail_int, mach_int)(&zgsz3567, zgaz3261);
            lbits zgsz3568;
            CREATE(lbits)(&zgsz3568);
            shiftr(&zgsz3568, zgsz3566, zgsz3567);
            zgaz3262 = CONVERT_OF(fbits, lbits)(zgsz3568, true);
            KILL(lbits)(&zgsz3568);
            KILL(sail_int)(&zgsz3567);
            KILL(lbits)(&zgsz3566);
          }
        }
        int64_t zgaz3263;
        {
          sail_int zgsz3575;
          CREATE(sail_int)(&zgsz3575);
          CONVERT_OF(sail_int, mach_int)(&zgsz3575, zcap_mantissa_width);
          sail_int zgsz3576;
          CREATE(sail_int)(&zgsz3576);
          CONVERT_OF(sail_int, mach_int)(&zgsz3576, INT64_C(3));
          sail_int zgsz3577;
          CREATE(sail_int)(&zgsz3577);
          sub_int(&zgsz3577, zgsz3575, zgsz3576);
          zgaz3263 = CONVERT_OF(mach_int, sail_int)(zgsz3577);
          KILL(sail_int)(&zgsz3577);
          KILL(sail_int)(&zgsz3576);
          KILL(sail_int)(&zgsz3575);
        }
        {
          lbits zgsz3572;
          CREATE(lbits)(&zgsz3572);
          CONVERT_OF(lbits, fbits)(&zgsz3572, zgaz3262, UINT64_C(33) , true);
          sail_int zgsz3573;
          CREATE(sail_int)(&zgsz3573);
          CONVERT_OF(sail_int, mach_int)(&zgsz3573, zgaz3263);
          lbits zgsz3574;
          CREATE(lbits)(&zgsz3574);
          sail_truncate(&zgsz3574, zgsz3572, zgsz3573);
          zT_ie = CONVERT_OF(fbits, lbits)(zgsz3574, true);
          KILL(lbits)(&zgsz3574);
          KILL(sail_int)(&zgsz3573);
          KILL(lbits)(&zgsz3572);
        }
      }
      uint64_t zmaskLo;
      {
        int64_t zgaz3259;
        {
          sail_int zgsz3586;
          CREATE(sail_int)(&zgsz3586);
          CONVERT_OF(sail_int, mach_int)(&zgsz3586, INT64_C(32));
          sail_int zgsz3587;
          CREATE(sail_int)(&zgsz3587);
          CONVERT_OF(sail_int, mach_int)(&zgsz3587, INT64_C(1));
          sail_int zgsz3588;
          CREATE(sail_int)(&zgsz3588);
          add_int(&zgsz3588, zgsz3586, zgsz3587);
          zgaz3259 = CONVERT_OF(mach_int, sail_int)(zgsz3588);
          KILL(sail_int)(&zgsz3588);
          KILL(sail_int)(&zgsz3587);
          KILL(sail_int)(&zgsz3586);
        }
        sbits zgaz3260;
        {
          int64_t zgaz3258;
          {
            sail_int zgsz3580;
            CREATE(sail_int)(&zgsz3580);
            CONVERT_OF(sail_int, mach_int)(&zgsz3580, ze);
            sail_int zgsz3581;
            CREATE(sail_int)(&zgsz3581);
            CONVERT_OF(sail_int, mach_int)(&zgsz3581, INT64_C(3));
            sail_int zgsz3582;
            CREATE(sail_int)(&zgsz3582);
            add_int(&zgsz3582, zgsz3580, zgsz3581);
            zgaz3258 = CONVERT_OF(mach_int, sail_int)(zgsz3582);
            KILL(sail_int)(&zgsz3582);
            KILL(sail_int)(&zgsz3581);
            KILL(sail_int)(&zgsz3580);
          }
          {
            sail_int zgsz3578;
            CREATE(sail_int)(&zgsz3578);
            CONVERT_OF(sail_int, mach_int)(&zgsz3578, zgaz3258);
            lbits zgsz3579;
            CREATE(lbits)(&zgsz3579);
            sailgen_ones(&zgsz3579, zgsz3578);
            zgaz3260 = CONVERT_OF(sbits, lbits)(zgsz3579, true);
            KILL(lbits)(&zgsz3579);
            KILL(sail_int)(&zgsz3578);
          }
        }
        {
          sail_int zgsz3583;
          CREATE(sail_int)(&zgsz3583);
          CONVERT_OF(sail_int, mach_int)(&zgsz3583, zgaz3259);
          lbits zgsz3584;
          CREATE(lbits)(&zgsz3584);
          CONVERT_OF(lbits, sbits)(&zgsz3584, zgaz3260, true);
          lbits zgsz3585;
          CREATE(lbits)(&zgsz3585);
          sailgen_EXTZ(&zgsz3585, zgsz3583, zgsz3584);
          zmaskLo = CONVERT_OF(fbits, lbits)(zgsz3585, true);
          KILL(lbits)(&zgsz3585);
          KILL(lbits)(&zgsz3584);
          KILL(sail_int)(&zgsz3583);
        }
      }
      {
        int64_t zgaz3243;
        {
          uint64_t zgaz3242;
          {
            lbits zgsz3591;
            CREATE(lbits)(&zgsz3591);
            CONVERT_OF(lbits, fbits)(&zgsz3591, zext_base, UINT64_C(33) , true);
            lbits zgsz3592;
            CREATE(lbits)(&zgsz3592);
            CONVERT_OF(lbits, fbits)(&zgsz3592, zmaskLo, UINT64_C(33) , true);
            lbits zgsz3593;
            CREATE(lbits)(&zgsz3593);
            and_bits(&zgsz3593, zgsz3591, zgsz3592);
            zgaz3242 = CONVERT_OF(fbits, lbits)(zgsz3593, true);
            KILL(lbits)(&zgsz3593);
            KILL(lbits)(&zgsz3592);
            KILL(lbits)(&zgsz3591);
          }
          {
            lbits zgsz3589;
            CREATE(lbits)(&zgsz3589);
            CONVERT_OF(lbits, fbits)(&zgsz3589, zgaz3242, UINT64_C(33) , true);
            sail_int zgsz3590;
            CREATE(sail_int)(&zgsz3590);
            sail_unsigned(&zgsz3590, zgsz3589);
            zgaz3243 = CONVERT_OF(mach_int, sail_int)(zgsz3590);
            KILL(sail_int)(&zgsz3590);
            KILL(lbits)(&zgsz3589);
          }
        }
        {
          sail_int zgsz3594;
          CREATE(sail_int)(&zgsz3594);
          CONVERT_OF(sail_int, mach_int)(&zgsz3594, zgaz3243);
          sail_int zgsz3595;
          CREATE(sail_int)(&zgsz3595);
          CONVERT_OF(sail_int, mach_int)(&zgsz3595, INT64_C(0));
          zlostSignificantBase = sailgen_neq_int(zgsz3594, zgsz3595);
          KILL(sail_int)(&zgsz3595);
          KILL(sail_int)(&zgsz3594);
        }
        unit zgsz358;
        zgsz358 = UNIT;
      }
      {
        int64_t zgaz3245;
        {
          uint64_t zgaz3244;
          {
            lbits zgsz3598;
            CREATE(lbits)(&zgsz3598);
            CONVERT_OF(lbits, fbits)(&zgsz3598, ztop, UINT64_C(33) , true);
            lbits zgsz3599;
            CREATE(lbits)(&zgsz3599);
            CONVERT_OF(lbits, fbits)(&zgsz3599, zmaskLo, UINT64_C(33) , true);
            lbits zgsz3600;
            CREATE(lbits)(&zgsz3600);
            and_bits(&zgsz3600, zgsz3598, zgsz3599);
            zgaz3244 = CONVERT_OF(fbits, lbits)(zgsz3600, true);
            KILL(lbits)(&zgsz3600);
            KILL(lbits)(&zgsz3599);
            KILL(lbits)(&zgsz3598);
          }
          {
            lbits zgsz3596;
            CREATE(lbits)(&zgsz3596);
            CONVERT_OF(lbits, fbits)(&zgsz3596, zgaz3244, UINT64_C(33) , true);
            sail_int zgsz3597;
            CREATE(sail_int)(&zgsz3597);
            sail_unsigned(&zgsz3597, zgsz3596);
            zgaz3245 = CONVERT_OF(mach_int, sail_int)(zgsz3597);
            KILL(sail_int)(&zgsz3597);
            KILL(lbits)(&zgsz3596);
          }
        }
        {
          sail_int zgsz3601;
          CREATE(sail_int)(&zgsz3601);
          CONVERT_OF(sail_int, mach_int)(&zgsz3601, zgaz3245);
          sail_int zgsz3602;
          CREATE(sail_int)(&zgsz3602);
          CONVERT_OF(sail_int, mach_int)(&zgsz3602, INT64_C(0));
          zlostSignificantTop = sailgen_neq_int(zgsz3601, zgsz3602);
          KILL(sail_int)(&zgsz3602);
          KILL(sail_int)(&zgsz3601);
        }
        unit zgsz357;
        zgsz357 = UNIT;
      }
      {
        unit zgsz356;
        if (zlostSignificantTop) {
          {
            lbits zgsz3603;
            CREATE(lbits)(&zgsz3603);
            CONVERT_OF(lbits, fbits)(&zgsz3603, zT_ie, UINT64_C(5) , true);
            sail_int zgsz3604;
            CREATE(sail_int)(&zgsz3604);
            CONVERT_OF(sail_int, mach_int)(&zgsz3604, INT64_C(1));
            lbits zgsz3605;
            CREATE(lbits)(&zgsz3605);
            add_bits_int(&zgsz3605, zgsz3603, zgsz3604);
            zT_ie = CONVERT_OF(fbits, lbits)(zgsz3605, true);
            KILL(lbits)(&zgsz3605);
            KILL(sail_int)(&zgsz3604);
            KILL(lbits)(&zgsz3603);
          }
          zgsz356 = UNIT;
        } else {  zgsz356 = UNIT;  }
      }
      uint64_t zlen_ie;
      {
        lbits zgsz3648;
        CREATE(lbits)(&zgsz3648);
        CONVERT_OF(lbits, fbits)(&zgsz3648, zT_ie, UINT64_C(5) , true);
        lbits zgsz3649;
        CREATE(lbits)(&zgsz3649);
        CONVERT_OF(lbits, fbits)(&zgsz3649, zB_ie, UINT64_C(5) , true);
        lbits zgsz3650;
        CREATE(lbits)(&zgsz3650);
        sub_bits(&zgsz3650, zgsz3648, zgsz3649);
        zlen_ie = CONVERT_OF(fbits, lbits)(zgsz3650, true);
        KILL(lbits)(&zgsz3650);
        KILL(lbits)(&zgsz3649);
        KILL(lbits)(&zgsz3648);
      }
      {
        bool zgaz3248;
        {
          fbits zgaz3247;
          {
            int64_t zgaz3246;
            {
              sail_int zgsz3608;
              CREATE(sail_int)(&zgsz3608);
              CONVERT_OF(sail_int, mach_int)(&zgsz3608, zcap_mantissa_width);
              sail_int zgsz3609;
              CREATE(sail_int)(&zgsz3609);
              CONVERT_OF(sail_int, mach_int)(&zgsz3609, INT64_C(4));
              sail_int zgsz3610;
              CREATE(sail_int)(&zgsz3610);
              sub_int(&zgsz3610, zgsz3608, zgsz3609);
              zgaz3246 = CONVERT_OF(mach_int, sail_int)(zgsz3610);
              KILL(sail_int)(&zgsz3610);
              KILL(sail_int)(&zgsz3609);
              KILL(sail_int)(&zgsz3608);
            }
            {
              lbits zgsz3606;
              CREATE(lbits)(&zgsz3606);
              CONVERT_OF(lbits, fbits)(&zgsz3606, zlen_ie, UINT64_C(5) , true);
              sail_int zgsz3607;
              CREATE(sail_int)(&zgsz3607);
              CONVERT_OF(sail_int, mach_int)(&zgsz3607, zgaz3246);
              zgaz3247 = bitvector_access(zgsz3606, zgsz3607);
              KILL(sail_int)(&zgsz3607);
              KILL(lbits)(&zgsz3606);
            }
          }
          zgaz3248 = eq_bit(zgaz3247, UINT64_C(1));
        }
        unit zgsz360;
        if (zgaz3248) {
          {
            zincE = true;
            unit zgsz366;
            zgsz366 = UNIT;
          }
          {
            bool zgsz361;
            if (zlostSignificantBase) {  zgsz361 = true;  } else {
              fbits zgaz3249;
              {
                lbits zgsz3611;
                CREATE(lbits)(&zgsz3611);
                CONVERT_OF(lbits, fbits)(&zgsz3611, zB_ie, UINT64_C(5) , true);
                sail_int zgsz3612;
                CREATE(sail_int)(&zgsz3612);
                CONVERT_OF(sail_int, mach_int)(&zgsz3612, INT64_C(0));
                zgaz3249 = bitvector_access(zgsz3611, zgsz3612);
                KILL(sail_int)(&zgsz3612);
                KILL(lbits)(&zgsz3611);
              }
              zgsz361 = eq_bit(zgaz3249, UINT64_C(1));
            }
            zlostSignificantBase = zgsz361;
            unit zgsz365;
            zgsz365 = UNIT;
          }
          {
            bool zgsz362;
            if (zlostSignificantTop) {  zgsz362 = true;  } else {
              fbits zgaz3250;
              {
                lbits zgsz3613;
                CREATE(lbits)(&zgsz3613);
                CONVERT_OF(lbits, fbits)(&zgsz3613, zT_ie, UINT64_C(5) , true);
                sail_int zgsz3614;
                CREATE(sail_int)(&zgsz3614);
                CONVERT_OF(sail_int, mach_int)(&zgsz3614, INT64_C(0));
                zgaz3250 = bitvector_access(zgsz3613, zgsz3614);
                KILL(sail_int)(&zgsz3614);
                KILL(lbits)(&zgsz3613);
              }
              zgsz362 = eq_bit(zgaz3250, UINT64_C(1));
            }
            zlostSignificantTop = zgsz362;
            unit zgsz364;
            zgsz364 = UNIT;
          }
          {
            uint64_t zgaz3252;
            {
              int64_t zgaz3251;
              {
                sail_int zgsz3618;
                CREATE(sail_int)(&zgsz3618);
                CONVERT_OF(sail_int, mach_int)(&zgsz3618, ze);
                sail_int zgsz3619;
                CREATE(sail_int)(&zgsz3619);
                CONVERT_OF(sail_int, mach_int)(&zgsz3619, INT64_C(4));
                sail_int zgsz3620;
                CREATE(sail_int)(&zgsz3620);
                add_int(&zgsz3620, zgsz3618, zgsz3619);
                zgaz3251 = CONVERT_OF(mach_int, sail_int)(zgsz3620);
                KILL(sail_int)(&zgsz3620);
                KILL(sail_int)(&zgsz3619);
                KILL(sail_int)(&zgsz3618);
              }
              {
                lbits zgsz3615;
                CREATE(lbits)(&zgsz3615);
                CONVERT_OF(lbits, fbits)(&zgsz3615, zbase, UINT64_C(32) , true);
                sail_int zgsz3616;
                CREATE(sail_int)(&zgsz3616);
                CONVERT_OF(sail_int, mach_int)(&zgsz3616, zgaz3251);
                lbits zgsz3617;
                CREATE(lbits)(&zgsz3617);
                shiftr(&zgsz3617, zgsz3615, zgsz3616);
                zgaz3252 = CONVERT_OF(fbits, lbits)(zgsz3617, true);
                KILL(lbits)(&zgsz3617);
                KILL(sail_int)(&zgsz3616);
                KILL(lbits)(&zgsz3615);
              }
            }
            int64_t zgaz3253;
            {
              sail_int zgsz3624;
              CREATE(sail_int)(&zgsz3624);
              CONVERT_OF(sail_int, mach_int)(&zgsz3624, zcap_mantissa_width);
              sail_int zgsz3625;
              CREATE(sail_int)(&zgsz3625);
              CONVERT_OF(sail_int, mach_int)(&zgsz3625, INT64_C(3));
              sail_int zgsz3626;
              CREATE(sail_int)(&zgsz3626);
              sub_int(&zgsz3626, zgsz3624, zgsz3625);
              zgaz3253 = CONVERT_OF(mach_int, sail_int)(zgsz3626);
              KILL(sail_int)(&zgsz3626);
              KILL(sail_int)(&zgsz3625);
              KILL(sail_int)(&zgsz3624);
            }
            {
              lbits zgsz3621;
              CREATE(lbits)(&zgsz3621);
              CONVERT_OF(lbits, fbits)(&zgsz3621, zgaz3252, UINT64_C(32) , true);
              sail_int zgsz3622;
              CREATE(sail_int)(&zgsz3622);
              CONVERT_OF(sail_int, mach_int)(&zgsz3622, zgaz3253);
              lbits zgsz3623;
              CREATE(lbits)(&zgsz3623);
              sail_truncate(&zgsz3623, zgsz3621, zgsz3622);
              zB_ie = CONVERT_OF(fbits, lbits)(zgsz3623, true);
              KILL(lbits)(&zgsz3623);
              KILL(sail_int)(&zgsz3622);
              KILL(lbits)(&zgsz3621);
            }
            unit zgsz363;
            zgsz363 = UNIT;
          }
          int64_t zincT;
          if (zlostSignificantTop) {  zincT = INT64_C(1);  } else {  zincT = INT64_C(0);  }
          uint64_t zgaz3257;
          {
            uint64_t zgaz3255;
            {
              int64_t zgaz3254;
              {
                sail_int zgsz3630;
                CREATE(sail_int)(&zgsz3630);
                CONVERT_OF(sail_int, mach_int)(&zgsz3630, ze);
                sail_int zgsz3631;
                CREATE(sail_int)(&zgsz3631);
                CONVERT_OF(sail_int, mach_int)(&zgsz3631, INT64_C(4));
                sail_int zgsz3632;
                CREATE(sail_int)(&zgsz3632);
                add_int(&zgsz3632, zgsz3630, zgsz3631);
                zgaz3254 = CONVERT_OF(mach_int, sail_int)(zgsz3632);
                KILL(sail_int)(&zgsz3632);
                KILL(sail_int)(&zgsz3631);
                KILL(sail_int)(&zgsz3630);
              }
              {
                lbits zgsz3627;
                CREATE(lbits)(&zgsz3627);
                CONVERT_OF(lbits, fbits)(&zgsz3627, ztop, UINT64_C(33) , true);
                sail_int zgsz3628;
                CREATE(sail_int)(&zgsz3628);
                CONVERT_OF(sail_int, mach_int)(&zgsz3628, zgaz3254);
                lbits zgsz3629;
                CREATE(lbits)(&zgsz3629);
                shiftr(&zgsz3629, zgsz3627, zgsz3628);
                zgaz3255 = CONVERT_OF(fbits, lbits)(zgsz3629, true);
                KILL(lbits)(&zgsz3629);
                KILL(sail_int)(&zgsz3628);
                KILL(lbits)(&zgsz3627);
              }
            }
            int64_t zgaz3256;
            {
              sail_int zgsz3636;
              CREATE(sail_int)(&zgsz3636);
              CONVERT_OF(sail_int, mach_int)(&zgsz3636, zcap_mantissa_width);
              sail_int zgsz3637;
              CREATE(sail_int)(&zgsz3637);
              CONVERT_OF(sail_int, mach_int)(&zgsz3637, INT64_C(3));
              sail_int zgsz3638;
              CREATE(sail_int)(&zgsz3638);
              sub_int(&zgsz3638, zgsz3636, zgsz3637);
              zgaz3256 = CONVERT_OF(mach_int, sail_int)(zgsz3638);
              KILL(sail_int)(&zgsz3638);
              KILL(sail_int)(&zgsz3637);
              KILL(sail_int)(&zgsz3636);
            }
            {
              lbits zgsz3633;
              CREATE(lbits)(&zgsz3633);
              CONVERT_OF(lbits, fbits)(&zgsz3633, zgaz3255, UINT64_C(33) , true);
              sail_int zgsz3634;
              CREATE(sail_int)(&zgsz3634);
              CONVERT_OF(sail_int, mach_int)(&zgsz3634, zgaz3256);
              lbits zgsz3635;
              CREATE(lbits)(&zgsz3635);
              sail_truncate(&zgsz3635, zgsz3633, zgsz3634);
              zgaz3257 = CONVERT_OF(fbits, lbits)(zgsz3635, true);
              KILL(lbits)(&zgsz3635);
              KILL(sail_int)(&zgsz3634);
              KILL(lbits)(&zgsz3633);
            }
          }
          {
            lbits zgsz3639;
            CREATE(lbits)(&zgsz3639);
            CONVERT_OF(lbits, fbits)(&zgsz3639, zgaz3257, UINT64_C(5) , true);
            sail_int zgsz3640;
            CREATE(sail_int)(&zgsz3640);
            CONVERT_OF(sail_int, mach_int)(&zgsz3640, zincT);
            lbits zgsz3641;
            CREATE(lbits)(&zgsz3641);
            add_bits_int(&zgsz3641, zgsz3639, zgsz3640);
            zT_ie = CONVERT_OF(fbits, lbits)(zgsz3641, true);
            KILL(lbits)(&zgsz3641);
            KILL(sail_int)(&zgsz3640);
            KILL(lbits)(&zgsz3639);
          }
          zgsz360 = UNIT;
        } else {  zgsz360 = UNIT;  }
      }
      {
        {
          lbits zgsz3642;
          CREATE(lbits)(&zgsz3642);
          CONVERT_OF(lbits, fbits)(&zgsz3642, zB_ie, UINT64_C(5) , true);
          lbits zgsz3643;
          CREATE(lbits)(&zgsz3643);
          CONVERT_OF(lbits, fbits)(&zgsz3643, UINT64_C(0b000), UINT64_C(3) , true);
          lbits zgsz3644;
          CREATE(lbits)(&zgsz3644);
          append(&zgsz3644, zgsz3642, zgsz3643);
          zBbits = CONVERT_OF(fbits, lbits)(zgsz3644, true);
          KILL(lbits)(&zgsz3644);
          KILL(lbits)(&zgsz3643);
          KILL(lbits)(&zgsz3642);
        }
        unit zgsz359;
        zgsz359 = UNIT;
      }
      {
        lbits zgsz3645;
        CREATE(lbits)(&zgsz3645);
        CONVERT_OF(lbits, fbits)(&zgsz3645, zT_ie, UINT64_C(5) , true);
        lbits zgsz3646;
        CREATE(lbits)(&zgsz3646);
        CONVERT_OF(lbits, fbits)(&zgsz3646, UINT64_C(0b000), UINT64_C(3) , true);
        lbits zgsz3647;
        CREATE(lbits)(&zgsz3647);
        append(&zgsz3647, zgsz3645, zgsz3646);
        zTbits = CONVERT_OF(fbits, lbits)(zgsz3647, true);
        KILL(lbits)(&zgsz3647);
        KILL(lbits)(&zgsz3646);
        KILL(lbits)(&zgsz3645);
      }
      zgsz355 = UNIT;
    } else {  zgsz355 = UNIT;  }
  }
  struct zCapability znewCap;
  {
    uint64_t zgaz3269;
    {
      int64_t zgaz3268;
      if (zincE) {
        {
          sail_int zgsz3651;
          CREATE(sail_int)(&zgsz3651);
          CONVERT_OF(sail_int, mach_int)(&zgsz3651, ze);
          sail_int zgsz3652;
          CREATE(sail_int)(&zgsz3652);
          CONVERT_OF(sail_int, mach_int)(&zgsz3652, INT64_C(1));
          sail_int zgsz3653;
          CREATE(sail_int)(&zgsz3653);
          add_int(&zgsz3653, zgsz3651, zgsz3652);
          zgaz3268 = CONVERT_OF(mach_int, sail_int)(zgsz3653);
          KILL(sail_int)(&zgsz3653);
          KILL(sail_int)(&zgsz3652);
          KILL(sail_int)(&zgsz3651);
        }
      } else {  zgaz3268 = ze;  }
      {
        sail_int zgsz3654;
        CREATE(sail_int)(&zgsz3654);
        CONVERT_OF(sail_int, mach_int)(&zgsz3654, INT64_C(6));
        sail_int zgsz3655;
        CREATE(sail_int)(&zgsz3655);
        CONVERT_OF(sail_int, mach_int)(&zgsz3655, zgaz3268);
        lbits zgsz3656;
        CREATE(lbits)(&zgsz3656);
        sailgen_to_bits(&zgsz3656, zgsz3654, zgsz3655);
        zgaz3269 = CONVERT_OF(fbits, lbits)(zgsz3656, true);
        KILL(lbits)(&zgsz3656);
        KILL(sail_int)(&zgsz3655);
        KILL(sail_int)(&zgsz3654);
      }
    }
    struct zCapability zgsz367;
    zgsz367 = zcap;
    zgsz367.zB = zBbits;
    zgsz367.zE = zgaz3269;
    zgsz367.zT = zTbits;
    zgsz367.zaddress = zbase;
    zgsz367.zinternal_E = zie;
    znewCap = zgsz367;
  }
  bool zexact;
  {
    bool zgaz3267;
    {
      bool zgsz368;
      if (zlostSignificantBase) {  zgsz368 = true;  } else {  zgsz368 = zlostSignificantTop;  }
      zgaz3267 = zgsz368;
    }
    zexact = not(zgaz3267);
  }
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz369;
  zgsz369.ztup0 = zexact;
  zgsz369.ztup1 = znewCap;
  zcbz326 = zgsz369;












end_function_116: ;
  return zcbz326;
end_block_exception_117: ;
  struct zCapability zcbz355 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz354 = { .ztup1 = zcbz355, .ztup0 = false };
  return zcbz354;
}

static uint64_t sailgen_getCapPerms(struct zCapability);

static uint64_t sailgen_getCapPerms(struct zCapability zcap)
{
  __label__ end_function_119, end_block_exception_120;

  uint64_t zcbz327;
  uint64_t zperms;
  {
    uint64_t zgaz3277;
    zgaz3277 = sailgen_getCapHardPerms(zcap);
    {
      sail_int zgsz3669;
      CREATE(sail_int)(&zgsz3669);
      CONVERT_OF(sail_int, mach_int)(&zgsz3669, INT64_C(15));
      lbits zgsz3670;
      CREATE(lbits)(&zgsz3670);
      CONVERT_OF(lbits, fbits)(&zgsz3670, zgaz3277, UINT64_C(12) , true);
      lbits zgsz3671;
      CREATE(lbits)(&zgsz3671);
      sailgen_EXTZ(&zgsz3671, zgsz3669, zgsz3670);
      zperms = CONVERT_OF(fbits, lbits)(zgsz3671, true);
      KILL(lbits)(&zgsz3671);
      KILL(lbits)(&zgsz3670);
      KILL(sail_int)(&zgsz3669);
    }
  }
  uint64_t zgaz3276;
  zgaz3276 = zcap.zuperms;
  {
    lbits zgsz3672;
    CREATE(lbits)(&zgsz3672);
    CONVERT_OF(lbits, fbits)(&zgsz3672, zgaz3276, UINT64_C(0) , true);
    lbits zgsz3673;
    CREATE(lbits)(&zgsz3673);
    CONVERT_OF(lbits, fbits)(&zgsz3673, zperms, UINT64_C(15) , true);
    lbits zgsz3674;
    CREATE(lbits)(&zgsz3674);
    append(&zgsz3674, zgsz3672, zgsz3673);
    zcbz327 = CONVERT_OF(fbits, lbits)(zgsz3674, true);
    KILL(lbits)(&zgsz3674);
    KILL(lbits)(&zgsz3673);
    KILL(lbits)(&zgsz3672);
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
  uint64_t zgaz3279;
  {
    uint64_t zgaz3278;
    {
      lbits zgsz3678;
      CREATE(lbits)(&zgsz3678);
      CONVERT_OF(lbits, fbits)(&zgsz3678, zperms, UINT64_C(15) , true);
      sail_int zgsz3679;
      CREATE(sail_int)(&zgsz3679);
      CONVERT_OF(sail_int, mach_int)(&zgsz3679, zcap_uperms_shift);
      lbits zgsz3680;
      CREATE(lbits)(&zgsz3680);
      shiftr(&zgsz3680, zgsz3678, zgsz3679);
      zgaz3278 = CONVERT_OF(fbits, lbits)(zgsz3680, true);
      KILL(lbits)(&zgsz3680);
      KILL(sail_int)(&zgsz3679);
      KILL(lbits)(&zgsz3678);
    }
    {
      lbits zgsz3675;
      CREATE(lbits)(&zgsz3675);
      CONVERT_OF(lbits, fbits)(&zgsz3675, zgaz3278, UINT64_C(15) , true);
      sail_int zgsz3676;
      CREATE(sail_int)(&zgsz3676);
      CONVERT_OF(sail_int, mach_int)(&zgsz3676, zcap_uperms_width);
      lbits zgsz3677;
      CREATE(lbits)(&zgsz3677);
      sail_truncate(&zgsz3677, zgsz3675, zgsz3676);
      zgaz3279 = CONVERT_OF(fbits, lbits)(zgsz3677, true);
      KILL(lbits)(&zgsz3677);
      KILL(sail_int)(&zgsz3676);
      KILL(lbits)(&zgsz3675);
    }
  }
  bool zgaz3281;
  {
    fbits zgaz3280;
    {
      lbits zgsz3681;
      CREATE(lbits)(&zgsz3681);
      CONVERT_OF(lbits, fbits)(&zgsz3681, zperms, UINT64_C(15) , true);
      sail_int zgsz3682;
      CREATE(sail_int)(&zgsz3682);
      CONVERT_OF(sail_int, mach_int)(&zgsz3682, INT64_C(11));
      zgaz3280 = bitvector_access(zgsz3681, zgsz3682);
      KILL(sail_int)(&zgsz3682);
      KILL(lbits)(&zgsz3681);
    }
    zgaz3281 = sailgen_bit_to_bool(zgaz3280);
  }
  bool zgaz3283;
  {
    fbits zgaz3282;
    {
      lbits zgsz3683;
      CREATE(lbits)(&zgsz3683);
      CONVERT_OF(lbits, fbits)(&zgsz3683, zperms, UINT64_C(15) , true);
      sail_int zgsz3684;
      CREATE(sail_int)(&zgsz3684);
      CONVERT_OF(sail_int, mach_int)(&zgsz3684, INT64_C(10));
      zgaz3282 = bitvector_access(zgsz3683, zgsz3684);
      KILL(sail_int)(&zgsz3684);
      KILL(lbits)(&zgsz3683);
    }
    zgaz3283 = sailgen_bit_to_bool(zgaz3282);
  }
  bool zgaz3285;
  {
    fbits zgaz3284;
    {
      lbits zgsz3685;
      CREATE(lbits)(&zgsz3685);
      CONVERT_OF(lbits, fbits)(&zgsz3685, zperms, UINT64_C(15) , true);
      sail_int zgsz3686;
      CREATE(sail_int)(&zgsz3686);
      CONVERT_OF(sail_int, mach_int)(&zgsz3686, INT64_C(9));
      zgaz3284 = bitvector_access(zgsz3685, zgsz3686);
      KILL(sail_int)(&zgsz3686);
      KILL(lbits)(&zgsz3685);
    }
    zgaz3285 = sailgen_bit_to_bool(zgaz3284);
  }
  bool zgaz3287;
  {
    fbits zgaz3286;
    {
      lbits zgsz3687;
      CREATE(lbits)(&zgsz3687);
      CONVERT_OF(lbits, fbits)(&zgsz3687, zperms, UINT64_C(15) , true);
      sail_int zgsz3688;
      CREATE(sail_int)(&zgsz3688);
      CONVERT_OF(sail_int, mach_int)(&zgsz3688, INT64_C(8));
      zgaz3286 = bitvector_access(zgsz3687, zgsz3688);
      KILL(sail_int)(&zgsz3688);
      KILL(lbits)(&zgsz3687);
    }
    zgaz3287 = sailgen_bit_to_bool(zgaz3286);
  }
  bool zgaz3289;
  {
    fbits zgaz3288;
    {
      lbits zgsz3689;
      CREATE(lbits)(&zgsz3689);
      CONVERT_OF(lbits, fbits)(&zgsz3689, zperms, UINT64_C(15) , true);
      sail_int zgsz3690;
      CREATE(sail_int)(&zgsz3690);
      CONVERT_OF(sail_int, mach_int)(&zgsz3690, INT64_C(7));
      zgaz3288 = bitvector_access(zgsz3689, zgsz3690);
      KILL(sail_int)(&zgsz3690);
      KILL(lbits)(&zgsz3689);
    }
    zgaz3289 = sailgen_bit_to_bool(zgaz3288);
  }
  bool zgaz3291;
  {
    fbits zgaz3290;
    {
      lbits zgsz3691;
      CREATE(lbits)(&zgsz3691);
      CONVERT_OF(lbits, fbits)(&zgsz3691, zperms, UINT64_C(15) , true);
      sail_int zgsz3692;
      CREATE(sail_int)(&zgsz3692);
      CONVERT_OF(sail_int, mach_int)(&zgsz3692, INT64_C(6));
      zgaz3290 = bitvector_access(zgsz3691, zgsz3692);
      KILL(sail_int)(&zgsz3692);
      KILL(lbits)(&zgsz3691);
    }
    zgaz3291 = sailgen_bit_to_bool(zgaz3290);
  }
  bool zgaz3293;
  {
    fbits zgaz3292;
    {
      lbits zgsz3693;
      CREATE(lbits)(&zgsz3693);
      CONVERT_OF(lbits, fbits)(&zgsz3693, zperms, UINT64_C(15) , true);
      sail_int zgsz3694;
      CREATE(sail_int)(&zgsz3694);
      CONVERT_OF(sail_int, mach_int)(&zgsz3694, INT64_C(5));
      zgaz3292 = bitvector_access(zgsz3693, zgsz3694);
      KILL(sail_int)(&zgsz3694);
      KILL(lbits)(&zgsz3693);
    }
    zgaz3293 = sailgen_bit_to_bool(zgaz3292);
  }
  bool zgaz3295;
  {
    fbits zgaz3294;
    {
      lbits zgsz3695;
      CREATE(lbits)(&zgsz3695);
      CONVERT_OF(lbits, fbits)(&zgsz3695, zperms, UINT64_C(15) , true);
      sail_int zgsz3696;
      CREATE(sail_int)(&zgsz3696);
      CONVERT_OF(sail_int, mach_int)(&zgsz3696, INT64_C(4));
      zgaz3294 = bitvector_access(zgsz3695, zgsz3696);
      KILL(sail_int)(&zgsz3696);
      KILL(lbits)(&zgsz3695);
    }
    zgaz3295 = sailgen_bit_to_bool(zgaz3294);
  }
  bool zgaz3297;
  {
    fbits zgaz3296;
    {
      lbits zgsz3697;
      CREATE(lbits)(&zgsz3697);
      CONVERT_OF(lbits, fbits)(&zgsz3697, zperms, UINT64_C(15) , true);
      sail_int zgsz3698;
      CREATE(sail_int)(&zgsz3698);
      CONVERT_OF(sail_int, mach_int)(&zgsz3698, INT64_C(3));
      zgaz3296 = bitvector_access(zgsz3697, zgsz3698);
      KILL(sail_int)(&zgsz3698);
      KILL(lbits)(&zgsz3697);
    }
    zgaz3297 = sailgen_bit_to_bool(zgaz3296);
  }
  bool zgaz3299;
  {
    fbits zgaz3298;
    {
      lbits zgsz3699;
      CREATE(lbits)(&zgsz3699);
      CONVERT_OF(lbits, fbits)(&zgsz3699, zperms, UINT64_C(15) , true);
      sail_int zgsz3700;
      CREATE(sail_int)(&zgsz3700);
      CONVERT_OF(sail_int, mach_int)(&zgsz3700, INT64_C(2));
      zgaz3298 = bitvector_access(zgsz3699, zgsz3700);
      KILL(sail_int)(&zgsz3700);
      KILL(lbits)(&zgsz3699);
    }
    zgaz3299 = sailgen_bit_to_bool(zgaz3298);
  }
  bool zgaz3301;
  {
    fbits zgaz3300;
    {
      lbits zgsz3701;
      CREATE(lbits)(&zgsz3701);
      CONVERT_OF(lbits, fbits)(&zgsz3701, zperms, UINT64_C(15) , true);
      sail_int zgsz3702;
      CREATE(sail_int)(&zgsz3702);
      CONVERT_OF(sail_int, mach_int)(&zgsz3702, INT64_C(1));
      zgaz3300 = bitvector_access(zgsz3701, zgsz3702);
      KILL(sail_int)(&zgsz3702);
      KILL(lbits)(&zgsz3701);
    }
    zgaz3301 = sailgen_bit_to_bool(zgaz3300);
  }
  bool zgaz3303;
  {
    fbits zgaz3302;
    {
      lbits zgsz3703;
      CREATE(lbits)(&zgsz3703);
      CONVERT_OF(lbits, fbits)(&zgsz3703, zperms, UINT64_C(15) , true);
      sail_int zgsz3704;
      CREATE(sail_int)(&zgsz3704);
      CONVERT_OF(sail_int, mach_int)(&zgsz3704, INT64_C(0));
      zgaz3302 = bitvector_access(zgsz3703, zgsz3704);
      KILL(sail_int)(&zgsz3704);
      KILL(lbits)(&zgsz3703);
    }
    zgaz3303 = sailgen_bit_to_bool(zgaz3302);
  }
  struct zCapability zgsz370;
  zgsz370 = zcap;
  zgsz370.zaccess_system_regs = zgaz3283;
  zgsz370.zglobal = zgaz3303;
  zgsz370.zpermit_cinvoke = zgaz3287;
  zgsz370.zpermit_execute = zgaz3301;
  zgsz370.zpermit_load = zgaz3299;
  zgsz370.zpermit_load_cap = zgaz3295;
  zgsz370.zpermit_seal = zgaz3289;
  zgsz370.zpermit_set_CID = zgaz3281;
  zgsz370.zpermit_store = zgaz3297;
  zgsz370.zpermit_store_cap = zgaz3293;
  zgsz370.zpermit_store_local_cap = zgaz3291;
  zgsz370.zpermit_unseal = zgaz3285;
  zgsz370.zuperms = zgaz3279;
  zcbz328 = zgsz370;














end_function_122: ;
  return zcbz328;
end_block_exception_123: ;
  struct zCapability zcbz356 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz356;
}

static uint64_t sailgen_getCapFlags(struct zCapability);

static uint64_t sailgen_getCapFlags(struct zCapability zcap)
{
  __label__ end_function_125, end_block_exception_126;

  uint64_t zcbz329;
  bool zgaz3304;
  zgaz3304 = zcap.zflag_cap_mode;
  zcbz329 = sailgen_bool_to_bits(zgaz3304);

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
  int64_t zgaz3306;
  {
    uint64_t zgaz3305;
    zgaz3305 = zcap.zotype;
    {
      lbits zgsz3705;
      CREATE(lbits)(&zgsz3705);
      CONVERT_OF(lbits, fbits)(&zgsz3705, zgaz3305, UINT64_C(4) , true);
      sail_int zgsz3706;
      CREATE(sail_int)(&zgsz3706);
      sail_signed(&zgsz3706, zgsz3705);
      zgaz3306 = CONVERT_OF(mach_int, sail_int)(zgsz3706);
      KILL(sail_int)(&zgsz3706);
      KILL(lbits)(&zgsz3705);
    }
  }
  {
    sail_int zgsz3707;
    CREATE(sail_int)(&zgsz3707);
    CONVERT_OF(sail_int, mach_int)(&zgsz3707, zgaz3306);
    sail_int zgsz3708;
    CREATE(sail_int)(&zgsz3708);
    CONVERT_OF(sail_int, mach_int)(&zgsz3708, zotype_unsealed);
    zcbz330 = sailgen_neq_int(zgsz3707, zgsz3708);
    KILL(sail_int)(&zgsz3708);
    KILL(sail_int)(&zgsz3707);
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
  int64_t zgaz3308;
  {
    uint64_t zgaz3307;
    zgaz3307 = zcap.zotype;
    {
      lbits zgsz3709;
      CREATE(lbits)(&zgsz3709);
      CONVERT_OF(lbits, fbits)(&zgsz3709, zgaz3307, UINT64_C(4) , true);
      sail_int zgsz3710;
      CREATE(sail_int)(&zgsz3710);
      sail_unsigned(&zgsz3710, zgsz3709);
      zgaz3308 = CONVERT_OF(mach_int, sail_int)(zgsz3710);
      KILL(sail_int)(&zgsz3710);
      KILL(lbits)(&zgsz3709);
    }
  }
  {
    sail_int zgsz3711;
    CREATE(sail_int)(&zgsz3711);
    CONVERT_OF(sail_int, mach_int)(&zgsz3711, zgaz3308);
    sail_int zgsz3712;
    CREATE(sail_int)(&zgsz3712);
    CONVERT_OF(sail_int, mach_int)(&zgsz3712, zcap_max_otype);
    zcbz331 = gt(zgsz3711, zgsz3712);
    KILL(sail_int)(&zgsz3712);
    KILL(sail_int)(&zgsz3711);
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
  struct ztuple_z8z5bv32zCz0z5bv33z9 zgaz3309;
  zgaz3309 = sailgen_getCapBoundsBits(zc);
  uint64_t zgsz371;
  {
    uint64_t zbase;
    zbase = zgaz3309.ztup0;
    zgsz371 = zbase;
    goto finish_match_134;
  }
case_135: ;
  sail_match_failure("getCapBaseBits");
finish_match_134: ;
  zcbz332 = zgsz371;


end_function_136: ;
  return zcbz332;
end_block_exception_137: ;

  return UINT64_C(0xdeadc0de);
}

static int64_t sailgen_getCapLength(struct zCapability);

static int64_t sailgen_getCapLength(struct zCapability zc)
{
  __label__ case_140, finish_match_139, end_function_141, end_block_exception_142;

  int64_t zcbz333;
  struct ztuple_z8z5i64zCz0z5i64z9 zgaz3310;
  zgaz3310 = sailgen_getCapBounds(zc);
  int64_t zgsz373;
  {
    int64_t zbase;
    zbase = zgaz3310.ztup0;
    int64_t ztop;
    ztop = zgaz3310.ztup1;
    {
      bool zgaz3313;
      {
        bool zgaz3312;
        {
          bool zgaz3311;
          zgaz3311 = zc.ztag;
          zgaz3312 = not(zgaz3311);
        }
        bool zgsz374;
        if (zgaz3312) {  zgsz374 = true;  } else {
          {
            sail_int zgsz3713;
            CREATE(sail_int)(&zgsz3713);
            CONVERT_OF(sail_int, mach_int)(&zgsz3713, ztop);
            sail_int zgsz3714;
            CREATE(sail_int)(&zgsz3714);
            CONVERT_OF(sail_int, mach_int)(&zgsz3714, zbase);
            zgsz374 = gteq(zgsz3713, zgsz3714);
            KILL(sail_int)(&zgsz3714);
            KILL(sail_int)(&zgsz3713);
          }
        }
        zgaz3313 = zgsz374;
      }
      unit zgsz375;
      zgsz375 = sail_assert(zgaz3313, "src/cheri_cap_common.sail:453.40-453.41");
    }
    int64_t zgaz3314;
    {
      sail_int zgsz3720;
      CREATE(sail_int)(&zgsz3720);
      CONVERT_OF(sail_int, mach_int)(&zgsz3720, ztop);
      sail_int zgsz3721;
      CREATE(sail_int)(&zgsz3721);
      CONVERT_OF(sail_int, mach_int)(&zgsz3721, zbase);
      sail_int zgsz3722;
      CREATE(sail_int)(&zgsz3722);
      sub_int(&zgsz3722, zgsz3720, zgsz3721);
      zgaz3314 = CONVERT_OF(mach_int, sail_int)(zgsz3722);
      KILL(sail_int)(&zgsz3722);
      KILL(sail_int)(&zgsz3721);
      KILL(sail_int)(&zgsz3720);
    }
    int64_t zgaz3315;
    {
      sail_int zgsz3718;
      CREATE(sail_int)(&zgsz3718);
      CONVERT_OF(sail_int, mach_int)(&zgsz3718, zcap_len_width);
      sail_int zgsz3719;
      CREATE(sail_int)(&zgsz3719);
      pow2(&zgsz3719, zgsz3718);
      zgaz3315 = CONVERT_OF(mach_int, sail_int)(zgsz3719);
      KILL(sail_int)(&zgsz3719);
      KILL(sail_int)(&zgsz3718);
    }
    {
      sail_int zgsz3715;
      CREATE(sail_int)(&zgsz3715);
      CONVERT_OF(sail_int, mach_int)(&zgsz3715, zgaz3314);
      sail_int zgsz3716;
      CREATE(sail_int)(&zgsz3716);
      CONVERT_OF(sail_int, mach_int)(&zgsz3716, zgaz3315);
      sail_int zgsz3717;
      CREATE(sail_int)(&zgsz3717);
      emod_int(&zgsz3717, zgsz3715, zgsz3716);
      zgsz373 = CONVERT_OF(mach_int, sail_int)(zgsz3717);
      KILL(sail_int)(&zgsz3717);
      KILL(sail_int)(&zgsz3716);
      KILL(sail_int)(&zgsz3715);
    }
    goto finish_match_139;
  }
case_140: ;
  sail_match_failure("getCapLength");
finish_match_139: ;
  zcbz333 = zgsz373;


end_function_141: ;
  return zcbz333;
end_block_exception_142: ;

  return INT64_C(0xdeadc0de);
}

static bool sailgen_inCapBounds(struct zCapability, uint64_t, int64_t);

static bool sailgen_inCapBounds(struct zCapability zcap, uint64_t zaddr, int64_t zsizze)
{
  __label__ case_145, finish_match_144, end_function_146, end_block_exception_147;

  bool zcbz334;
  struct ztuple_z8z5i64zCz0z5i64z9 zgaz3316;
  zgaz3316 = sailgen_getCapBounds(zcap);
  bool zgsz377;
  {
    int64_t zbase;
    zbase = zgaz3316.ztup0;
    int64_t ztop;
    ztop = zgaz3316.ztup1;
    int64_t za;
    {
      lbits zgsz3730;
      CREATE(lbits)(&zgsz3730);
      CONVERT_OF(lbits, fbits)(&zgsz3730, zaddr, UINT64_C(32) , true);
      sail_int zgsz3731;
      CREATE(sail_int)(&zgsz3731);
      sail_unsigned(&zgsz3731, zgsz3730);
      za = CONVERT_OF(mach_int, sail_int)(zgsz3731);
      KILL(sail_int)(&zgsz3731);
      KILL(lbits)(&zgsz3730);
    }
    bool zgaz3318;
    {
      sail_int zgsz3728;
      CREATE(sail_int)(&zgsz3728);
      CONVERT_OF(sail_int, mach_int)(&zgsz3728, za);
      sail_int zgsz3729;
      CREATE(sail_int)(&zgsz3729);
      CONVERT_OF(sail_int, mach_int)(&zgsz3729, zbase);
      zgaz3318 = gteq(zgsz3728, zgsz3729);
      KILL(sail_int)(&zgsz3729);
      KILL(sail_int)(&zgsz3728);
    }
    bool zgsz378;
    if (zgaz3318) {
      int64_t zgaz3317;
      {
        sail_int zgsz3725;
        CREATE(sail_int)(&zgsz3725);
        CONVERT_OF(sail_int, mach_int)(&zgsz3725, za);
        sail_int zgsz3726;
        CREATE(sail_int)(&zgsz3726);
        CONVERT_OF(sail_int, mach_int)(&zgsz3726, zsizze);
        sail_int zgsz3727;
        CREATE(sail_int)(&zgsz3727);
        add_int(&zgsz3727, zgsz3725, zgsz3726);
        zgaz3317 = CONVERT_OF(mach_int, sail_int)(zgsz3727);
        KILL(sail_int)(&zgsz3727);
        KILL(sail_int)(&zgsz3726);
        KILL(sail_int)(&zgsz3725);
      }
      {
        sail_int zgsz3723;
        CREATE(sail_int)(&zgsz3723);
        CONVERT_OF(sail_int, mach_int)(&zgsz3723, zgaz3317);
        sail_int zgsz3724;
        CREATE(sail_int)(&zgsz3724);
        CONVERT_OF(sail_int, mach_int)(&zgsz3724, ztop);
        zgsz378 = lteq(zgsz3723, zgsz3724);
        KILL(sail_int)(&zgsz3724);
        KILL(sail_int)(&zgsz3723);
      }
    } else {  zgsz378 = false;  }
    zgsz377 = zgsz378;
    goto finish_match_144;
  }
case_145: ;
  sail_match_failure("inCapBounds");
finish_match_144: ;
  zcbz334 = zgsz377;


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
  bool zgaz3320;
  {
    bool zgaz3319;
    zgaz3319 = zcap.ztag;
    bool zgsz380;
    if (zgaz3319) {  zgsz380 = not(zcond);  } else {  zgsz380 = false;  }
    zgaz3320 = zgsz380;
  }
  struct zCapability zgsz381;
  zgsz381 = zcap;
  zgsz381.ztag = zgaz3320;
  zcbz335 = zgsz381;


end_function_149: ;
  return zcbz335;
end_block_exception_150: ;
  struct zCapability zcbz357 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz357;
}

static struct zCapability sailgen_clearTagIfSealed(struct zCapability);

static struct zCapability sailgen_clearTagIfSealed(struct zCapability zcap)
{
  __label__ end_function_152, end_block_exception_153;

  struct zCapability zcbz336;
  bool zgaz3321;
  zgaz3321 = sailgen_isCapSealed(zcap);
  zcbz336 = sailgen_clearTagIf(zcap, zgaz3321);

end_function_152: ;
  return zcbz336;
end_block_exception_153: ;
  struct zCapability zcbz358 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  return zcbz358;
}

static bool sailgen_capBoundsEqual(struct zCapability, struct zCapability);

static bool sailgen_capBoundsEqual(struct zCapability zc1, struct zCapability zc2)
{
  __label__ case_156, finish_match_155, end_function_159, end_block_exception_160;

  bool zcbz337;
  struct ztuple_z8z5i64zCz0z5i64z9 zgaz3322;
  zgaz3322 = sailgen_getCapBounds(zc1);
  bool zgsz382;
  {
    __label__ case_158, finish_match_157;

    int64_t zbase1;
    zbase1 = zgaz3322.ztup0;
    int64_t ztop1;
    ztop1 = zgaz3322.ztup1;
    struct ztuple_z8z5i64zCz0z5i64z9 zgaz3323;
    zgaz3323 = sailgen_getCapBounds(zc2);
    bool zgsz383;
    {
      int64_t zbase2;
      zbase2 = zgaz3323.ztup0;
      int64_t ztop2;
      ztop2 = zgaz3323.ztup1;
      bool zgaz3324;
      {
        sail_int zgsz3734;
        CREATE(sail_int)(&zgsz3734);
        CONVERT_OF(sail_int, mach_int)(&zgsz3734, zbase1);
        sail_int zgsz3735;
        CREATE(sail_int)(&zgsz3735);
        CONVERT_OF(sail_int, mach_int)(&zgsz3735, zbase2);
        zgaz3324 = eq_int(zgsz3734, zgsz3735);
        KILL(sail_int)(&zgsz3735);
        KILL(sail_int)(&zgsz3734);
      }
      bool zgsz384;
      if (zgaz3324) {
        {
          sail_int zgsz3732;
          CREATE(sail_int)(&zgsz3732);
          CONVERT_OF(sail_int, mach_int)(&zgsz3732, ztop1);
          sail_int zgsz3733;
          CREATE(sail_int)(&zgsz3733);
          CONVERT_OF(sail_int, mach_int)(&zgsz3733, ztop2);
          zgsz384 = eq_int(zgsz3732, zgsz3733);
          KILL(sail_int)(&zgsz3733);
          KILL(sail_int)(&zgsz3732);
        }
      } else {  zgsz384 = false;  }
      zgsz383 = zgsz384;
      goto finish_match_157;
    }
  case_158: ;
    sail_match_failure("capBoundsEqual");
  finish_match_157: ;
    zgsz382 = zgsz383;
    goto finish_match_155;
  }
case_156: ;
  sail_match_failure("capBoundsEqual");
finish_match_155: ;
  zcbz337 = zgsz382;


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
  struct zCapability zcbz360 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz359 = { .ztup1 = zcbz360, .ztup0 = false };
  return zcbz359;
}

static bool sailgen_fastRepCheck(struct zCapability, uint64_t);

static bool sailgen_fastRepCheck(struct zCapability zc, uint64_t zi)
{
  __label__ end_function_165, end_block_exception_166;

  bool zcbz339;
  int64_t zE;
  {
    uint64_t zgaz3338;
    zgaz3338 = zc.zE;
    {
      lbits zgsz3736;
      CREATE(lbits)(&zgsz3736);
      CONVERT_OF(lbits, fbits)(&zgsz3736, zgaz3338, UINT64_C(6) , true);
      sail_int zgsz3737;
      CREATE(sail_int)(&zgsz3737);
      sail_unsigned(&zgsz3737, zgsz3736);
      zE = CONVERT_OF(mach_int, sail_int)(zgsz3737);
      KILL(sail_int)(&zgsz3737);
      KILL(lbits)(&zgsz3736);
    }
  }
  bool zgaz3326;
  {
    int64_t zgaz3325;
    {
      sail_int zgsz3740;
      CREATE(sail_int)(&zgsz3740);
      CONVERT_OF(sail_int, mach_int)(&zgsz3740, zcap_max_E);
      sail_int zgsz3741;
      CREATE(sail_int)(&zgsz3741);
      CONVERT_OF(sail_int, mach_int)(&zgsz3741, INT64_C(2));
      sail_int zgsz3742;
      CREATE(sail_int)(&zgsz3742);
      sub_int(&zgsz3742, zgsz3740, zgsz3741);
      zgaz3325 = CONVERT_OF(mach_int, sail_int)(zgsz3742);
      KILL(sail_int)(&zgsz3742);
      KILL(sail_int)(&zgsz3741);
      KILL(sail_int)(&zgsz3740);
    }
    {
      sail_int zgsz3738;
      CREATE(sail_int)(&zgsz3738);
      CONVERT_OF(sail_int, mach_int)(&zgsz3738, zE);
      sail_int zgsz3739;
      CREATE(sail_int)(&zgsz3739);
      CONVERT_OF(sail_int, mach_int)(&zgsz3739, zgaz3325);
      zgaz3326 = gteq(zgsz3738, zgsz3739);
      KILL(sail_int)(&zgsz3739);
      KILL(sail_int)(&zgsz3738);
    }
  }
  if (zgaz3326) {  zcbz339 = true;  } else {
    int64_t zi_top;
    {
      uint64_t zgaz3337;
      {
        int64_t zgaz3336;
        {
          sail_int zgsz3746;
          CREATE(sail_int)(&zgsz3746);
          CONVERT_OF(sail_int, mach_int)(&zgsz3746, zE);
          sail_int zgsz3747;
          CREATE(sail_int)(&zgsz3747);
          CONVERT_OF(sail_int, mach_int)(&zgsz3747, zcap_mantissa_width);
          sail_int zgsz3748;
          CREATE(sail_int)(&zgsz3748);
          add_int(&zgsz3748, zgsz3746, zgsz3747);
          zgaz3336 = CONVERT_OF(mach_int, sail_int)(zgsz3748);
          KILL(sail_int)(&zgsz3748);
          KILL(sail_int)(&zgsz3747);
          KILL(sail_int)(&zgsz3746);
        }
        {
          lbits zgsz3743;
          CREATE(lbits)(&zgsz3743);
          CONVERT_OF(lbits, fbits)(&zgsz3743, zi, UINT64_C(32) , true);
          sail_int zgsz3744;
          CREATE(sail_int)(&zgsz3744);
          CONVERT_OF(sail_int, mach_int)(&zgsz3744, zgaz3336);
          lbits zgsz3745;
          CREATE(lbits)(&zgsz3745);
          arith_shiftr(&zgsz3745, zgsz3743, zgsz3744);
          zgaz3337 = CONVERT_OF(fbits, lbits)(zgsz3745, true);
          KILL(lbits)(&zgsz3745);
          KILL(sail_int)(&zgsz3744);
          KILL(lbits)(&zgsz3743);
        }
      }
      {
        lbits zgsz3749;
        CREATE(lbits)(&zgsz3749);
        CONVERT_OF(lbits, fbits)(&zgsz3749, zgaz3337, UINT64_C(32) , true);
        sail_int zgsz3750;
        CREATE(sail_int)(&zgsz3750);
        sail_signed(&zgsz3750, zgsz3749);
        zi_top = CONVERT_OF(mach_int, sail_int)(zgsz3750);
        KILL(sail_int)(&zgsz3750);
        KILL(lbits)(&zgsz3749);
      }
    }
    uint64_t zi_mid;
    {
      uint64_t zgaz3335;
      {
        lbits zgsz3754;
        CREATE(lbits)(&zgsz3754);
        CONVERT_OF(lbits, fbits)(&zgsz3754, zi, UINT64_C(32) , true);
        sail_int zgsz3755;
        CREATE(sail_int)(&zgsz3755);
        CONVERT_OF(sail_int, mach_int)(&zgsz3755, zE);
        lbits zgsz3756;
        CREATE(lbits)(&zgsz3756);
        shiftr(&zgsz3756, zgsz3754, zgsz3755);
        zgaz3335 = CONVERT_OF(fbits, lbits)(zgsz3756, true);
        KILL(lbits)(&zgsz3756);
        KILL(sail_int)(&zgsz3755);
        KILL(lbits)(&zgsz3754);
      }
      {
        lbits zgsz3751;
        CREATE(lbits)(&zgsz3751);
        CONVERT_OF(lbits, fbits)(&zgsz3751, zgaz3335, UINT64_C(32) , true);
        sail_int zgsz3752;
        CREATE(sail_int)(&zgsz3752);
        CONVERT_OF(sail_int, mach_int)(&zgsz3752, zcap_mantissa_width);
        lbits zgsz3753;
        CREATE(lbits)(&zgsz3753);
        sail_truncate(&zgsz3753, zgsz3751, zgsz3752);
        zi_mid = CONVERT_OF(fbits, lbits)(zgsz3753, true);
        KILL(lbits)(&zgsz3753);
        KILL(sail_int)(&zgsz3752);
        KILL(lbits)(&zgsz3751);
      }
    }
    uint64_t za_mid;
    {
      uint64_t zgaz3334;
      {
        uint64_t zgaz3333;
        zgaz3333 = zc.zaddress;
        {
          lbits zgsz3757;
          CREATE(lbits)(&zgsz3757);
          CONVERT_OF(lbits, fbits)(&zgsz3757, zgaz3333, UINT64_C(32) , true);
          sail_int zgsz3758;
          CREATE(sail_int)(&zgsz3758);
          CONVERT_OF(sail_int, mach_int)(&zgsz3758, zE);
          lbits zgsz3759;
          CREATE(lbits)(&zgsz3759);
          shiftr(&zgsz3759, zgsz3757, zgsz3758);
          zgaz3334 = CONVERT_OF(fbits, lbits)(zgsz3759, true);
          KILL(lbits)(&zgsz3759);
          KILL(sail_int)(&zgsz3758);
          KILL(lbits)(&zgsz3757);
        }
      }
      {
        lbits zgsz3760;
        CREATE(lbits)(&zgsz3760);
        CONVERT_OF(lbits, fbits)(&zgsz3760, zgaz3334, UINT64_C(32) , true);
        sail_int zgsz3761;
        CREATE(sail_int)(&zgsz3761);
        CONVERT_OF(sail_int, mach_int)(&zgsz3761, zcap_mantissa_width);
        lbits zgsz3762;
        CREATE(lbits)(&zgsz3762);
        sail_truncate(&zgsz3762, zgsz3760, zgsz3761);
        za_mid = CONVERT_OF(fbits, lbits)(zgsz3762, true);
        KILL(lbits)(&zgsz3762);
        KILL(sail_int)(&zgsz3761);
        KILL(lbits)(&zgsz3760);
      }
    }
    uint64_t zB3;
    {
      uint64_t zgaz3332;
      zgaz3332 = zc.zB;
      {
        lbits zgsz3763;
        CREATE(lbits)(&zgsz3763);
        CONVERT_OF(lbits, fbits)(&zgsz3763, zgaz3332, UINT64_C(8) , true);
        sail_int zgsz3764;
        CREATE(sail_int)(&zgsz3764);
        CONVERT_OF(sail_int, mach_int)(&zgsz3764, INT64_C(3));
        lbits zgsz3765;
        CREATE(lbits)(&zgsz3765);
        sail_truncateLSB(&zgsz3765, zgsz3763, zgsz3764);
        zB3 = CONVERT_OF(fbits, lbits)(zgsz3765, true);
        KILL(lbits)(&zgsz3765);
        KILL(sail_int)(&zgsz3764);
        KILL(lbits)(&zgsz3763);
      }
    }
    uint64_t zR3;
    {
      lbits zgsz3790;
      CREATE(lbits)(&zgsz3790);
      CONVERT_OF(lbits, fbits)(&zgsz3790, zB3, UINT64_C(3) , true);
      lbits zgsz3791;
      CREATE(lbits)(&zgsz3791);
      CONVERT_OF(lbits, fbits)(&zgsz3791, UINT64_C(0b001), UINT64_C(3) , true);
      lbits zgsz3792;
      CREATE(lbits)(&zgsz3792);
      sub_bits(&zgsz3792, zgsz3790, zgsz3791);
      zR3 = CONVERT_OF(fbits, lbits)(zgsz3792, true);
      KILL(lbits)(&zgsz3792);
      KILL(lbits)(&zgsz3791);
      KILL(lbits)(&zgsz3790);
    }
    uint64_t zR;
    {
      uint64_t zgaz3331;
      {
        int64_t zgaz3330;
        {
          sail_int zgsz3768;
          CREATE(sail_int)(&zgsz3768);
          CONVERT_OF(sail_int, mach_int)(&zgsz3768, zcap_mantissa_width);
          sail_int zgsz3769;
          CREATE(sail_int)(&zgsz3769);
          CONVERT_OF(sail_int, mach_int)(&zgsz3769, INT64_C(3));
          sail_int zgsz3770;
          CREATE(sail_int)(&zgsz3770);
          sub_int(&zgsz3770, zgsz3768, zgsz3769);
          zgaz3330 = CONVERT_OF(mach_int, sail_int)(zgsz3770);
          KILL(sail_int)(&zgsz3770);
          KILL(sail_int)(&zgsz3769);
          KILL(sail_int)(&zgsz3768);
        }
        {
          sail_int zgsz3766;
          CREATE(sail_int)(&zgsz3766);
          CONVERT_OF(sail_int, mach_int)(&zgsz3766, zgaz3330);
          lbits zgsz3767;
          CREATE(lbits)(&zgsz3767);
          sailgen_zzeros_implicit(&zgsz3767, zgsz3766);
          zgaz3331 = CONVERT_OF(fbits, lbits)(zgsz3767, true);
          KILL(lbits)(&zgsz3767);
          KILL(sail_int)(&zgsz3766);
        }
      }
      {
        lbits zgsz3771;
        CREATE(lbits)(&zgsz3771);
        CONVERT_OF(lbits, fbits)(&zgsz3771, zR3, UINT64_C(3) , true);
        lbits zgsz3772;
        CREATE(lbits)(&zgsz3772);
        CONVERT_OF(lbits, fbits)(&zgsz3772, zgaz3331, UINT64_C(5) , true);
        lbits zgsz3773;
        CREATE(lbits)(&zgsz3773);
        append(&zgsz3773, zgsz3771, zgsz3772);
        zR = CONVERT_OF(fbits, lbits)(zgsz3773, true);
        KILL(lbits)(&zgsz3773);
        KILL(lbits)(&zgsz3772);
        KILL(lbits)(&zgsz3771);
      }
    }
    uint64_t zdiff;
    {
      lbits zgsz3787;
      CREATE(lbits)(&zgsz3787);
      CONVERT_OF(lbits, fbits)(&zgsz3787, zR, UINT64_C(8) , true);
      lbits zgsz3788;
      CREATE(lbits)(&zgsz3788);
      CONVERT_OF(lbits, fbits)(&zgsz3788, za_mid, UINT64_C(8) , true);
      lbits zgsz3789;
      CREATE(lbits)(&zgsz3789);
      sub_bits(&zgsz3789, zgsz3787, zgsz3788);
      zdiff = CONVERT_OF(fbits, lbits)(zgsz3789, true);
      KILL(lbits)(&zgsz3789);
      KILL(lbits)(&zgsz3788);
      KILL(lbits)(&zgsz3787);
    }
    uint64_t zdiff1;
    {
      lbits zgsz3784;
      CREATE(lbits)(&zgsz3784);
      CONVERT_OF(lbits, fbits)(&zgsz3784, zdiff, UINT64_C(8) , true);
      sail_int zgsz3785;
      CREATE(sail_int)(&zgsz3785);
      CONVERT_OF(sail_int, mach_int)(&zgsz3785, INT64_C(1));
      lbits zgsz3786;
      CREATE(lbits)(&zgsz3786);
      sub_bits_int(&zgsz3786, zgsz3784, zgsz3785);
      zdiff1 = CONVERT_OF(fbits, lbits)(zgsz3786, true);
      KILL(lbits)(&zgsz3786);
      KILL(sail_int)(&zgsz3785);
      KILL(lbits)(&zgsz3784);
    }
    bool zgaz3327;
    {
      sail_int zgsz3782;
      CREATE(sail_int)(&zgsz3782);
      CONVERT_OF(sail_int, mach_int)(&zgsz3782, zi_top);
      sail_int zgsz3783;
      CREATE(sail_int)(&zgsz3783);
      CONVERT_OF(sail_int, mach_int)(&zgsz3783, INT64_C(0));
      zgaz3327 = eq_int(zgsz3782, zgsz3783);
      KILL(sail_int)(&zgsz3783);
      KILL(sail_int)(&zgsz3782);
    }
    if (zgaz3327) {
      {
        lbits zgsz3780;
        CREATE(lbits)(&zgsz3780);
        CONVERT_OF(lbits, fbits)(&zgsz3780, zi_mid, UINT64_C(8) , true);
        lbits zgsz3781;
        CREATE(lbits)(&zgsz3781);
        CONVERT_OF(lbits, fbits)(&zgsz3781, zdiff1, UINT64_C(8) , true);
        zcbz339 = sailgen_z8operatorz0zI_uz9(zgsz3780, zgsz3781);
        KILL(lbits)(&zgsz3781);
        KILL(lbits)(&zgsz3780);
      }
    } else {
      bool zgaz3328;
      {
        sail_int zgsz3778;
        CREATE(sail_int)(&zgsz3778);
        CONVERT_OF(sail_int, mach_int)(&zgsz3778, zi_top);
        sail_int zgsz3779;
        CREATE(sail_int)(&zgsz3779);
        CONVERT_OF(sail_int, mach_int)(&zgsz3779, INT64_C(-1));
        zgaz3328 = eq_int(zgsz3778, zgsz3779);
        KILL(sail_int)(&zgsz3779);
        KILL(sail_int)(&zgsz3778);
      }
      if (zgaz3328) {
        bool zgaz3329;
        {
          lbits zgsz3776;
          CREATE(lbits)(&zgsz3776);
          CONVERT_OF(lbits, fbits)(&zgsz3776, zi_mid, UINT64_C(8) , true);
          lbits zgsz3777;
          CREATE(lbits)(&zgsz3777);
          CONVERT_OF(lbits, fbits)(&zgsz3777, zdiff, UINT64_C(8) , true);
          zgaz3329 = sailgen_z8operatorz0zKzJ_uz9(zgsz3776, zgsz3777);
          KILL(lbits)(&zgsz3777);
          KILL(lbits)(&zgsz3776);
        }
        bool zgsz389;
        if (zgaz3329) {
          {
            lbits zgsz3774;
            CREATE(lbits)(&zgsz3774);
            CONVERT_OF(lbits, fbits)(&zgsz3774, zR, UINT64_C(8) , true);
            lbits zgsz3775;
            CREATE(lbits)(&zgsz3775);
            CONVERT_OF(lbits, fbits)(&zgsz3775, za_mid, UINT64_C(8) , true);
            zgsz389 = neq_bits(zgsz3774, zgsz3775);
            KILL(lbits)(&zgsz3775);
            KILL(lbits)(&zgsz3774);
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

  int64_t zlen;
  zlen = sailgen_getCapLength(zcap);
  sail_string zlen_str;
  CREATE(sail_string)(&zlen_str);
  {
    uint64_t zgaz3367;
    {
      int64_t zgaz3366;
      {
        sail_int zgsz3796;
        CREATE(sail_int)(&zgsz3796);
        CONVERT_OF(sail_int, mach_int)(&zgsz3796, zcap_len_width);
        sail_int zgsz3797;
        CREATE(sail_int)(&zgsz3797);
        CONVERT_OF(sail_int, mach_int)(&zgsz3797, INT64_C(3));
        sail_int zgsz3798;
        CREATE(sail_int)(&zgsz3798);
        add_int(&zgsz3798, zgsz3796, zgsz3797);
        zgaz3366 = CONVERT_OF(mach_int, sail_int)(zgsz3798);
        KILL(sail_int)(&zgsz3798);
        KILL(sail_int)(&zgsz3797);
        KILL(sail_int)(&zgsz3796);
      }
      {
        sail_int zgsz3793;
        CREATE(sail_int)(&zgsz3793);
        CONVERT_OF(sail_int, mach_int)(&zgsz3793, zgaz3366);
        sail_int zgsz3794;
        CREATE(sail_int)(&zgsz3794);
        CONVERT_OF(sail_int, mach_int)(&zgsz3794, zlen);
        lbits zgsz3795;
        CREATE(lbits)(&zgsz3795);
        sailgen_to_bits(&zgsz3795, zgsz3793, zgsz3794);
        zgaz3367 = CONVERT_OF(fbits, lbits)(zgsz3795, true);
        KILL(lbits)(&zgsz3795);
        KILL(sail_int)(&zgsz3794);
        KILL(sail_int)(&zgsz3793);
      }
    }
    {
      lbits zgsz3799;
      CREATE(lbits)(&zgsz3799);
      CONVERT_OF(lbits, fbits)(&zgsz3799, zgaz3367, UINT64_C(36) , true);
      string_of_lbits(&zlen_str, zgsz3799);
      KILL(lbits)(&zgsz3799);
    }
  }
  uint64_t zotype64;
  {
    bool zgaz3363;
    zgaz3363 = sailgen_hasReservedOType(zcap);
    if (zgaz3363) {
      uint64_t zgaz3364;
      zgaz3364 = zcap.zotype;
      {
        sail_int zgsz3803;
        CREATE(sail_int)(&zgsz3803);
        CONVERT_OF(sail_int, mach_int)(&zgsz3803, INT64_C(32));
        lbits zgsz3804;
        CREATE(lbits)(&zgsz3804);
        CONVERT_OF(lbits, fbits)(&zgsz3804, zgaz3364, UINT64_C(4) , true);
        lbits zgsz3805;
        CREATE(lbits)(&zgsz3805);
        sailgen_EXTS(&zgsz3805, zgsz3803, zgsz3804);
        zotype64 = CONVERT_OF(fbits, lbits)(zgsz3805, true);
        KILL(lbits)(&zgsz3805);
        KILL(lbits)(&zgsz3804);
        KILL(sail_int)(&zgsz3803);
      }
    } else {
      uint64_t zgaz3365;
      zgaz3365 = zcap.zotype;
      {
        sail_int zgsz3800;
        CREATE(sail_int)(&zgsz3800);
        CONVERT_OF(sail_int, mach_int)(&zgsz3800, INT64_C(32));
        lbits zgsz3801;
        CREATE(lbits)(&zgsz3801);
        CONVERT_OF(lbits, fbits)(&zgsz3801, zgaz3365, UINT64_C(4) , true);
        lbits zgsz3802;
        CREATE(lbits)(&zgsz3802);
        sailgen_EXTZ(&zgsz3802, zgsz3800, zgsz3801);
        zotype64 = CONVERT_OF(fbits, lbits)(zgsz3802, true);
        KILL(lbits)(&zgsz3802);
        KILL(lbits)(&zgsz3801);
        KILL(sail_int)(&zgsz3800);
      }
    }
  }
  sail_string zgaz3362;
  CREATE(sail_string)(&zgaz3362);
  {
    sail_string zgaz3360;
    CREATE(sail_string)(&zgaz3360);
    {
      bool zgaz3339;
      zgaz3339 = zcap.ztag;
      if (zgaz3339) {  COPY(sail_string)(&zgaz3360, "1");  } else {  COPY(sail_string)(&zgaz3360, "0");  }
    }
    sail_string zgaz3361;
    CREATE(sail_string)(&zgaz3361);
    {
      sail_string zgaz3359;
      CREATE(sail_string)(&zgaz3359);
      {
        sail_string zgaz3357;
        CREATE(sail_string)(&zgaz3357);
        {
          bool zgaz3340;
          zgaz3340 = sailgen_isCapSealed(zcap);
          if (zgaz3340) {  COPY(sail_string)(&zgaz3357, "1");  } else {  COPY(sail_string)(&zgaz3357, "0");  }
        }
        sail_string zgaz3358;
        CREATE(sail_string)(&zgaz3358);
        {
          sail_string zgaz3356;
          CREATE(sail_string)(&zgaz3356);
          {
            sail_string zgaz3354;
            CREATE(sail_string)(&zgaz3354);
            {
              uint64_t zgaz3342;
              {
                uint64_t zgaz3341;
                zgaz3341 = sailgen_getCapPerms(zcap);
                {
                  lbits zgsz3806;
                  CREATE(lbits)(&zgsz3806);
                  CONVERT_OF(lbits, fbits)(&zgsz3806, UINT64_C(0b0), UINT64_C(1) , true);
                  lbits zgsz3807;
                  CREATE(lbits)(&zgsz3807);
                  CONVERT_OF(lbits, fbits)(&zgsz3807, zgaz3341, UINT64_C(15) , true);
                  lbits zgsz3808;
                  CREATE(lbits)(&zgsz3808);
                  append(&zgsz3808, zgsz3806, zgsz3807);
                  zgaz3342 = CONVERT_OF(fbits, lbits)(zgsz3808, true);
                  KILL(lbits)(&zgsz3808);
                  KILL(lbits)(&zgsz3807);
                  KILL(lbits)(&zgsz3806);
                }
              }
              {
                lbits zgsz3809;
                CREATE(lbits)(&zgsz3809);
                CONVERT_OF(lbits, fbits)(&zgsz3809, zgaz3342, UINT64_C(16) , true);
                string_of_lbits(&zgaz3354, zgsz3809);
                KILL(lbits)(&zgsz3809);
              }
            }
            sail_string zgaz3355;
            CREATE(sail_string)(&zgaz3355);
            {
              sail_string zgaz3353;
              CREATE(sail_string)(&zgaz3353);
              {
                sail_string zgaz3351;
                CREATE(sail_string)(&zgaz3351);
                {
                  lbits zgsz3812;
                  CREATE(lbits)(&zgsz3812);
                  CONVERT_OF(lbits, fbits)(&zgsz3812, zotype64, UINT64_C(32) , true);
                  string_of_lbits(&zgaz3351, zgsz3812);
                  KILL(lbits)(&zgsz3812);
                }
                sail_string zgaz3352;
                CREATE(sail_string)(&zgaz3352);
                {
                  sail_string zgaz3350;
                  CREATE(sail_string)(&zgaz3350);
                  {
                    sail_string zgaz3348;
                    CREATE(sail_string)(&zgaz3348);
                    {
                      uint64_t zgaz3343;
                      zgaz3343 = zcap.zaddress;
                      {
                        lbits zgsz3810;
                        CREATE(lbits)(&zgsz3810);
                        CONVERT_OF(lbits, fbits)(&zgsz3810, zgaz3343, UINT64_C(32) , true);
                        string_of_lbits(&zgaz3348, zgsz3810);
                        KILL(lbits)(&zgsz3810);
                      }
                    }
                    sail_string zgaz3349;
                    CREATE(sail_string)(&zgaz3349);
                    {
                      sail_string zgaz3347;
                      CREATE(sail_string)(&zgaz3347);
                      {
                        sail_string zgaz3345;
                        CREATE(sail_string)(&zgaz3345);
                        {
                          uint64_t zgaz3344;
                          zgaz3344 = sailgen_getCapBaseBits(zcap);
                          {
                            lbits zgsz3811;
                            CREATE(lbits)(&zgsz3811);
                            CONVERT_OF(lbits, fbits)(&zgsz3811, zgaz3344, UINT64_C(32) , true);
                            string_of_lbits(&zgaz3345, zgsz3811);
                            KILL(lbits)(&zgsz3811);
                          }
                        }
                        sail_string zgaz3346;
                        CREATE(sail_string)(&zgaz3346);
                        concat_str(&zgaz3346, " length:", zlen_str);
                        concat_str(&zgaz3347, zgaz3345, zgaz3346);
                        KILL(sail_string)(&zgaz3346);
                        KILL(sail_string)(&zgaz3345);
                      }
                      concat_str(&zgaz3349, " base:", zgaz3347);
                      KILL(sail_string)(&zgaz3347);
                    }
                    concat_str(&zgaz3350, zgaz3348, zgaz3349);
                    KILL(sail_string)(&zgaz3349);
                    KILL(sail_string)(&zgaz3348);
                  }
                  concat_str(&zgaz3352, " address:", zgaz3350);
                  KILL(sail_string)(&zgaz3350);
                }
                concat_str(&zgaz3353, zgaz3351, zgaz3352);
                KILL(sail_string)(&zgaz3352);
                KILL(sail_string)(&zgaz3351);
              }
              concat_str(&zgaz3355, " type:", zgaz3353);
              KILL(sail_string)(&zgaz3353);
            }
            concat_str(&zgaz3356, zgaz3354, zgaz3355);
            KILL(sail_string)(&zgaz3355);
            KILL(sail_string)(&zgaz3354);
          }
          concat_str(&zgaz3358, " perms:", zgaz3356);
          KILL(sail_string)(&zgaz3356);
        }
        concat_str(&zgaz3359, zgaz3357, zgaz3358);
        KILL(sail_string)(&zgaz3358);
        KILL(sail_string)(&zgaz3357);
      }
      concat_str(&zgaz3361, " s:", zgaz3359);
      KILL(sail_string)(&zgaz3359);
    }
    concat_str(&zgaz3362, zgaz3360, zgaz3361);
    KILL(sail_string)(&zgaz3361);
    KILL(sail_string)(&zgaz3360);
  }
  concat_str((*(&zcbz340)), " t:", zgaz3362);
  KILL(sail_string)(&zgaz3362);

  KILL(sail_string)(&zlen_str);

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
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3370;
  {
    uint64_t zgaz3368;
    {
      sail_int zgsz3816;
      CREATE(sail_int)(&zgsz3816);
      CONVERT_OF(sail_int, mach_int)(&zgsz3816, INT64_C(32));
      sail_int zgsz3817;
      CREATE(sail_int)(&zgsz3817);
      CONVERT_OF(sail_int, mach_int)(&zgsz3817, INT64_C(0));
      lbits zgsz3818;
      CREATE(lbits)(&zgsz3818);
      sailgen_to_bits(&zgsz3818, zgsz3816, zgsz3817);
      zgaz3368 = CONVERT_OF(fbits, lbits)(zgsz3818, true);
      KILL(lbits)(&zgsz3818);
      KILL(sail_int)(&zgsz3817);
      KILL(sail_int)(&zgsz3816);
    }
    uint64_t zgaz3369;
    {
      lbits zgsz3813;
      CREATE(lbits)(&zgsz3813);
      CONVERT_OF(lbits, fbits)(&zgsz3813, UINT64_C(0b0), UINT64_C(1) , true);
      lbits zgsz3814;
      CREATE(lbits)(&zgsz3814);
      CONVERT_OF(lbits, fbits)(&zgsz3814, zlen, UINT64_C(32) , true);
      lbits zgsz3815;
      CREATE(lbits)(&zgsz3815);
      append(&zgsz3815, zgsz3813, zgsz3814);
      zgaz3369 = CONVERT_OF(fbits, lbits)(zgsz3815, true);
      KILL(lbits)(&zgsz3815);
      KILL(lbits)(&zgsz3814);
      KILL(lbits)(&zgsz3813);
    }
    zgaz3370 = sailgen_setCapBounds(zdefault_cap, zgaz3368, zgaz3369);
  }
  uint64_t zgsz390;
  {
    bool zexact;
    zexact = zgaz3370.ztup0;
    struct zCapability zc;
    zc = zgaz3370.ztup1;
    int64_t ze;
    {
      int64_t zgaz3376;
      {
        uint64_t zgaz3375;
        zgaz3375 = zc.zE;
        {
          lbits zgsz3819;
          CREATE(lbits)(&zgsz3819);
          CONVERT_OF(lbits, fbits)(&zgsz3819, zgaz3375, UINT64_C(6) , true);
          sail_int zgsz3820;
          CREATE(sail_int)(&zgsz3820);
          sail_unsigned(&zgsz3820, zgsz3819);
          zgaz3376 = CONVERT_OF(mach_int, sail_int)(zgsz3820);
          KILL(sail_int)(&zgsz3820);
          KILL(lbits)(&zgsz3819);
        }
      }
      {
        sail_int zgsz3821;
        CREATE(sail_int)(&zgsz3821);
        CONVERT_OF(sail_int, mach_int)(&zgsz3821, zgaz3376);
        sail_int zgsz3822;
        CREATE(sail_int)(&zgsz3822);
        CONVERT_OF(sail_int, mach_int)(&zgsz3822, zcap_max_E);
        sail_int zgsz3823;
        CREATE(sail_int)(&zgsz3823);
        min_int(&zgsz3823, zgsz3821, zgsz3822);
        ze = CONVERT_OF(mach_int, sail_int)(zgsz3823);
        KILL(sail_int)(&zgsz3823);
        KILL(sail_int)(&zgsz3822);
        KILL(sail_int)(&zgsz3821);
      }
    }
    int64_t zez7;
    {
      bool zgaz3374;
      zgaz3374 = zc.zinternal_E;
      if (zgaz3374) {
        {
          sail_int zgsz3824;
          CREATE(sail_int)(&zgsz3824);
          CONVERT_OF(sail_int, mach_int)(&zgsz3824, ze);
          sail_int zgsz3825;
          CREATE(sail_int)(&zgsz3825);
          CONVERT_OF(sail_int, mach_int)(&zgsz3825, zinternal_E_take_bits);
          sail_int zgsz3826;
          CREATE(sail_int)(&zgsz3826);
          add_int(&zgsz3826, zgsz3824, zgsz3825);
          zez7 = CONVERT_OF(mach_int, sail_int)(zgsz3826);
          KILL(sail_int)(&zgsz3826);
          KILL(sail_int)(&zgsz3825);
          KILL(sail_int)(&zgsz3824);
        }
      } else {  zez7 = INT64_C(0);  }
    }
    sbits zgaz3372;
    {
      int64_t zgaz3371;
      {
        sail_int zgsz3829;
        CREATE(sail_int)(&zgsz3829);
        CONVERT_OF(sail_int, mach_int)(&zgsz3829, INT64_C(32));
        sail_int zgsz3830;
        CREATE(sail_int)(&zgsz3830);
        CONVERT_OF(sail_int, mach_int)(&zgsz3830, zez7);
        sail_int zgsz3831;
        CREATE(sail_int)(&zgsz3831);
        sub_int(&zgsz3831, zgsz3829, zgsz3830);
        zgaz3371 = CONVERT_OF(mach_int, sail_int)(zgsz3831);
        KILL(sail_int)(&zgsz3831);
        KILL(sail_int)(&zgsz3830);
        KILL(sail_int)(&zgsz3829);
      }
      {
        sail_int zgsz3827;
        CREATE(sail_int)(&zgsz3827);
        CONVERT_OF(sail_int, mach_int)(&zgsz3827, zgaz3371);
        lbits zgsz3828;
        CREATE(lbits)(&zgsz3828);
        sailgen_ones(&zgsz3828, zgsz3827);
        zgaz3372 = CONVERT_OF(sbits, lbits)(zgsz3828, true);
        KILL(lbits)(&zgsz3828);
        KILL(sail_int)(&zgsz3827);
      }
    }
    sbits zgaz3373;
    {
      sail_int zgsz3835;
      CREATE(sail_int)(&zgsz3835);
      CONVERT_OF(sail_int, mach_int)(&zgsz3835, zez7);
      lbits zgsz3836;
      CREATE(lbits)(&zgsz3836);
      sailgen_zzeros_implicit(&zgsz3836, zgsz3835);
      zgaz3373 = CONVERT_OF(sbits, lbits)(zgsz3836, true);
      KILL(lbits)(&zgsz3836);
      KILL(sail_int)(&zgsz3835);
    }
    {
      lbits zgsz3832;
      CREATE(lbits)(&zgsz3832);
      CONVERT_OF(lbits, sbits)(&zgsz3832, zgaz3372, true);
      lbits zgsz3833;
      CREATE(lbits)(&zgsz3833);
      CONVERT_OF(lbits, sbits)(&zgsz3833, zgaz3373, true);
      lbits zgsz3834;
      CREATE(lbits)(&zgsz3834);
      append(&zgsz3834, zgsz3832, zgsz3833);
      zgsz390 = CONVERT_OF(fbits, lbits)(zgsz3834, true);
      KILL(lbits)(&zgsz3834);
      KILL(lbits)(&zgsz3833);
      KILL(lbits)(&zgsz3832);
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
  uint64_t zgaz3378;
  {
    uint64_t zgaz3377;
    {
      lbits zgsz3840;
      CREATE(lbits)(&zgsz3840);
      CONVERT_OF(lbits, fbits)(&zgsz3840, zm, UINT64_C(32) , true);
      lbits zgsz3841;
      CREATE(lbits)(&zgsz3841);
      not_bits(&zgsz3841, zgsz3840);
      zgaz3377 = CONVERT_OF(fbits, lbits)(zgsz3841, true);
      KILL(lbits)(&zgsz3841);
      KILL(lbits)(&zgsz3840);
    }
    {
      lbits zgsz3837;
      CREATE(lbits)(&zgsz3837);
      CONVERT_OF(lbits, fbits)(&zgsz3837, zlen, UINT64_C(32) , true);
      lbits zgsz3838;
      CREATE(lbits)(&zgsz3838);
      CONVERT_OF(lbits, fbits)(&zgsz3838, zgaz3377, UINT64_C(32) , true);
      lbits zgsz3839;
      CREATE(lbits)(&zgsz3839);
      add_bits(&zgsz3839, zgsz3837, zgsz3838);
      zgaz3378 = CONVERT_OF(fbits, lbits)(zgsz3839, true);
      KILL(lbits)(&zgsz3839);
      KILL(lbits)(&zgsz3838);
      KILL(lbits)(&zgsz3837);
    }
  }
  {
    lbits zgsz3842;
    CREATE(lbits)(&zgsz3842);
    CONVERT_OF(lbits, fbits)(&zgsz3842, zgaz3378, UINT64_C(32) , true);
    lbits zgsz3843;
    CREATE(lbits)(&zgsz3843);
    CONVERT_OF(lbits, fbits)(&zgsz3843, zm, UINT64_C(32) , true);
    lbits zgsz3844;
    CREATE(lbits)(&zgsz3844);
    and_bits(&zgsz3844, zgsz3842, zgsz3843);
    zcbz342 = CONVERT_OF(fbits, lbits)(zgsz3844, true);
    KILL(lbits)(&zgsz3844);
    KILL(lbits)(&zgsz3843);
    KILL(lbits)(&zgsz3842);
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
static uint64_t zTestLen;

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_doCSetBounds(struct zCapability, uint64_t);

static struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 sailgen_doCSetBounds(struct zCapability zinput, uint64_t zreq_len)
{
  __label__ case_180, finish_match_179, end_function_181, end_block_exception_182;

  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz343;
  uint64_t znewBase;
  znewBase = zinput.zaddress;
  uint64_t znewTop;
  {
    uint64_t zgaz3384;
    {
      int64_t zgaz3383;
      {
        sail_int zgsz3848;
        CREATE(sail_int)(&zgsz3848);
        CONVERT_OF(sail_int, mach_int)(&zgsz3848, INT64_C(32));
        sail_int zgsz3849;
        CREATE(sail_int)(&zgsz3849);
        CONVERT_OF(sail_int, mach_int)(&zgsz3849, INT64_C(1));
        sail_int zgsz3850;
        CREATE(sail_int)(&zgsz3850);
        add_int(&zgsz3850, zgsz3848, zgsz3849);
        zgaz3383 = CONVERT_OF(mach_int, sail_int)(zgsz3850);
        KILL(sail_int)(&zgsz3850);
        KILL(sail_int)(&zgsz3849);
        KILL(sail_int)(&zgsz3848);
      }
      {
        sail_int zgsz3845;
        CREATE(sail_int)(&zgsz3845);
        CONVERT_OF(sail_int, mach_int)(&zgsz3845, zgaz3383);
        lbits zgsz3846;
        CREATE(lbits)(&zgsz3846);
        CONVERT_OF(lbits, fbits)(&zgsz3846, znewBase, UINT64_C(32) , true);
        lbits zgsz3847;
        CREATE(lbits)(&zgsz3847);
        sailgen_EXTZ(&zgsz3847, zgsz3845, zgsz3846);
        zgaz3384 = CONVERT_OF(fbits, lbits)(zgsz3847, true);
        KILL(lbits)(&zgsz3847);
        KILL(lbits)(&zgsz3846);
        KILL(sail_int)(&zgsz3845);
      }
    }
    {
      lbits zgsz3851;
      CREATE(lbits)(&zgsz3851);
      CONVERT_OF(lbits, fbits)(&zgsz3851, zgaz3384, UINT64_C(33) , true);
      lbits zgsz3852;
      CREATE(lbits)(&zgsz3852);
      CONVERT_OF(lbits, fbits)(&zgsz3852, zreq_len, UINT64_C(33) , true);
      lbits zgsz3853;
      CREATE(lbits)(&zgsz3853);
      add_bits(&zgsz3853, zgsz3851, zgsz3852);
      znewTop = CONVERT_OF(fbits, lbits)(zgsz3853, true);
      KILL(lbits)(&zgsz3853);
      KILL(lbits)(&zgsz3852);
      KILL(lbits)(&zgsz3851);
    }
  }
  bool zinBounds;
  {
    int64_t zgaz3382;
    {
      lbits zgsz3854;
      CREATE(lbits)(&zgsz3854);
      CONVERT_OF(lbits, fbits)(&zgsz3854, zreq_len, UINT64_C(33) , true);
      sail_int zgsz3855;
      CREATE(sail_int)(&zgsz3855);
      sail_unsigned(&zgsz3855, zgsz3854);
      zgaz3382 = CONVERT_OF(mach_int, sail_int)(zgsz3855);
      KILL(sail_int)(&zgsz3855);
      KILL(lbits)(&zgsz3854);
    }
    zinBounds = sailgen_inCapBounds(zinput, znewBase, zgaz3382);
  }
  struct zCapability zinCap;
  zinCap = sailgen_clearTagIfSealed(zinput);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3379;
  zgaz3379 = sailgen_setCapBounds(zinCap, znewBase, znewTop);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz392;
  {
    bool zexact;
    zexact = zgaz3379.ztup0;
    struct zCapability znewCap;
    znewCap = zgaz3379.ztup1;
    struct zCapability zgaz3381;
    {
      bool zgaz3380;
      zgaz3380 = not(zinBounds);
      zgaz3381 = sailgen_clearTagIf(znewCap, zgaz3380);
    }
    struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgsz393;
    zgsz393.ztup0 = zexact;
    zgsz393.ztup1 = zgaz3381;
    zgsz392 = zgsz393;
    goto finish_match_179;
  }
case_180: ;
  sail_match_failure("doCSetBounds");
finish_match_179: ;
  zcbz343 = zgsz392;






end_function_181: ;
  return zcbz343;
end_block_exception_182: ;
  struct zCapability zcbz362 = { .zuperms = UINT64_C(0xdeadc0de), .ztag = false, .zreserved = UINT64_C(0xdeadc0de), .zpermit_unseal = false, .zpermit_store_local_cap = false, .zpermit_store_cap = false, .zpermit_store = false, .zpermit_set_CID = false, .zpermit_seal = false, .zpermit_load_cap = false, .zpermit_load = false, .zpermit_execute = false, .zpermit_cinvoke = false, .zotype = UINT64_C(0xdeadc0de), .zinternal_E = false, .zglobal = false, .zflag_cap_mode = false, .zaddress = UINT64_C(0xdeadc0de), .zaccess_system_regs = false, .zT = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zB = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zcbz361 = { .ztup1 = zcbz362, .ztup0 = false };
  return zcbz361;
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
  uint64_t zbits1;
  zbits1 = sailgen_capToBits(zTestCap);
  uint64_t zbits2;
  zbits2 = sailgen_capToMemBits(zTestCap);
  struct zCapability zcap1;
  {
    bool zgaz3391;
    zgaz3391 = zTestCap.ztag;
    zcap1 = sailgen_capBitsToCapability(zgaz3391, zbits1);
  }
  struct zCapability zcap2;
  {
    bool zgaz3390;
    zgaz3390 = zTestCap.ztag;
    zcap2 = sailgen_memBitsToCapability(zgaz3390, zbits2);
  }
  bool zrep;
  zrep = sailgen_fastRepCheck(zTestCap, zTestAddr);
  struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3385;
  zgaz3385 = sailgen_doCSetBounds(zTestCap, zTestLen);
  unit zgsz397;
  {
    __label__ case_187, finish_match_186;

    bool zexact;
    zexact = zgaz3385.ztup0;
    struct zCapability zcap;
    zcap = zgaz3385.ztup1;
    struct ztuple_z8z5boolzCz0z5structz0zzCapabilityz9 zgaz3386;
    zgaz3386 = sailgen_setCapAddr(zTestCap, zTestAddr);
    unit zgsz398;
    {
      __label__ case_189, finish_match_188;

      bool zrepresentable;
      zrepresentable = zgaz3386.ztup0;
      struct zCapability zcapshadowz30;
      zcapshadowz30 = zgaz3386.ztup1;
      struct ztuple_z8z5i64zCz0z5i64z9 zgaz3387;
      zgaz3387 = sailgen_getCapBounds(zTestCap);
      unit zgsz399;
      {
        int64_t zbase;
        zbase = zgaz3387.ztup0;
        int64_t ztop;
        ztop = zgaz3387.ztup1;
        uint64_t zflags;
        zflags = sailgen_getCapFlags(zTestCap);
        struct zCapability znew_cap;
        {
          uint64_t zgaz3389;
          {
            int64_t zgaz3388;
            {
              sail_int zgsz3858;
              CREATE(sail_int)(&zgsz3858);
              CONVERT_OF(sail_int, mach_int)(&zgsz3858, INT64_C(15));
              sail_int zgsz3859;
              CREATE(sail_int)(&zgsz3859);
              CONVERT_OF(sail_int, mach_int)(&zgsz3859, INT64_C(0));
              sail_int zgsz3860;
              CREATE(sail_int)(&zgsz3860);
              add_int(&zgsz3860, zgsz3858, zgsz3859);
              zgaz3388 = CONVERT_OF(mach_int, sail_int)(zgsz3860);
              KILL(sail_int)(&zgsz3860);
              KILL(sail_int)(&zgsz3859);
              KILL(sail_int)(&zgsz3858);
            }
            {
              sail_int zgsz3856;
              CREATE(sail_int)(&zgsz3856);
              CONVERT_OF(sail_int, mach_int)(&zgsz3856, zgaz3388);
              lbits zgsz3857;
              CREATE(lbits)(&zgsz3857);
              sailgen_ones(&zgsz3857, zgsz3856);
              zgaz3389 = CONVERT_OF(fbits, lbits)(zgsz3857, true);
              KILL(lbits)(&zgsz3857);
              KILL(sail_int)(&zgsz3856);
            }
          }
          znew_cap = sailgen_setCapPerms(zTestCap, zgaz3389);
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
        goto finish_match_188;
      }
    case_189: ;
      sail_match_failure("main");
    finish_match_188: ;
      zgsz398 = zgsz399;
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
      sail_int zgsz3861;
      CREATE(sail_int)(&zgsz3861);
      CONVERT_OF(sail_int, mach_int)(&zgsz3861, INT64_C(32));
      lbits zgsz3862;
      CREATE(lbits)(&zgsz3862);
      UNDEFINED(lbits)(&zgsz3862, zgsz3861);
      zTestAddr = CONVERT_OF(fbits, lbits)(zgsz3862, true);
      KILL(lbits)(&zgsz3862);
      KILL(sail_int)(&zgsz3861);
    }
    unit zgsz3104;
    zgsz3104 = UNIT;
  }
  int64_t zgaz3392;
  {
    sail_int zgsz3865;
    CREATE(sail_int)(&zgsz3865);
    CONVERT_OF(sail_int, mach_int)(&zgsz3865, INT64_C(32));
    sail_int zgsz3866;
    CREATE(sail_int)(&zgsz3866);
    CONVERT_OF(sail_int, mach_int)(&zgsz3866, INT64_C(1));
    sail_int zgsz3867;
    CREATE(sail_int)(&zgsz3867);
    add_int(&zgsz3867, zgsz3865, zgsz3866);
    zgaz3392 = CONVERT_OF(mach_int, sail_int)(zgsz3867);
    KILL(sail_int)(&zgsz3867);
    KILL(sail_int)(&zgsz3866);
    KILL(sail_int)(&zgsz3865);
  }
  {
    sail_int zgsz3863;
    CREATE(sail_int)(&zgsz3863);
    CONVERT_OF(sail_int, mach_int)(&zgsz3863, zgaz3392);
    lbits zgsz3864;
    CREATE(lbits)(&zgsz3864);
    UNDEFINED(lbits)(&zgsz3864, zgsz3863);
    zTestLen = CONVERT_OF(fbits, lbits)(zgsz3864, true);
    KILL(lbits)(&zgsz3864);
    KILL(sail_int)(&zgsz3863);
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


