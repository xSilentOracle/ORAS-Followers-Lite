// SPDX-License-Identifier: MIT
#include "../lite/GxQueuePlan.hpp"
#include <cassert>
#include <cstdio>
using namespace GxQueuePlan;
static Command MakeFill(uint32_t color=0x1F000000u) {
 Command c{};c.words[0]=kMemoryFill;c.words[1]=color;
 c.words[4]=0x1F200000u;c.words[5]=0xFFFFFFFFu;
 c.words[6]=0x1F300000u;
 return c;
}
static Command MakeTop(uint32_t src=0x1F008000u) {
 Command c{};c.words[0]=kDisplayTransfer;
 c.words[1]=src;c.words[4]=kTopOutputDims;return c;
}
int main(){
 State state{};
 auto d=Observe(state,MakeTop());
 assert(d.recognized_top&&!d.safe_position);
 Observe(state,MakeFill());
 assert(state.color_valid&&state.frame_number==1);
 Command scratch{}; assert(EncodeProcessList(scratch,0x14000000u,64,true));
 Command list{};
 assert(EncodeProcessList(list,0x14000000u,0x1000,true));
 assert(list.words[0]==1&&list.words[1]==0x14000000u&&list.words[7]==1u);
 assert(!EncodeProcessList(list,0x14000001u,0x1000,true));
 assert(!EncodeProcessList(list,0x14000000u,0x1003,true));
 assert(!EncodeProcessList(list,0x14000000u,0,true));
 assert(!EncodeProcessList(list,0x14000000u,0x500000,true));
 EncodeProcessList(list,0x14000000u,0x1000,true);
 Observe(state,list);
 d=Observe(state,MakeTop());
 assert(d.recognized_top&&d.safe_position);
 assert(d.render_color==0x1F008000u&&d.render_depth==0x1F208000u);
 assert(d.last_draw_lists==1&&d.frame_number==1);
 d=Observe(state,MakeTop());
 assert(d.recognized_top&&!d.safe_position);
 Observe(state,MakeFill());
 d=Observe(state,MakeTop());
 assert(!d.safe_position); // must have a draw command each frame
 Observe(state,MakeFill());
 Observe(state,list);
 d=Observe(state,MakeTop(0x1F600000u));
 assert(!d.safe_position); // wrong region
 Observe(state,MakeFill());
 Observe(state,list);
 d=Observe(state,MakeTop(0x1F008001u));
 assert(!d.safe_position); // invalid alignment
 Observe(state,MakeFill(0x1F000004u));
 assert(state.frame_number==5); // malformed fill must not reset
 Observe(state,MakeFill());
 Command badList{};badList.words[0]=kProcessCommandList;
 badList.words[1]=0x14000001u;badList.words[2]=123u;
 Observe(state,badList);
 assert(!Observe(state,MakeTop()).safe_position);
 Command unrelated{};unrelated.words[0]=kDisplayTransfer;
 unrelated.words[4]=(320u<<16)|240u;
 Observe(state,MakeFill());Observe(state,list);
 assert(!Observe(state,unrelated).recognized_top);
 assert(Observe(state,MakeTop()).safe_position);
 std::puts("GX queue planner: packet validation and FIFO-order tests passed");
}
