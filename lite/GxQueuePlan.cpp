// SPDX-License-Identifier: MIT
#include "GxQueuePlan.hpp"
#include <limits>
namespace GxQueuePlan {
static bool Within(uint32_t base, uint32_t end, uint32_t span) {
    return base >= kVramMin && base < kVramMax && end > base &&
           end <= kVramMax && span <= end - base;
}
Decision Observe(State& st, const Command& req) {
    Decision d{};
    const uint32_t id = req.words[0] & 0xFFu;
    if(id == kMemoryFill) {
        // ORAS fills the color/depth pair before rendering the field. This
        // signaturing is taken from the existing DepthProbe.cpp hook.
        if (req.words[5] != 0xFFFFFFFFu ||
            !Within(req.words[4],req.words[6],kTopFrameBytes) ||
            req.words[1] < kVramMin || req.words[1] >= kVramMax ||
            (req.words[1] & 7u) || (req.words[4] & 7u)) return d;
        st.color_base = req.words[1];
        st.depth_base = req.words[4];
        st.depth_end = req.words[6];
        st.draw_lists = 0;
        st.top_seen = false;
        st.color_valid = true;
        ++st.frame_number;
        return d;
    }
    if(id == kProcessCommandList) {
        // Ignore malformed command lists rather than counting them as a
        // completed rendering step.
        const uint32_t addr=req.words[1],bytes=req.words[2];
        if (bytes != 0 && bytes <= (4u << 20) &&
            (addr & 7u) == 0 && (bytes & 7u) == 0) {
            ++st.draw_lists;
        }
        return d;
    }
    if(id != kDisplayTransfer || req.words[4] != kTopOutputDims) return d;
    d.recognized_top = true;
    if(!st.color_valid || st.top_seen) return d;
    st.top_seen = true;
    const uint32_t src=req.words[1];
    if(src < st.color_base || src >= kVramMax || (src & 7u)) return d;
    const uint32_t offset=src-st.color_base;
    // The ORAS depth target must have the same layout as its color buffer.
    if (offset > st.depth_end-st.depth_base ||
        !Within(st.depth_base+offset,st.depth_end,kTopFrameBytes))
        return d;
    if(!st.draw_lists) return d;
    d.safe_position=true;
    d.render_color=src;
    d.render_depth=st.depth_base+offset;
    d.last_draw_lists=st.draw_lists;
    d.frame_number=st.frame_number;
    return d;
}
bool EncodeProcessList(Command& out, uint32_t address, uint32_t bytes,
                       bool flush_cache) {
    out=Command{};
    // GX type1 expects a linear/VRAM physical buffer, 8-byte-aligned.
    if(!address || !bytes || (address & 7u) || (bytes & 7u) ||
       bytes > (4u << 20)) return false;
    // This just *describes* a GX packet for testing.
    // Word7 is a boolean flush flag according to the shared memory spec.
    out.words[0]=kProcessCommandList;
    out.words[1]=address;
    out.words[2]=bytes;
    out.words[3]=0;
    out.words[7]=flush_cache ? 1u : 0u;
    return true;
}
} // namespace GxQueuePlan
