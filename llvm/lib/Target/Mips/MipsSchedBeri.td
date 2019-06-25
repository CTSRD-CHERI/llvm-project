
def BeriModel : SchedMachineModel {
  let MicroOpBufferSize = 0; // Explicitly set to zero since BERI is in-order.
  let IssueWidth = 1;        // 1 micro-op is dispatched per cycle.
  let LoadLatency = 2;       // Optimistic load latency assuming bypass.
                             // This is overriden by OperandCycles if the
                             // Itineraries are queried instead.
  let MispredictPenalty = 8;
  let CompleteModel = 0;

  let PostRAScheduler = 1;
}

