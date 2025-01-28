
#ifndef _TEST_CAP_M_AP_H
#define _TEST_CAP_M_AP_H

/*
 * At first, it looked like we could generate both test cases (M, AP ->
 * cr_m, cr_arch_perm and cr_m, cr_arch_perm -> M, AP) from one macro.
 * This didn't work: for error cases, the result depends on the direction.
 */

#define LVB_0 0
#define LVB_1 1

#define TEST_CASE_M_AP_COMP(_cr_lvbits, _cr_m, _cr_arch_perm, _m_ap) \
   TEST_CASE(#_cr_lvbits "-" #_cr_arch_perm "-" #_cr_m " compress", "[compress]") { \
      _cc_cap_t cap; \
      memset(&cap, 0x00, sizeof(cap)); \
      cap.cr_lvbits = (_cr_lvbits); \
\
      cap.cr_arch_perm |= (_cr_arch_perm); \
      cap.cr_m |= (_cr_m); \
      _cc_N(m_ap_compress)(&cap); \
\
      CHECK((_cc_N(get_m)(&cap) << _CC_N(FIELD_AP_SIZE) | \
                  _cc_N(get_ap)(&cap))  == (_m_ap)); \
}


#define TEST_CASE_M_AP_DECOMP(_cr_lvbits, _m_ap, _cr_m, _cr_arch_perm) \
   TEST_CASE(#_cr_lvbits "-" #_m_ap " decompress", "[decompress]") { \
      _cc_cap_t cap; \
      memset(&cap, 0x00, sizeof(cap)); \
      cap.cr_lvbits = (_cr_lvbits); \
\
      _cc_N(update_m)(&cap, (_m_ap) >> _CC_N(FIELD_AP_SIZE)); \
      _cc_N(update_ap)(&cap, (_m_ap) & _CC_N(FIELD_AP_MASK_NOT_SHIFTED)); \
      _cc_N(m_ap_decompress)(&cap); \
\
      CHECK(cap.cr_arch_perm == (_cr_arch_perm)); \
      CHECK(cap.cr_m == (_cr_m)); \
}

#endif
