// SPDX-License-Identifier: MIT
// Quad vertices for PICA200 top-screen render target (rotated 240x400).
#pragma once
#include <array>
#include "PicaFrameCodec.hpp"
namespace SpriteQuad {
struct Vertex {float x,y,z,w,u,v;};
using Triangles=std::array<Vertex,6>;
bool Build(const PicaFrameCodec::SpriteInfo& sprite,Triangles& output);
} // namespace SpriteQuad
