#error "The original in-hook Citro3D renderer caused a console crash. Do not compile this code until GPU queue integration has been replaced."
// Experimental PICA200 textured quad compositing.
//
// Instead of writing the 400x240 LCD framebuffer, the pre-rendered Pokémon
// frame is painted into a private RAM image (RGB565, key 0), swizzled into
// a GPU_RGBA8 texture, and presented as a quad on the game's 3D VRAM color
// target just before nn::gx's top-screen display transfer.
//
// This code depends on the precise US ORAS build verified by GameHook.
// It has NOT been verified in Tico and may interfere with the game's GX queue.
// Keep this branch separate from working native-render proof of concept.
#include "GpuFollower.hpp"
#include "Follower3D.hpp"
#include "GameHook.hpp"
#include "Log.hpp"
#include <3ds.h>
#include <citro2d.h>
#include <CTRPluginFramework.hpp>
#include <cstring>

using namespace CTRPluginFramework;
namespace GpuFollower {
static const int TW=128, TH=128, SW=400, SH=240;
static const u32 GX_FILL=2, GX_TRANSFER=3;
static const u32 TOP_DIMS=(400u<<16)|240u;
static const u32 GX_FUNC=0x001740C4;
static const u32 PROLOG[3]={0xE92D47F0,0xE5903000,0xE3530000};
static u16 s_scratch[SW*SH];
static u32 s_pixels[TW*TH] __attribute__((aligned(16)));
static C3D_Tex s_tex{};
static Tex3DS_SubTexture s_sub{};
static Hook s_hook;
static LightLock s_lock;
static bool s_gpu_ready=false;
static bool s_hooked=false;
static volatile bool s_active=false, s_reentry=false;
static volatile u32 s_color_base=0, s_depth_base=0, s_depth_end=0;
static int s_x=0,s_y=0,s_w=0,s_h=0;
static bool s_valid=false;

static unsigned Morton(unsigned x,unsigned y) {
    return (x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);
}

void ScratchTarget(R3D::Target &t) {
    std::memset(s_scratch,0,sizeof(s_scratch));
    t=R3D::Target{};
    t.base=(u8*)s_scratch;
    t.dx=2;
    t.dy=SW*2;
    t.fmt=R3D::FB_RGB565;
    t.w=SW;
    t.h=SH;
    t.track=false;
}

void Clear() {
    LightLock_Lock(&s_lock);
    s_valid=false;
    LightLock_Unlock(&s_lock);
}

void Publish(bool drew) {
    if (!drew) { Clear(); return; }
    int x0,y0,x1,y1;
    Follower3D::LastRect(x0,y0,x1,y1);
    if (x0<0) x0=0;
    if (y0<0) y0=0;
    if (x1>SW) x1=SW;
    if (y1>SH) y1=SH;
    if (x1<=x0||y1<=y0) { Clear(); return; }
    // Restrict the transient texture. The pack supports all species, including
    // oversized ones; sprites exceeding 128px are centered and cropped here.
    if (x1-x0>TW) {
        int center=(x0+x1)/2;
        x0=center-TW/2;
        x1=x0+TW;
    }
    if (y1-y0>TH) {
        int center=(y0+y1)/2;
        y0=center-TH/2;
        y1=y0+TH;
    }
    if (x0<0||y0<0||x1>SW||y1>SH) { Clear(); return; }
    const int w=x1-x0,h=y1-y0;
    LightLock_Lock(&s_lock);
    std::memset(s_pixels,0,sizeof(s_pixels));
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
        const u16 p=s_scratch[(y0+y)*SW+x0+x];
        if (!p) continue;
        const unsigned r=((p>>11)&31)*255/31;
        const unsigned g=((p>>5)&63)*255/63;
        const unsigned b=(p&31)*255/31;
        // PICA RGBA8 stores the components as 0xRRGGBBAA in a u32 word.
        const u32 rgba=(r<<24)|(g<<16)|(b<<8)|0xFFu;
        const unsigned index=(((y>>3)*(TW>>3)+(x>>3))<<6)+Morton(x&7,y&7);
        s_pixels[index]=rgba;
    }
    s_x=x0;s_y=y0;s_w=w;s_h=h;
    s_sub.width=(u16)w;
    s_sub.height=(u16)h;
    s_sub.left=0.f;
    s_sub.right=(float)w/TW;
    s_sub.top=1.f;
    s_sub.bottom=1.f-(float)h/TH;
    s_valid=true;
    LightLock_Unlock(&s_lock);
}

// Called on the game's GX submission path while it sends its top-screen copy.
static void DrawNativeGpu(u32 color_base) {
    if (!s_gpu_ready||!s_valid||s_reentry||!color_base) return;
    s_reentry=true;
    LightLock_Lock(&s_lock);
    if (s_valid && C3D_FrameBegin(C3D_FRAME_NONBLOCK)) {
        // Borrow the existing native game color render target. Never clear it.
        // This is the 256x512 PICA tiled VRAM surface, NOT the LCD framebuffer.
        C3D_RenderTarget target{};
        C3D_FrameBufAttrib(&target.frameBuf,256,512,false);
        C3D_FrameBufColor(&target.frameBuf,(void*)color_base,GPU_RB_RGBA8);
        C3D_FrameBufDepth(&target.frameBuf,nullptr,GPU_RB_DEPTH24);
        C3D_TexLoadImage(&s_tex,s_pixels,GPU_TEXFACE_2D,0);
        C2D_SceneBegin(&target);
        C2D_Image image={&s_tex,&s_sub};
        // ORAS displays the 400x240 top scene rotated in the 256x512 target:
        // screen (sx,sy) -> target (239-sy,sx).
        C2D_DrawParams params={};
        params.pos.x=(float)(239-s_y);
        params.pos.y=(float)s_x;
        params.pos.w=(float)s_w;
        params.pos.h=(float)s_h;
        params.center.x=0.f;
        params.center.y=0.f;
        params.angle=1.570796327f;
        params.depth=0.f;
        C2D_DrawImage(image,&params,nullptr);
        C2D_Flush();
        C3D_FrameEnd(GX_CMDLIST_FLUSH);
    }
    LightLock_Unlock(&s_lock);
    s_reentry=false;
}
static void InspectGx(u32 *cmd) {
    const u32 id=cmd[0]&0xFF;
    if (id==GX_FILL) {
        if (cmd[4]>=0x1F000000&&cmd[4]<0x1F600000
            &&cmd[6]>cmd[4]&&cmd[5]==0xFFFFFFFFu) {
            s_color_base=cmd[1];
            s_depth_base=cmd[4];
            s_depth_end=cmd[6];
        }
        return;
    }
    if (id!=GX_TRANSFER||cmd[4]!=TOP_DIMS) return;
    const u32 color=s_color_base, depth=s_depth_base;
    if (!color||!depth||cmd[1]<color) return;
    const u32 off=cmd[1]-color;
    if (depth+off+256*400*4>s_depth_end) return;
    DrawNativeGpu(color);
}
static u32 GxHook(u32 holder,u32 *cmd) {
    if (s_active && cmd && !s_reentry) InspectGx(cmd);
    return HookContext::GetCurrent().OriginalFunction<u32>(holder,cmd);
}
bool Init() {
    LightLock_Init(&s_lock);
    if (!C3D_Init(0x10000)) return false;
    if (!C2D_Init(16)) { C3D_Fini(); return false; }
    if (!C3D_TexInit(&s_tex,TW,TH,GPU_RGBA8)) {
        C2D_Fini(); C3D_Fini(); return false;
    }
    C3D_TexSetFilter(&s_tex,GPU_LINEAR,GPU_LINEAR);
    C2D_Prepare();
    s_gpu_ready=true;
    return true;
}
bool Install() {
    if (!GameHook::Active()||!s_gpu_ready) return false;
    for (u32 i=0;i<3;i++)
        if (*(volatile u32*)(GX_FUNC+4*i)!=PROLOG[i]) return false;
    s_hook.InitializeForMitm(GX_FUNC,(u32)GxHook);
    if (s_hook.Enable()!=HookResult::Success) return false;
    s_hooked=true;
    s_active=true;
    return true;
}
void Fini() {
    s_active=false;
    // Existing GX hook cannot safely be uninstalled while the game is running.
    // Keep the backing texture resident until process exit.
}
} // namespace GpuFollower
