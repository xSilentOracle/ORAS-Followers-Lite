// SPDX-License-Identifier: MIT
#include "PicaSpriteFragment.hpp"
namespace PicaSpriteFragment {
bool Append(PicaPackets::Writer& w){
 // The six-stage TEV setup follows C3D_TexEnvInit, then
 // C3D_TexEnvSrc(stage0,C3D_Both,GPU_TEXTURE0,...)
 // and C3D_TexEnvFunc(stage0,C3D_Both,GPU_REPLACE).
 constexpr uint32_t kStage0[5]={0x00030003u,0u,0u,0xFFFFFFFFu,0u};
 constexpr uint32_t kPassthrough[5]={0x000F000Fu,0u,0u,0xFFFFFFFFu,0u};
 constexpr uint16_t kStages[6]={0x00C0,0x00C8,0x00D0,0x00D8,0x00F0,0x00F8};
 for(int i=0;i<6;++i)
    if(!w.WriteMany(kStages[i],i? kPassthrough:kStage0,5,true))return false;
 // Explicit GL fragment blending: srcRGB=SRC_ALPHA(6),
 // dstRGB=ONE_MINUS_SRC_ALPHA(7), srcA=ONE(1), dstA=1-srcA(7).
 // C3D_AlphaBlend and C3D_FragOpMode defaults in citro3d/source/effect.c.
 if(!w.Write(0x006D,0)|| // don't write depth for billboard
    !w.Write(0x0040,0)|| // no face culling
    !w.Write(0x008F,0)||!w.Write(0x01C6,1)|| // disable 3DS lighting
    !w.Write(0x0100,0x00E40100u,0x7)|| // GL mode, alpha blending
    !w.Write(0x0101,0x71760000u)|| // alpha blend factors
    !w.Write(0x0104,0)|| // alpha test disabled
    !w.Write(0x0107,0x0F10u)|| // depth writes disabled, color writes on
    !w.Write(0x0113,0xFu))return false; // color writes enabled
 return true;
}
}
