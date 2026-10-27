// SPDX-License-Identifier: MIT
#pragma once
#include "GxQueuePlan.hpp"

namespace GxQueuePlan {
// Offline-only demonstration of inserting a type-1 command before the
// original top-screen transfer. This must NOT be attached to an actual hook.
// A real game hook also needs queue-capacity proofs, GPU-state restoration,
// and validated command buffers -- none exist yet.
using SubmitFn = bool (*)(void* user, const Command&);
struct ReplayResult {
    Decision decision{};
    bool injected{};
    bool original_submitted{};
    bool injection_failed{};
};
struct CompositorList {
    uint32_t addr{};
    uint32_t bytes{};
    bool ready{};
};
ReplayResult SimulateGameSubmission(State& state, const Command& incoming,
                                     const CompositorList& sprite_list,
                                     SubmitFn submit, void* user);
} // namespace GxQueuePlan
