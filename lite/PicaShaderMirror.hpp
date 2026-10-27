// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
// CPU-only model of PICA vertex-shader code and operand descriptor uploads.
// It watches the game's OWN PICA packet stream; no GPU memory is read/written.
// A pass-through plugin cannot restore shader SRAM merely by replaying
// the current transfer registers. This module captures the upload payload.
namespace PicaShaderMirror {
constexpr size_t kCodeCapacity=512;
constexpr size_t kDescriptorCapacity=128;
struct Snapshot {
    uint32_t code[kCodeCapacity]{};
    uint32_t descriptors[kDescriptorCapacity]{};
    uint8_t code_seen[kCodeCapacity]{};
    uint8_t descriptor_seen[kDescriptorCapacity]{};
    uint32_t code_cursor{};
    uint32_t desc_cursor{};
    uint16_t code_highwater{};
    uint16_t desc_highwater{};
    bool upload_started{};
    bool upload_ended{};
    bool descriptors_started{};
    bool code_position_valid{};
    bool desc_position_valid{};
    uint32_t generation{};
};
struct Report {
    bool complete_code_from_zero{};
    bool complete_descriptors_from_zero{};
    bool replay_candidate{};
    uint16_t code_words{};
    uint16_t descriptor_words{};
    uint32_t generation{};
};
// Atomic packet decoding; malformed PICA lists leave snapshot unchanged.
bool Observe(Snapshot& snapshot,const uint32_t* command_list,size_t word_count);
Report Inspect(const Snapshot& snapshot);
} // namespace PicaShaderMirror
