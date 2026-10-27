#include "../lite/PicaShaderReplay.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaShaderMirror;
using namespace PicaShaderReplay;
using namespace PicaPackets;
int main(){
 std::array<uint32_t,256> words{};
 Snapshot game{};
 Writer original(words.data(),words.size());
 uint32_t code[]={0x1111u,0x2222u,0x3333u,0x4444u};
 uint32_t desc[]={0x5555u,0x6666u};
 assert(original.Write(0x02CB,0));
 assert(original.WriteMany(0x02CC,code,4,false));
 assert(original.Write(0x02BF,1));
 assert(original.Write(0x02D5,0));
 assert(original.WriteMany(0x02D6,desc,2,false));
 assert(Observe(game,words.data(),original.SizeWords()));
 std::array<uint32_t,256> replay{};
 auto r=Build(game,replay.data(),replay.size());
 assert(r.ready&&r.code_words==4&&r.descriptor_words==2);
 assert(r.words%4==0);
 // Re-parse emitted commands into a fresh memory mirror and compare.
 Snapshot copied{};
 assert(Observe(copied,replay.data(),r.words));
 assert(Inspect(copied).replay_candidate);
 assert(copied.code_highwater==game.code_highwater);
 assert(copied.desc_highwater==game.desc_highwater);
 for(unsigned i=0;i<4;i++)assert(copied.code[i]==game.code[i]);
 for(unsigned i=0;i<2;i++)assert(copied.descriptors[i]==game.descriptors[i]);
 std::array<uint32_t,2> tooSmall{};
 assert(!Build(game,tooSmall.data(),tooSmall.size()).ready);
 Snapshot invalid=game;
 invalid.descriptor_seen[0]=0;
 assert(!Build(invalid,replay.data(),replay.size()).ready);
 invalid=game;
 invalid.upload_ended=false;
 assert(!Build(invalid,replay.data(),replay.size()).ready);
 std::puts("PICA shader replay: reconstructed game shader/upload bytes offline");
}
