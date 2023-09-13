/* Functions we want to test: */
register TestCap : bits(129)

/* like execute_SCBNDS but without the global writes */
function doCSetBounds(input: bits(129), req_len: bits(65)) -> bits(129) = {
  let result = CapSetBounds(input, req_len, false);
  if CapIsSealed(input) then {
    CapWithTagClear(result);
  } else {
    result
  }
}

function main () : unit -> unit = {
  TestCap = Zeros(129);
  TestCap2 = CMAX;
  /* while (true) do { */
      let bits1 = CapGetValue(TestCap);
      let bits2 = CapGetPermissions(TestCap);
      let bits3 = CapGetObjectType(TestCap);
      let bits4 = CapGetTag(TestCap);
      let bits5 = doCSetBounds(TestCap, Zeros(65));
      let bits6 = CapGetBounds(TestCap);
      let exp = CapGetExponent(TestCap);
      let rep = CapIsRepresentableFast(TestCap, Zeros(64));
  /* } */
}
