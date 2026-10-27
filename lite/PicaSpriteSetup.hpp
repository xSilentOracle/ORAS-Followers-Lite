// SPDX-License-Identifier: MIT
#pragma once
#include "PicaPackets.hpp"
#include <cstdint>
// Serializes a 512x256 RGBA8 texture and six vertices for a PICA200 quad.
// Inputs MUST be real GPU-readable PHYSICAL addresses with flushed caches.
// This code never touches hardware, and does not save/restore ORAS GPU state.
namespace PicaSpriteSetup {
struct Resources {
 uint32_t texture_physical{};
 uint32_t vertices_physical{};
};
bool Append(PicaPackets::Writer& w,const Resources& resources);
}
