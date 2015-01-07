
def BeriModel : SchedMachineModel {
  let MicroOpBufferSize = 0; // Explicitly set to zero since BERI is in-order.
  let IssueWidth = 1;        // 1 micro-op is dispatched per cycle.
  let MinLatency = 1 ;       // OperandCycles are interpreted as MinLatency.
  let LoadLatency = 4;       // Optimistic load latency assuming bypass.
                             // This is overriden by OperandCycles if the
                             // Itineraries are queried instead.
  let MispredictPenalty = 8;
}

