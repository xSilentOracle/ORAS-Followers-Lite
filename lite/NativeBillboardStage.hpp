// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
namespace NativeBillboardStage {
constexpr int kScreenWidth=400, kScreenHeight=240, kTextureSize=256;
struct Rect { int x,y,w,h; };
// CPU-only conversion from offscreen RGB565 (key color 0) to logical RGBA8.
// The texture is LINEAR and requires conversion into the game's native GPU
// format before it can be used by an injected billboard actor.
bool ConvertRgb565ToRgba(const uint16_t* source,int source_stride,
 int x0,int y0,int x1,int y1,uint32_t* dest,Rect& rect);
}