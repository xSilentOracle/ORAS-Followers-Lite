#include "../lite/PicaShaderMirror.hpp"
#include "../lite/PicaPackets.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaShaderMirror;
using namespace PicaPackets;
int main(){
 Snapshot state{};
 std::array<uint32_t,128> list{};
 Writer instructions(list.data(),list.size());
 uint32_t code[]={0x87654321u,0x99992222u,0x0BADC0DEu,0x1000u};
 assert(instructions.Write(0x02CB,0));
 assert(instructions.WriteMany(0x02CC,code,4,false));
 assert(instructions.Write(0x02BF,1u));
 assert(Observe(state,list.data(),instructions.SizeWords()));
 auto status=Inspect(state);
 assert(status.complete_code_from_zero);
 assert(!status.replay_candidate);
 assert(status.code_words==4 && status.generation==1);
 for(int i=0;i<4;i++)assert(state.code[i]==code[i]);
 Writer descriptors(list.data(),list.size());
 uint32_t desc[]={0x11110000u,0xABCDABCDu};
 assert(descriptors.Write(0x02D5,0));
 assert(descriptors.WriteMany(0x02D6,desc,2,false));
 assert(Observe(state,list.data(),descriptors.SizeWords()));
 status=Inspect(state);
 assert(status.replay_candidate&&status.descriptor_words==2);
 assert(state.descriptors[0]==desc[0]&&state.descriptors[1]==desc[1]);
 // Restarting an incomplete shader invalidates the previous one.
 Writer incomplete(list.data(),list.size());
 assert(incomplete.Write(0x02CB,0));
 assert(incomplete.Write(0x02CC,0x12345678u));
 assert(Observe(state,list.data(),incomplete.SizeWords()));
 assert(!Inspect(state).replay_candidate);
 // Malformed packets cannot damage existing captured shader memory.
 const auto before=state;
 uint32_t invalid[]={0,Header(0x02CC,15,2,false),1u};
 assert(!Observe(state,invalid,3));
 assert(state.code[0]==before.code[0]);
 assert(state.generation==before.generation);
 // out-of-range descriptor address fails rather than overflowing.
 Writer invalidIndex(list.data(),list.size());
 assert(invalidIndex.Write(0x02D5,128u));
 assert(!Observe(state,list.data(),invalidIndex.SizeWords()));
 assert(state.descriptors[0]==before.descriptors[0]);
 // Masked writes cannot corrupt the mirror.
 Writer masked(list.data(),list.size());
 assert(masked.Write(0x02CB,1u,1u));
 assert(!Observe(state,list.data(),masked.SizeWords()));
 // Even valid full upload isn't a proven safe GPU restore.
 std::puts("PICA shader mirror: code and descriptor upload tracking passed");
}
