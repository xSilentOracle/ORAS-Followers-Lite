#include "../lite/PicaSpriteList.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaSpriteList;
int main(){
 std::array<uint32_t,16> instructions{};
 std::array<uint32_t,6> opdescs{};
 for(size_t i=0;i<instructions.size();++i)instructions[i]=uint32_t(i*101+4);
 for(size_t i=0;i<opdescs.size();++i)opdescs[i]=uint32_t(i*13+7);
 PicaVertexUpload::Shader s{};
 s.instructions=instructions.data();s.instruction_words=instructions.size();
 s.descriptors=opdescs.data();s.descriptor_words=opdescs.size();
 s.main_offset=0;
 s.outmap[0]=2;s.outmap[1]=0x03020100;s.outmap[2]=0x1f1f0004;
 for(int i=3;i<8;i++)s.outmap[i]=0x1f1f1f1f;
 s.outmap_mask=3u;
 std::array<uint32_t,8192> buffer{};
 auto result=Build(buffer.data(),buffer.size(),s,{0x20000000u,0x20080000u});
 assert(result.built&&!result.safe_to_submit);
 assert(result.words>=130 && result.words<buffer.size());
 assert(result.words%4==0);
 // The draw-array trigger precedes the finalize command pair.
 bool draw=false;
 for(size_t i=0;i+1<result.words;){
  const auto info=PicaPackets::DecodeHeader(buffer[i+1]);
  if(info.reg==0x22E&&buffer[i]==1)draw=true;
  i+=2+(info.parameters-1)+((info.parameters-1)&1u);
 }
 assert(draw);
 auto failed=Build(buffer.data(),20,s,{0x20000000u,0x20080000u});
 assert(!failed.built && !failed.safe_to_submit);
 assert(!Build(buffer.data(),buffer.size(),s,{1,0x20080000u}).built);
 std::puts("PICA command-list composition: valid offline GPU list, submission disabled");
}
