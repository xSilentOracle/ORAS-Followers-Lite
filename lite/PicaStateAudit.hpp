// SPDX-License-Identifier: MIT
#pragma once
#include "PicaPackets.hpp"
#include <cstddef>
#include <cstdint>

namespace PicaStateAudit {
// A shadow copy of the LAST VALUES WRITTEN to GPU registers.
// This is NOT hardware readback, and not a snapshot of GPU shader memory.
struct Shadow {
    uint32_t values[0x400]{};
    uint32_t known_bits[0x400]{};
};
struct Report {
    bool parsed{};
    bool all_registers_known{};
    bool needs_shader_reupload{};
    bool has_irreversible_effects{};
    bool restorable{}; // fail-closed; true only for side-effect-free scalar writes
    uint16_t first_unknown_register{};
    size_t packet_count{};
    size_t word_count{};
};
// Decode the PICA GPUCMD wire format into shadow registers. Transactional:
// malformed streams never partially update 'shadow'.
bool Observe(Shadow& shadow,const uint32_t* words,size_t word_count);
// Verify that prior register values are known AND that every sprite write
// can be restored. Draw triggers, shader upload & cache flush are not
// reversible by replaying registers and therefore return restorable=false.
Report AnalyzeRestore(const Shadow& before,
                      const uint32_t* sprite_words,size_t sprite_word_count);
} // namespace PicaStateAudit
