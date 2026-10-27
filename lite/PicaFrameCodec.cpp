// SPDX-License-Identifier: MIT
#include "PicaFrameCodec.hpp"
#include <algorithm>
#include <cstring>

namespace PicaFrameCodec {
namespace {
constexpr unsigned Morton(unsigned x, unsigned y) {
    return (x&1u) | ((y&1u)<<1) | ((x&2u)<<1) | ((y&2u)<<2) |
           ((x&4u)<<2) | ((y&4u)<<3);
}
constexpr std::size_t Index(int x, int y) {
    return static_cast<std::size_t>(((y/8)*(TextureWidth/8)+(x/8))*64) +
           Morton(static_cast<unsigned>(x&7), static_cast<unsigned>(y&7));
}
}
uint32_t ReadTiled(const uint32_t* p, int x, int y) {
    if (!p || x<0 || y<0 || x>=TextureWidth || y>=TextureHeight) return 0;
    return p[Index(x,y)];
}
SpriteInfo Encode(const uint16_t* src, int stride, Rect source,
                  uint32_t* dst, std::size_t count) {
    SpriteInfo out{};
    if (!src || !dst || stride<ScreenWidth || count<TexturePixels) return out;
    std::memset(dst,0,TexturePixels*sizeof(uint32_t));
    source.left=std::clamp(source.left,0,ScreenWidth);
    source.right=std::clamp(source.right,0,ScreenWidth);
    source.top=std::clamp(source.top,0,ScreenHeight);
    source.bottom=std::clamp(source.bottom,0,ScreenHeight);
    if (source.right<=source.left || source.bottom<=source.top) return out;
    out.bounds=source;
    out.visible=true;
    // The sprite lives at its screen position inside a native-size texture.
    // Texture coordinates cover that exact region and no large Pokémon is cropped.
    out.u0=float(source.left)/TextureWidth;
    out.u1=float(source.right)/TextureWidth;
    out.v0=float(source.top)/TextureHeight;
    out.v1=float(source.bottom)/TextureHeight;
    for(int y=source.top;y<source.bottom;++y){
      for(int x=source.left;x<source.right;++x){
        const uint16_t c=src[y*stride+x];
        if (!c) continue;
        const uint32_t r=((c>>11)&31u)*255u/31u;
        const uint32_t g=((c>>5)&63u)*255u/63u;
        const uint32_t b=(c&31u)*255u/31u;
        dst[Index(x,y)]=(r<<24)|(g<<16)|(b<<8)|0xffu;
      }
    }
    return out;
}
} // namespace PicaFrameCodec
