// SPDX-License-Identifier: MIT
#pragma once
#include "PicaShaderMirror.hpp"
#include "PicaPackets.hpp"
#include <cstddef>
// Serialize a captured game's PICA vertex shader memory into a PRIVATE
// GPU command-list. NEVER submits a GX command, never hooks game code.
namespace PicaShaderReplay {
struct Result {
    bool ready{};
    size_t words{};
    uint16_t code_words{};
    uint16_t descriptor_words{};
};
Result Build(const PicaShaderMirror::Snapshot& source,
             uint32_t* output,size_t capacity_words);
} // namespace PicaShaderReplay
