// SPDX-License-Identifier: MIT
#include "SpriteQuad.hpp"
#include <cmath>
namespace SpriteQuad {
static Vertex At(float sx,float sy,float u,float v){
    // ORAS top screen exists in the 256x512 rotated PICA color target:
    // (sx, sy) -> (239 - sy, sx), per original DepthProbe::TexelAddr.
    const float rx=239.f-sy;
    const float ry=sx;
    Vertex o{};
    // Clip-space position for the entire 256x512 render target.
    o.x=2.f*((rx+0.5f)/256.f)-1.f;
    o.y=1.f-2.f*((ry+0.5f)/512.f);
    o.z=0.f;o.w=1.f;
    o.u=u;
    // PICA texture UV coordinates run bottom-up in Citro3D.
    o.v=1.f-v;
    return o;
}
bool Build(const PicaFrameCodec::SpriteInfo& sprite,Triangles& out){
    for(auto& v:out)v={};
    if(!sprite.visible)return false;
    const auto& b=sprite.bounds;
    if(b.left<0 || b.top<0 || b.right>PicaFrameCodec::ScreenWidth ||
       b.bottom>PicaFrameCodec::ScreenHeight ||
       b.right<=b.left || b.bottom<=b.top)return false;
    const Vertex tl=At(float(b.left),float(b.top),sprite.u0,sprite.v0);
    const Vertex tr=At(float(b.right),float(b.top),sprite.u1,sprite.v0);
    const Vertex bl=At(float(b.left),float(b.bottom),sprite.u0,sprite.v1);
    const Vertex br=At(float(b.right),float(b.bottom),sprite.u1,sprite.v1);
    // Two triangles, matching a 2D source rectangle's four oriented corners.
    out={tl,tr,bl,tr,br,bl};
    for(const auto& v:out)if(!std::isfinite(v.x)||!std::isfinite(v.y)||
        !std::isfinite(v.u)||!std::isfinite(v.v))return false;
    return true;
}
} // namespace SpriteQuad
