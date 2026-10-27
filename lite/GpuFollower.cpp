// SPDX-License-Identifier: MIT
// 721-species follower sprite preparation using the real PICA200 shader.
// Draws the Pokémon into a private CPU scratch surface, then prepares a
// GPU-readable textured quad and command list. GPU SUBMISSION IS DISABLED:
// this is an end-to-end staging build, NOT a visible follower mod.
#include "GpuFollower.hpp"
#include "Follower3D.hpp"
#include "PicaFrameCodec.hpp"
#include "PicaSpriteList.hpp"
#include "SpriteQuad.hpp"
#include "SpriteShaderBinary.hpp" // generated at build from pinned Picasso
#include "Log.hpp"
#include <3ds.h>
#include <cstring>

namespace GpuFollower {
namespace {
constexpr size_t kTextureBytes=PicaFrameCodec::TexturePixels*sizeof(u32);
constexpr size_t kCmdWords=4096;
static u16 s_scratch[PicaFrameCodec::ScreenWidth*PicaFrameCodec::ScreenHeight];
static u32* s_texture=nullptr;
static SpriteQuad::Vertex* s_vertices=nullptr;
static u32* s_commands=nullptr;
static PicaFrameCodec::SpriteInfo s_meta{};
static DVLB_s* s_dvlb=nullptr;
static PicaVertexUpload::Shader s_shader{};
static LightLock s_guard;
static bool s_ready=false;
static bool s_prepared=false;
static volatile u32 s_frame_count=0;
static volatile u32 s_command_failures=0;
static size_t s_list_words=0;
}

bool Init() {
    LightLock_Init(&s_guard);
    s_texture=(u32*)linearAlloc(kTextureBytes);
    s_vertices=(SpriteQuad::Vertex*)linearAlloc(sizeof(SpriteQuad::Triangles));
    s_commands=(u32*)linearAlloc(kCmdWords*sizeof(u32));
    if(!s_texture||!s_vertices||!s_commands) {
        Log::Write("PICA v2: insufficient linear memory for staging");
        Fini();
        return false;
    }
    // A PICA shader binary is linked into THIS plugin, not uploaded into the
    // game's graphics pipeline until a fully-restorable queue path exists.
    s_dvlb=DVLB_ParseFile(SpriteShaderBinary::kWords,SpriteShaderBinary::kByteSize);
    if(!s_dvlb||s_dvlb->numDVLE==0||
       s_dvlb->DVLE[0].type!=VERTEX_SHDR||
       !s_dvlb->DVLE[0].dvlp){
        Log::Write("PICA v2: invalid generated shader binary");
        Fini();
        return false;
    }
    const auto& dvle=s_dvlb->DVLE[0];
    s_shader.instructions=dvle.dvlp->codeData;
    s_shader.instruction_words=dvle.dvlp->codeSize;
    s_shader.descriptors=dvle.dvlp->opcdescData;
    s_shader.descriptor_words=dvle.dvlp->opdescSize;
    s_shader.main_offset=dvle.mainOffset;
    s_shader.outmap_mask=dvle.outmapMask;
    std::memcpy(s_shader.outmap,dvle.outmapData,sizeof(s_shader.outmap));
    s_shader.outmap_mode=dvle.outmapMode;
    s_shader.outmap_clock=dvle.outmapClock;
    s_ready=true;
    Log::Write("PICA v2: sprite+shader+draw list staging prepared; GPU output DISABLED");
    return true;
}
bool Install() {
    // Absolutely no GPU queue hook. Compiled command lists are NEVER submitted.
    return s_ready;
}
void ScratchTarget(R3D::Target& t) {
    t=R3D::Target{};
    std::memset(s_scratch,0,sizeof(s_scratch));
    t.base=(u8*)s_scratch;
    t.dx=sizeof(u16);
    t.dy=PicaFrameCodec::ScreenWidth*sizeof(u16);
    t.fmt=R3D::FB_RGB565;
    t.w=PicaFrameCodec::ScreenWidth;
    t.h=PicaFrameCodec::ScreenHeight;
    t.track=false;
}
void Clear() {
    if(!s_ready)return;
    LightLock_Lock(&s_guard);
    s_meta.visible=false;
    s_prepared=false;
    LightLock_Unlock(&s_guard);
}
void Publish(bool drew) {
    if(!s_ready||!drew){Clear();return;}
    int x0,y0,x1,y1;
    Follower3D::LastRect(x0,y0,x1,y1);
    LightLock_Lock(&s_guard);
    s_prepared=false;
    s_meta=PicaFrameCodec::Encode(s_scratch,PicaFrameCodec::ScreenWidth,
       {x0,y0,x1,y1},s_texture,PicaFrameCodec::TexturePixels);
    SpriteQuad::Triangles quad{};
    if(s_meta.visible&&SpriteQuad::Build(s_meta,quad)){
        std::memcpy(s_vertices,quad.data(),sizeof(quad));
        // The generated texture / triangles are accessible to the GPU when
        // the game's queue is eventually validated. This does not submit work.
        GSPGPU_FlushDataCache(s_texture,kTextureBytes);
        GSPGPU_FlushDataCache(s_vertices,sizeof(quad));
        const PicaSpriteSetup::Resources resources={
             osConvertVirtToPhys(s_texture),
             osConvertVirtToPhys(s_vertices)
        };
        const auto packet=PicaSpriteList::Build(s_commands,kCmdWords,
                                               s_shader,resources);
        s_prepared=packet.built && !packet.safe_to_submit;
        s_list_words=packet.words;
        if(s_prepared)++s_frame_count;
        else ++s_command_failures;
    }
    LightLock_Unlock(&s_guard);
}
void Fini(){
    s_ready=false;
    if(s_dvlb){DVLB_Free(s_dvlb);s_dvlb=nullptr;}
    if(s_commands){linearFree(s_commands);s_commands=nullptr;}
    if(s_vertices){linearFree(s_vertices);s_vertices=nullptr;}
    if(s_texture){linearFree(s_texture);s_texture=nullptr;}
    s_prepared=false;
}
} // namespace GpuFollower
