// SPDX-License-Identifier: MIT
// Pure CPU preparation of a PICA200 tiled RGBA8 sprite image.
// No 3GX hook, Citro3D calls or GPU/VRAM access here.
#pragma once
#include <cstddef>
#include <cstdint>

namespace PicaFrameCodec {
constexpr int ScreenWidth = 400;
constexpr int ScreenHeight = 240;
constexpr int TextureWidth = 512;
constexpr int TextureHeight = 256;
constexpr std::size_t TexturePixels = TextureWidth * TextureHeight;
struct Rect { int left, top, right, bottom; };
struct SpriteInfo {
    Rect bounds{};
    float u0{},v0{},u1{},v1{};
    bool visible{};
};
// Input uses RGB565 with 0 as transparent; input rows are top-to-bottom.
// dst has TexturePixels uint32_t words in PICA 8x8 Morton tile ordering.
// Bit layout is R(31:24), G(23:16), B(15:8), A(7:0).
// Bounded to a padded 512x256 texture so giant Pokémon are never cropped.
SpriteInfo Encode(const uint16_t* image, int stride, Rect source,
                  uint32_t* dst, std::size_t dstCount);
// Inverse for native tests and offline debugging; not used in the 3GX.
uint32_t ReadTiled(const uint32_t* dst, int x, int y);
} // namespace PicaFrameCodec
