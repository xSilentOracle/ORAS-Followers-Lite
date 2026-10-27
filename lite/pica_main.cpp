// PICA sprite-layer experiment; all 721 species from mmo_pokes_3d.bin.
// No MMO network, no LCD framebuffer blitting by this plugin.
#include "GpuFollower.hpp"
#include "GameHook.hpp"
#include "GameState.hpp"
#include "GameStats.hpp"
#include "Overlay.hpp"
#include "Follower3D.hpp"
#include "Config.hpp"
#include "Log.hpp"
#include <3ds.h>
#include <CTRPluginFramework.hpp>
using namespace CTRPluginFramework;
static volatile bool s_running = true;
void UiSendSimple(u8,u8,u8,const char*) {}
static void StateThread(void*) {
    while(s_running) {
        const u64 now=osGetTime();
        Game::Update(now);
        const Game::LocalState st=Game::Snapshot();
        Stats::Update(now,st.inBattle);
        svcSleepThread(33000000LL);
    }
}
static void AssetsThread(void*) {
    while(s_running) {
        const bool more=Follower3D::Serve();
        svcSleepThread((more?8:40)*1000000LL);
    }
}
static bool SpriteCallback(const Screen& screen) {
    if (!screen.IsTop) return false;
    // Draws to an isolated RAM buffer. Returns false because the OSD never
    // touches the 3DS LCD framebuffer.
    Overlay::OnFollowerOnlyFrame(screen);
    return false;
}
namespace CTRPluginFramework {
void PatchProcess(FwkSettings&) {}
int main(void) {
    Log::Init();
    Cfg::Load();
    Cfg::followers=1;
    Cfg::hudMode=2;
    Cfg::showHud=false;
    Cfg::mapMode=2;
    Cfg::showNames=false;
    Cfg::showMySkin=false;
    Cfg::colorVariant=0;
    Cfg::gameLang=0;
    Cfg::perfMode=2;
    Cfg::hdCanvas=false;
    Cfg::ApplyPerf();
    if(!Overlay::Init()) Log::Write("PICA: overlay sprite init failed");
    Follower3D::Reserve();
    const bool gameOk=GameHook::Install();
    const bool gpuOk=gameOk && GpuFollower::Init() && GpuFollower::Install();
    Log::Write("PICA: gameOK %d gpuOK %d",int(gameOk),int(gpuOk));
    PluginMenu* menu = new PluginMenu("ORAS GPU Pokemon",0,2,0,
        "EXPERIMENTAL: PICA200 sprite compositor, may crash. Back up save.");
    menu->SynchronizeWithFrame(true);
    if(gpuOk) OSD::Run(SpriteCallback);
    Thread state=threadCreate(StateThread,nullptr,0x8000,0x31,-2,false);
    Thread assets=threadCreate(AssetsThread,nullptr,0x10000,0x3F,-2,false);
    menu->Run();
    s_running=false;
    GpuFollower::Fini();
    if(state){threadJoin(state,3000000000ULL);threadFree(state);}
    if(assets){threadJoin(assets,3000000000ULL);threadFree(assets);}
    delete menu;
    return 0;
}
}