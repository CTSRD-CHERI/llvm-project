
def BeriModel : SchedMachineModel {
  let MicroOpBufferSize = 0; // Explicitly set to zero since BERI is in-order.
  let IssueWidth = 1;        // 1 micro-op is dispatched per cycle.
  // For BERI loads are ideally followed by two instructions to avoid bubbles in the pipeline
  // LoadLatency seems to be include the one cycle for the instruction -> set to 3
  let LoadLatency = 3;
  let MispredictPenalty = 8;
  let CompleteModel = 0;

  let PostRAScheduler = 1;
}

