// Native ORAS field-rendered Pokemon follower PLAYTEST.
// Uses the exact GameHook that previously rendered a trailing native NPC at
// Tico 3x. Only game-owned field Pokémon models are used in this playtest.
// Unsupported lead species falls back to Zigzagoon as a visual test.
#include "GameHook.hpp"
#include "GameState.hpp"
#include "GameStats.hpp"
#include "Players.hpp"
#include "Protocol.hpp"
#include "NativeFieldPokemon.hpp"
#include "Log.hpp"
#include <3ds.h>
#include <CTRPluginFramework.hpp>
#include <cstring>
using namespace CTRPluginFramework;
static volatile bool s_running=true;
void UiSendSimple(u8,u8,u8,const char*){}
struct History {float x,y,z;u16 zone;bool valid;};
static void FollowerLoop(void*) {
 History hist[24]={};
 int index=0,count=0;
 u16 lastZone=0xFFFF, lastSpecies=0, lastModel=0;
 u64 lastStats=0;
 while(s_running){
   const u64 now=osGetTime();
   Game::Update(now);
   auto me=Game::Snapshot();
   Game::RefreshFlags(me);
   Game::RefreshPos(me);
   const bool ok=me.valid&&!me.inBattle&&
       me.playerState==Game::STATE_FREE&&!me.inApp&&me.ride==0;
   if(lastZone!=me.zone || !ok) {
     std::memset(hist,0,sizeof(hist));
     index=count=0;
     lastZone=me.zone;
   }
   if(now-lastStats>=400){
     Stats::Update(now,me.inBattle);
     lastStats=now;
   }
   const auto mine=Stats::GetMine();
   const u16 species=mine.okParty?mine.leadSpecies:0;
   u16 native=NativeFieldPokemon::ModelForSpecies(species);
   if(!native)native=NativeFieldPokemon::kDefault;
   if(species!=lastSpecies||native!=lastModel){
     Log::Write("Native Pokemon test: party leader #%u model=0x%X%s",
           (unsigned)species,(unsigned)native,
           NativeFieldPokemon::ModelForSpecies(species)?"":" [Zigzagoon test fallback]");
     lastSpecies=species;
     lastModel=native;
     index=count=0; // model switch without stale physical trail
   }
   MMO::RemoteView view={};
   if(ok){
      hist[index]={me.x,me.y,me.z,me.zone,true};
      index=(index+1)%24;
      if(count<24)++count;
      const int lag=count>12?12:count-1;
      const int delayed=(index+24-1-lag)%24;
      const History& h=hist[delayed];
      view.valid=h.valid && h.zone==me.zone;
      view.id=1;
      view.model=native;
      view.x=h.x;view.y=h.y;view.z=h.z;
      view.zone=me.zone;
      view.yaw=me.yaw;
      view.speed=me.speed;
      view.state=me.moveState;
   }
   GameHook::FeedViews(&view,1,now);
   svcSleepThread(33000000LL);
 }
 MMO::RemoteView blank={};
 GameHook::FeedViews(&blank,1,osGetTime());
}
namespace CTRPluginFramework {
void PatchProcess(FwkSettings&) {}
int main(void) {
 Log::Init();
 const bool ok=GameHook::Install();
 Log::Write("Native Pokemon field actor playtest: hook=%d",(int)ok);
 PluginMenu* menu=new PluginMenu("ORAS Native Pokemon Test",0,1,0,
  "Native 3D Pokemon models in Omega Ruby's own GPU renderer.\n"
  "Mapped lead species follows; other species show Zigzagoon as a test.\n"
  "This is not yet the full 721-species release.");
 menu->SynchronizeWithFrame(true);
 Thread th=ok?threadCreate(FollowerLoop,nullptr,0x9000,0x31,-2,false):nullptr;
 menu->Run();
 s_running=false;
 if(th){threadJoin(th,3000000000ULL);threadFree(th);}
 delete menu;
 return 0;
}
}
