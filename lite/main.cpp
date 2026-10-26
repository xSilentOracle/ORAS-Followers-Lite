#include "GameState.hpp"
#include "GameStats.hpp"
#include "Overlay.hpp"
#include "Follower3D.hpp"
#include "Config.hpp"
#include "Perf.hpp"
#include "Log.hpp"
#include <3ds.h>
#include <CTRPluginFramework.hpp>
using namespace CTRPluginFramework;
static volatile bool s_running=true;
static void OfflineStateThread(void *) {
 while(s_running) {
  const u64 now=osGetTime();
  Game::Update(now);
  Game::LocalState st=Game::Snapshot();
  Stats::Update(now,st.inBattle);
  svcSleepThread(33*1000000LL);
 }
}
static void FollowerAssetThread(void *) {
 while(s_running) {
  bool more=Follower3D::Serve();
  svcSleepThread((more?8:40)*1000000LL);
 }
}
static bool FollowerOnlyFrame(const Screen &s) {
 return s.IsTop && Overlay::OnFollowerOnlyFrame(s);
}
namespace CTRPluginFramework {
void PatchProcess(FwkSettings &) {}
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
 Cfg::hdCanvas=true;
 Cfg::ApplyPerf();
 if(!Overlay::Init()) Log::Write("Lite: renderer init failed");
 Follower3D::Reserve();
 PluginMenu* menu=new PluginMenu("ORAS Followers Lite",1,0,0,"Offline follower (experimental)");
 menu->SynchronizeWithFrame(true);
 OSD::Run(FollowerOnlyFrame);
 Thread a=threadCreate(OfflineStateThread,nullptr,0x8000,0x31,-2,false);
 Thread b=threadCreate(FollowerAssetThread,nullptr,0x10000,0x3F,-2,false);
 menu->Run();
 s_running=false;
 if(a){threadJoin(a,3000000000ULL);threadFree(a);}
 if(b){threadJoin(b,3000000000ULL);threadFree(b);}
 delete menu;
 return 0;
}
}