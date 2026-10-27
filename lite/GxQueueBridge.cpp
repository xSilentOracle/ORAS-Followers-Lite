// SPDX-License-Identifier: MIT
#include "GxQueueBridge.hpp"
namespace GxQueuePlan {
ReplayResult SimulateGameSubmission(State& st, const Command& incoming,
                                     const CompositorList& sprite,
                                     SubmitFn send, void* user) {
    ReplayResult r{};
    r.decision = Observe(st, incoming);
    if (!send) return r;
    if (r.decision.safe_position && sprite.ready) {
        Command command{};
        if (EncodeProcessList(command, sprite.addr, sprite.bytes, true)) {
            // Never attempt to inject if a callback reports a submission
            // failure: still forward the original game command unchanged.
            if(send(user,command)) r.injected=true;
            else r.injection_failed=true;
        }
    }
    r.original_submitted=send(user,incoming);
    return r;
}
} // namespace GxQueuePlan
