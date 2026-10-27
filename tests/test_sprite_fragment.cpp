#include "../lite/PicaSpriteFragment.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaPackets;
using namespace PicaSpriteFragment;
static bool Find(const uint32_t* p,size_t words,uint16_t reg,uint32_t val){
 size_t i=0;
 while(i+1<words){
  const auto info=DecodeHeader(p[i+1]);
  if(info.reg==reg&&p[i]==val)return true;
  i+=2+(info.parameters-1)+((info.parameters-1)&1u);
 }
 return false;
}
int main(){
 std::array<uint32_t,256> list{};
 Writer w(list.data(),list.size());
 assert(Append(w)&&w.Finalize());
 assert(Find(list.data(),w.SizeWords(),0xC0,0x00030003u));
 assert(Find(list.data(),w.SizeWords(),0xC8,0x000F000Fu));
 assert(Find(list.data(),w.SizeWords(),0xD0,0x000F000Fu));
 assert(Find(list.data(),w.SizeWords(),0xD8,0x000F000Fu));
 assert(Find(list.data(),w.SizeWords(),0xF0,0x000F000Fu));
 assert(Find(list.data(),w.SizeWords(),0xF8,0x000F000Fu));
 assert(Find(list.data(),w.SizeWords(),0x100,0x00E40100u));
 assert(Find(list.data(),w.SizeWords(),0x101,0x71760000u));
 assert(Find(list.data(),w.SizeWords(),0x107,0x0F10u));
 assert(w.SizeWords()%4==0);
 Writer shortWriter(list.data(),16);
 assert(!Append(shortWriter));
 assert(shortWriter.Failed());
 std::puts("PICA TEV+alpha blend: all texture stages and 3DS blend factors match");
}
