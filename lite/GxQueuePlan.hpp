// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
// Pure state machine for analyzing ORAS GX command flow.
// It NEVER executes GPU commands and does not hook the game.
namespace GxQueuePlan {
struct Command {
    uint32_t words[8]{};
};
struct State {
    uint32_t color_base{};
    uint32_t depth_base{};
    uint32_t depth_end{};
    uint32_t draw_lists{};
    uint32_t frame_number{};
    bool color_valid{};
    bool top_seen{};
};
struct Decision {
    bool recognized_top{};
    bool safe_position{};
    uint32_t render_color{};
    uint32_t render_depth{};
    uint32_t last_draw_lists{};
    uint32_t frame_number{};
};
constexpr uint8_t kProcessCommandList = 1;
constexpr uint8_t kMemoryFill = 2;
constexpr uint8_t kDisplayTransfer = 3;
constexpr uint32_t kTopOutputDims = (400u << 16) | 240u;
constexpr uint32_t kVramMin = 0x1F000000u;
constexpr uint32_t kVramMax = 0x1F600000u;
constexpr uint32_t kTopFrameBytes = 256u * 400u * 4u;
// Observe ordered game requests; return candidate insertion info.
// Caller may NOT emit commands based on this alone: it merely rules out
// obvious ordering and address failures.
Decision Observe(State& state, const Command& request);
// Encode a GX type=1 request in a *private buffer*. Never submit it.
// This is used by host tests of FIFO sequencing only.
bool EncodeProcessList(Command& out, uint32_t address, uint32_t bytes,
                       bool flush_cache);
} // namespace GxQueuePlan
