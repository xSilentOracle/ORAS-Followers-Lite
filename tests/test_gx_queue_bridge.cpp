// SPDX-License-Identifier: MIT
#include "../lite/GxQueueBridge.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace GxQueuePlan;
struct Recorder {
 std::array<Command,16> recorded{};
 int count=0;
 bool fail_injection=false;
};
bool Push(void* user,const Command& c) {
 auto& recorder=*static_cast<Recorder*>(user);
 if ((c.words[0]&0xFF)==kProcessCommandList && recorder.fail_injection &&
     c.words[1] == 0x14002000u)
     return false;
 assert(recorder.count < (int)recorder.recorded.size());
 recorder.recorded[recorder.count++]=c;
 return true;
}
static Command MemoryFill() {
 Command c{};
 c.words[0]=kMemoryFill;
 c.words[1]=0x1F000000;
 c.words[4]=0x1F200000;
 c.words[5]=0xFFFFFFFF;
 c.words[6]=0x1F300000;
 return c;
}
static Command GameDraw() {
 Command c{};
 c.words[0]=kProcessCommandList;
 c.words[1]=0x14000000;
 c.words[2]=0x8000;
 return c;
}
static Command TopTransfer() {
 Command c{};
 c.words[0]=kDisplayTransfer;
 c.words[1]=0x1F000000;
 c.words[4]=kTopOutputDims;
 return c;
}
int main() {
 State state{};
 Recorder recorder{};
 CompositorList image_list{0x14002000,0xA000,true};
 auto feed=[&](Command cmd) {
  return SimulateGameSubmission(state,cmd,image_list,Push,&recorder);
 };
 assert(feed(MemoryFill()).original_submitted);
 assert(feed(GameDraw()).original_submitted);
 auto result=feed(TopTransfer());
 assert(result.decision.safe_position&&result.injected&&result.original_submitted);
 assert(recorder.count==4);
 assert(recorder.recorded[0].words[0]==kMemoryFill);
 assert(recorder.recorded[1].words[0]==kProcessCommandList);
 assert(recorder.recorded[2].words[0]==kProcessCommandList);
 assert(recorder.recorded[2].words[1]==image_list.addr);
 assert(recorder.recorded[3].words[0]==kDisplayTransfer);
 // Original screen transfer must pass unchanged even on duplicate.
 assert(!feed(TopTransfer()).injected);
 assert(recorder.count==5);
 // No sprite texture: no insertion, original game transfer still forwarded.
 feed(MemoryFill());feed(GameDraw());
 image_list.ready=false;
 result=feed(TopTransfer());
 assert(!result.injected&&result.original_submitted);
 // If a synthetic command cannot be submitted, do not block the game.
 Recorder failing{};
 failing.fail_injection=true;
 state=State{};
 image_list.ready=true;
 SimulateGameSubmission(state,MemoryFill(),image_list,Push,&failing);
 SimulateGameSubmission(state,GameDraw(),image_list,Push,&failing);
 result=SimulateGameSubmission(state,TopTransfer(),image_list,Push,&failing);
 assert(result.injection_failed&&!result.injected&&result.original_submitted);
 assert(failing.count==3); // input original only (synthetic fails)
 // Invalid buffer never attempts to inject.
 recorder=Recorder{};state=State{};
 SimulateGameSubmission(state,MemoryFill(),image_list,Push,&recorder);
 SimulateGameSubmission(state,GameDraw(),image_list,Push,&recorder);
 image_list.addr=0x14002001;
 result=SimulateGameSubmission(state,TopTransfer(),image_list,Push,&recorder);
 assert(!result.injected&&result.original_submitted&&recorder.count==3);
 std::puts("GX bridge: validated ordering and original-command fallback");
}
