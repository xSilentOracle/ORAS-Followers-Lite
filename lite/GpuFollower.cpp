// SPDX-License-Identifier: MIT
// All-species framebuffer-independent sprite staging. NO GPU command submission.
// Replaces the unstable Citro3D-inside-nn::gx-hook prototype; intentionally
// produces no visible sprite until a queue-safe display path exists.
#include "GpuFollower.hpp"
#include "Follower3D.hpp"
#include "PicaFrameCodec.hpp"
#include "Log.hpp"
#include <3ds.h>
#include <cstring>

namespace GpuFollower {
static u16 s_scratch[PicaFrameCodec::ScreenWidth*PicaFrameCodec::ScreenHeight];
static u32 s_tiled[PicaFrameCodec::TexturePixels] __attribute__((aligned(16)));
static PicaFrameCodec::SpriteInfo s_meta{};
static LightLock s_guard;
static bool s_ready=false;
static volatile u32 s_frame_count=0;

bool Init() {
    LightLock_Init(&s_guard);
    s_ready=true;
    Log::Write("PICA v2: safe CPU texture codec ready; GPU output DISABLED");
    return true;
}
bool Install() {
    // Fail closed: no hook on game GPU queue, no calls into C3D/C2D, no VRAM writes.
    // The 3GX can validate sprite extraction, but cannot yet display followers.
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
    if (!s_ready) return;
    LightLock_Lock(&s_guard);
    s_meta.visible=false;
    LightLock_Unlock(&s_guard);
}
void Publish(bool drew) {
    if (!s_ready || !drew) { Clear(); return; }
    int x0,y0,x1,y1;
    Follower3D::LastRect(x0,y0,x1,y1);
    LightLock_Lock(&s_guard);
    s_meta=PicaFrameCodec::Encode(s_scratch,PicaFrameCodec::ScreenWidth,
        {x0,y0,x1,y1},s_tiled,PicaFrameCodec::TexturePixels);
    if(s_meta.visible) ++s_frame_count;
    LightLock_Unlock(&s_guard);
}
void Fini() { s_ready=false; }
} // namespace GpuFollower
