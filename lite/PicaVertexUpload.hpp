// SPDX-License-Identifier: MIT
#pragma once
#include "PicaPackets.hpp"
#include <cstddef>
#include <cstdint>
// CPU-only serialization of libctru's PICA vertex-shader upload sequence.
// No GSP queue calls, no shared GPU state, and no graphics library ownership.
namespace PicaVertexUpload {
struct Shader {
    const uint32_t* instructions{};
    size_t instruction_words{};
    const uint32_t* descriptors{};
    size_t descriptor_words{};
    uint32_t main_offset{};
    uint32_t outmap_mask{};
    uint32_t outmap[8]{};
    uint32_t outmap_mode{};
    uint32_t outmap_clock{};
};
bool Append(PicaPackets::Writer& out,const Shader& shader);
} // namespace PicaVertexUpload
