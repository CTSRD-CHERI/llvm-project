/* Functions we want to test: */
/* function capToBits(cap) : Capability -> CapBits */
/* function capToMemBits(cap) : Capability -> CapBits */
/* function capBitsToCapability(t, c) : (bool, CapBits) -> Capability */
/* function memBitsToCapability(tag, b) : (bool, CapBits) -> Capability */
/* function fastRepCheck(c, i) : (Capability, CapAddrBits) -> bool */
/* function setCapBounds(cap, base, top) : (Capability, CapAddrBits, CapLenBits) -> (bool, Capability) */
/* function getCapBounds(cap) : Capability -> (CapAddrInt, CapLen) */
/* val getCapFlags : Capability -> CFlags
   function getCapFlags(cap) = bool_to_bits(cap.flag_cap_mode) */
/* function setCapPerms(cap, perms) : (Capability, bits(31)) -> Capability */
/* function capToString (cap) -> string */
register TestCap : Capability
register TestAddr : CapAddrBits
register TestLen : CapLenBits

/* like execute(CSetBoundsExact(...)) but without the global accesses. */
function doCSetBounds(input: Capability, req_len: CapLenBits) -> (bool, Capability) = {
  let newBase = input.address;
  let newTop : CapLenBits = EXTZ(newBase) + req_len;
  let inBounds = inCapBounds(input, newBase, unsigned(req_len));

  let inCap = clearTagIfSealed(input);
  let (exact, newCap) = setCapBounds(inCap, newBase, newTop);
  (exact, clearTagIf(newCap, not(inBounds)))
}

function main () : unit -> unit = {
  TestCap = null_cap;
  /* while (true) do { */
      bits1 = capToBits(TestCap);
      bits2 = capToMemBits(TestCap);
      cap1 = capBitsToCapability(TestCap.tag, bits1);
      cap2 = memBitsToCapability(TestCap.tag, bits2);
      rep = fastRepCheck(TestCap, TestAddr);
      let (exact, cap) = doCSetBounds(TestCap, TestLen);
      let (representable, cap) = setCapAddr(TestCap, TestAddr);
      let (base, top) = getCapBounds(TestCap);
      flags = getCapFlags(TestCap);
      new_cap = setCapPerms(TestCap, ones()); /* rt_val[30..0] */
      cap_str = capToString(TestCap);
      len = getRepresentableLength(TestAddr);
      mask = getRepresentableAlignmentMask(TestAddr);
  /* } */
}
