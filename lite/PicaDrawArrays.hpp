// SPDX-License-Identifier: MIT
#pragma once
#include "PicaPackets.hpp"
#include <cstdint>
namespace PicaDrawArrays {
// Generates the exact C3D_DrawArrays(GPU_TRIANGLES) register sequence.
// Does NOT configure attributes, shaders, textures or framebuffer.
bool AppendTriangles(PicaPackets::Writer& writer,uint32_t first_vertex,
                     uint32_t vertex_count);
}
