// SPDX-License-Identifier: MIT
#include "PicaSpriteSetup.hpp"
namespace PicaSpriteSetup {
namespace {
constexpr uint32_t kBase=0x18000000u;
constexpr uint32_t kEnd=0x30000000u;
bool Valid(uint32_t address,uint32_t bytes) {
 return address>=kBase && address<=kEnd-bytes && (address&7u)==0;
}
}
bool Append(PicaPackets::Writer& w,const Resources& r){
 constexpr uint32_t texBytes=512u*256u*4u,vertexBytes=6u*6u*4u;
 if(!Valid(r.texture_physical,texBytes) ||
    !Valid(r.vertices_physical,vertexBytes))return false;
 // devkitPro/citro3d/source/texture.c, C3Di_SetTex(0):
 // GPU_RGBA8=0; GPU_TEX_2D=0; mag/min linear use bits1,2.
 const uint32_t textureRegs[5]={0u,(512u<<16)|256u,0x6u,0u,
                                 r.texture_physical>>3};
 if(!w.Write(0x0080,0x1u,0xBu) ||
    !w.WriteMany(0x0081,textureRegs,5,true)||
    !w.Write(0x008E,0u)||
    !w.Write(0x0080,0x10000u,0x4u))return false;
 // devkitPro/citro3d/source/attribs.c:
 // v0 = 4 x GPU_FLOAT; v1 = 2 x GPU_FLOAT.
 // Attributes use 24-byte stride, two vertex input loaders.
 const uint32_t attribFlags[2]={0x7Fu,0x1FFC0000u};
 if(!w.WriteMany(0x0201,attribFlags,2,true) ||
    !w.Write(0x02B9,0xA0000001u,0xBu)||
    !w.Write(0x0242,1)||
    !w.Write(0x02BB,0x10u)||!w.Write(0x02BC,0u))return false;
 // devkitPro/citro3d/source/buffers.c:
 // GPU attribute memory address derives from 0x18000000 physical base.
 if(!w.Write(0x0200,kBase>>3))return false;
 uint32_t buf[36]{};
 buf[0]=r.vertices_physical-kBase;
 buf[1]=0x10u;               // attr0 -> v0, attr1 -> v1
 buf[2]=0x20180000u;        // 2 attributes, 24-byte stride
 return w.WriteMany(0x0203,buf,36,true);
}
}
