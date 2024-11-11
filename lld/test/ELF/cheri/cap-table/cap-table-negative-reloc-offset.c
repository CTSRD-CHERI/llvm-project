// REQUIRES: mips
// RUN: %cheri128_cc1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck --check-prefix RELOCATIONS %s
// RUN: ld.lld -o %t.exe %t.o -verbose 2>&1 | FileCheck %s -check-prefix WARN
// RUN: llvm-objdump -d -r --cap-relocs -t %t.exe | FileCheck %s

enum roff_tok {
    ROFF_br = 0, ROFF_ce, ROFF_ft, ROFF_ll, ROFF_mc,
    ROFF_sp, ROFF_ta, ROFF_ti, ROFF_MAX, ROFF_ab, ROFF_ad, ROFF_af,
    ROFF_aln, ROFF_als, ROFF_am, ROFF_am1, ROFF_ami,
    ROFF_ami1, ROFF_as, ROFF_as1, ROFF_asciify, ROFF_backtrace,
    ROFF_bd, ROFF_bleedat, ROFF_blm, ROFF_box, ROFF_boxa, ROFF_bp,
    ROFF_BP, ROFF_break, ROFF_breakchar, ROFF_brnl, ROFF_brp,
    ROFF_brpnl, ROFF_c2, ROFF_cc, ROFF_cf, ROFF_cflags, ROFF_ch,
    ROFF_char, ROFF_chop, ROFF_class, ROFF_close, ROFF_CL,
    ROFF_color, ROFF_composite, ROFF_continue, ROFF_cp, ROFF_cropat,
    ROFF_cs, ROFF_cu, ROFF_da, ROFF_dch, ROFF_Dd, ROFF_de, ROFF_de1,
    ROFF_defcolor, ROFF_dei, ROFF_dei1, ROFF_device, ROFF_devicem,
    ROFF_di, ROFF_do, ROFF_ds, ROFF_ds1, ROFF_dwh, ROFF_dt, ROFF_ec,
    ROFF_ecr, ROFF_ecs, ROFF_el, ROFF_em, ROFF_EN, ROFF_eo, ROFF_EP,
    ROFF_EQ, ROFF_errprint, ROFF_ev, ROFF_evc, ROFF_ex,
    ROFF_fallback, ROFF_fam, ROFF_fc, ROFF_fchar, ROFF_fcolor,
    ROFF_fdeferlig, ROFF_feature,  /* MAN_fi; ignored in mdoc(7) */
    ROFF_fkern, ROFF_fl, ROFF_flig, ROFF_fp, ROFF_fps, ROFF_fschar,
    ROFF_fspacewidth, ROFF_fspecial, ROFF_ftr, ROFF_fzoom,
    ROFF_gcolor, ROFF_hc, ROFF_hcode, ROFF_hidechar, ROFF_hla,
    ROFF_hlm, ROFF_hpf, ROFF_hpfa, ROFF_hpfcode, ROFF_hw, ROFF_hy,
    ROFF_hylang, ROFF_hylen, ROFF_hym, ROFF_hypp, ROFF_hys,
    ROFF_ie, ROFF_if, ROFF_ig,  /* MAN_in; ignored in mdoc(7) */   ROFF_index,
    ROFF_it, ROFF_itc, ROFF_IX, ROFF_kern, ROFF_kernafter,
    ROFF_kernbefore, ROFF_kernpair, ROFF_lc, ROFF_lc_ctype,
    ROFF_lds, ROFF_length, ROFF_letadj, ROFF_lf, ROFF_lg, ROFF_lhang,
    ROFF_linetabs, ROFF_lnr, ROFF_lnrf, ROFF_lpfx, ROFF_ls,
    ROFF_lsm, ROFF_lt, ROFF_mediasize, ROFF_minss, ROFF_mk,
    ROFF_mso, ROFF_na, ROFF_ne,  /* MAN_nf; ignored in mdoc(7) */   ROFF_nh,
    ROFF_nhychar, ROFF_nm, ROFF_nn, ROFF_nop, ROFF_nr, ROFF_nrf,
    ROFF_nroff, ROFF_ns, ROFF_nx, ROFF_open, ROFF_opena, ROFF_os,
    ROFF_output, ROFF_padj, ROFF_papersize, ROFF_pc, ROFF_pev,
    ROFF_pi, ROFF_PI, ROFF_pl, ROFF_pm, ROFF_pn, ROFF_pnr, ROFF_po,
    ROFF_ps, ROFF_psbb, ROFF_pshape, ROFF_pso, ROFF_ptr,
    ROFF_pvs, ROFF_rchar, ROFF_rd, ROFF_recursionlimit, ROFF_return,
    ROFF_rfschar, ROFF_rhang, ROFF_rj, ROFF_rm, ROFF_rn, ROFF_rnn,
    ROFF_rr, ROFF_rs, ROFF_rt, ROFF_schar, ROFF_sentchar, ROFF_shc,
    ROFF_shift, ROFF_sizes, ROFF_so, ROFF_spacewidth, ROFF_special,
    ROFF_spreadwarn, ROFF_ss, ROFF_sty, ROFF_substring, ROFF_sv,
    ROFF_sy, ROFF_T_, ROFF_tc, ROFF_TE, ROFF_TH, ROFF_tkf, ROFF_tl,
    ROFF_tm, ROFF_tm1, ROFF_tmc, ROFF_tr, ROFF_track,
    ROFF_transchar, ROFF_trf, ROFF_trimat, ROFF_trin, ROFF_trnt,
    ROFF_troff, ROFF_TS, ROFF_uf, ROFF_ul, ROFF_unformat, ROFF_unwatch,
    ROFF_unwatchn, ROFF_vpt, ROFF_vs, ROFF_warn, ROFF_warnscale,
    ROFF_watch, ROFF_watchlength, ROFF_watchn, ROFF_wh, ROFF_while,
    ROFF_write, ROFF_writec, ROFF_writem, ROFF_xflag, ROFF_cblock,
    ROFF_RENAMED, ROFF_USERDEF, TOKEN_NONE, MDOC_Dd, MDOC_Dt, MDOC_Os,
    MDOC_Sh, MDOC_Ss, MDOC_Pp, MDOC_D1, MDOC_Dl, MDOC_Bd, MDOC_Ed, MDOC_Bl,
    MDOC_El, MDOC_It, MDOC_Ad, MDOC_An, MDOC_Ap, MDOC_Ar, MDOC_Cd, MDOC_Cm,
    MDOC_Dv, MDOC_Er, MDOC_Ev, MDOC_Ex, MDOC_Fa, MDOC_Fd, MDOC_Fl, MDOC_Fn,
    MDOC_Ft, MDOC_Ic, MDOC_In, MDOC_Li, MDOC_Nd, MDOC_Nm, MDOC_Op, MDOC_Ot,
    MDOC_Pa, MDOC_Rv, MDOC_St, MDOC_Va, MDOC_Vt, MDOC_Xr, MDOC__A, MDOC__B,
    MDOC__D, MDOC__I, MDOC__J, MDOC__N, MDOC__O, MDOC__P, MDOC__R, MDOC__T,
    MDOC__V, MDOC_Ac, MDOC_Ao, MDOC_Aq, MDOC_At, MDOC_Bc, MDOC_Bf, MDOC_Bo,
    MDOC_Bq, MDOC_Bsx, MDOC_Bx, MDOC_Db, MDOC_Dc, MDOC_Do, MDOC_Dq,
    MDOC_Ec, MDOC_Ef, MDOC_Em, MDOC_Eo, MDOC_Fx, MDOC_Ms, MDOC_No, MDOC_Ns,
    MDOC_Nx, MDOC_Ox, MDOC_Pc, MDOC_Pf, MDOC_Po, MDOC_Pq, MDOC_Qc, MDOC_Ql,
    MDOC_Qo, MDOC_Qq, MDOC_Re, MDOC_Rs, MDOC_Sc, MDOC_So, MDOC_Sq, MDOC_Sm,
    MDOC_Sx, MDOC_Sy, MDOC_Tn, MDOC_Ux, MDOC_Xc, MDOC_Xo, MDOC_Fo, MDOC_Fc,
    MDOC_Oo, MDOC_Oc, MDOC_Bk, MDOC_Ek, MDOC_Bt, MDOC_Hf, MDOC_Fr, MDOC_Ud,
    MDOC_Lb, MDOC_Lp, MDOC_Lk, MDOC_Mt, MDOC_Brq, MDOC_Bro,
    MDOC_Brc, MDOC__C, MDOC_Es, MDOC_En, MDOC_Dx, MDOC__Q, MDOC__U,
    MDOC_Ta, MDOC_MAX, MAN_TH, MAN_SH, MAN_SS, MAN_TP, MAN_LP,
    MAN_PP, MAN_P, MAN_IP, MAN_HP, MAN_SM, MAN_SB, MAN_BI, MAN_IB,
    MAN_BR, MAN_RB, MAN_R, MAN_B, MAN_I, MAN_IR, MAN_RI, MAN_nf,
    MAN_fi, MAN_RE, MAN_RS, MAN_DT, MAN_UC, MAN_PD, MAN_AT, MAN_in,
    MAN_OP, MAN_EX, MAN_EE, MAN_UR, MAN_UE, MAN_MAX
};



struct roff_man;

#define  MACRO_PROT_ARGS    struct roff_man *man, \
                             enum roff_tok tok, \
                             int line, \
                             int ppos, \
                             int *pos, \
                             char *buf

struct man_macro {
    void (*fp)(MACRO_PROT_ARGS);

    int flags;
#define  MAN_SCOPED        (1 << 0)  /* Optional next-line scope. */
#define  MAN_NSCOPED       (1 << 1)  /* Allowed in next-line element scope. */
#define  MAN_BSCOPE        (1 << 2)  /* Break next-line block scope. */
#define  MAN_JOIN          (1 << 3)  /* Join arguments together. */
};

extern const struct man_macro *const man_macros;

void in_line_eoln(MACRO_PROT_ARGS) {}
void blk_imp(MACRO_PROT_ARGS) {}
void blk_close(MACRO_PROT_ARGS) {}
void blk_exp(MACRO_PROT_ARGS) {}

const struct man_macro __man_macros[MAN_MAX - MAN_TH] = {
  {in_line_eoln, MAN_BSCOPE}, /* TH */
  {blk_imp,      MAN_BSCOPE | MAN_SCOPED}, /* SH */
  {blk_imp,      MAN_BSCOPE | MAN_SCOPED}, /* SS */
  {blk_imp,      MAN_BSCOPE | MAN_SCOPED}, /* TP */
  {blk_imp,      MAN_BSCOPE}, /* LP */
  {blk_imp,      MAN_BSCOPE}, /* PP */
  {blk_imp,      MAN_BSCOPE}, /* P */
  {blk_imp,      MAN_BSCOPE}, /* IP */
  {blk_imp,      MAN_BSCOPE}, /* HP */
  {in_line_eoln, MAN_SCOPED | MAN_JOIN}, /* SM */
  {in_line_eoln, MAN_SCOPED | MAN_JOIN}, /* SB */
  {in_line_eoln, 0}, /* BI */
  {in_line_eoln, 0}, /* IB */
  {in_line_eoln, 0}, /* BR */
  {in_line_eoln, 0}, /* RB */
  {in_line_eoln, MAN_SCOPED | MAN_JOIN}, /* R */
  {in_line_eoln, MAN_SCOPED | MAN_JOIN}, /* B */
  {in_line_eoln, MAN_SCOPED | MAN_JOIN}, /* I */
  {in_line_eoln, 0}, /* IR */
  {in_line_eoln, 0}, /* RI */
  {in_line_eoln, MAN_NSCOPED}, /* nf */
  {in_line_eoln, MAN_NSCOPED}, /* fi */
  {blk_close,    MAN_BSCOPE}, /* RE */
  {blk_exp,      MAN_BSCOPE}, /* RS */
  {in_line_eoln, 0}, /* DT */
  {in_line_eoln, 0}, /* UC */
  {in_line_eoln, MAN_NSCOPED}, /* PD */
  {in_line_eoln, 0}, /* AT */
  {in_line_eoln, 0}, /* in */
  {in_line_eoln, 0}, /* OP */
  {in_line_eoln, MAN_BSCOPE}, /* EX */
  {in_line_eoln, MAN_BSCOPE}, /* EE */
  {blk_exp,      MAN_BSCOPE}, /* UR */
  {blk_close,    MAN_BSCOPE}, /* UE */
};
const struct man_macro *const man_macros = __man_macros - MAN_TH;
// RELOCATIONS: 0x440 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __man_macros 0xFFFFFFFFFFFFD280

int __start(void) {
  return man_macros[MAN_TH].flags;
}

// verify that the __cap_reloc for man_macros has the right length 0x440 even though it is way out of bounds

// WARN: global capability offset -11648 is less than 0:
// WARN-NEXT: >>> Location: object man_macros
// WARN-NEXT: >>> defined in cap-table-negative-reloc-offset.c (
// WARN-NEXT: >>> Target: object __man_macros
// WARN-NEXT: >>> defined in cap-table-negative-reloc-offset.c


// CHECK-LABEL: SYMBOL TABLE:
// CHECK: [[__MAN_MACROS_ADDR:0000000000030820]] g     O .data.rel.ro		 0000000000000440 __man_macros
// CHECK: [[MAN_MACROS_ADDR:0000000000030c60]] g     O .data.rel.ro		 0000000000000010 man_macros

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK: 0x[[MAN_MACROS_ADDR]]	Base: __man_macros (0x[[__MAN_MACROS_ADDR]])	Offset: 0xffffffffffffd280	Length: 0x0000000000000440	Permissions: 0x4000000000000000 (Constant)
