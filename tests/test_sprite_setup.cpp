#include "../lite/PicaSpriteSetup.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaSpriteSetup;
using namespace PicaPackets;
static bool Find(const uint32_t* p,size_t n,uint16_t reg,uint32_t param) {
 size_t i=0;
 while(i+1<n){
   const auto x=DecodeHeader(p[i+1]);
   if(x.reg==reg&&p[i]==param)return true;
   i+=2+(x.parameters-1)+((x.parameters-1)&1u);
 }
 return false;
}
int main(){
 std::array<uint32_t,512> words{};
 Writer w(words.data(),words.size());
 Resources r{0x20000000u,0x20080000u};
 assert(Append(w,r));
 assert(w.Finalize());
 assert(Find(words.data(),w.SizeWords(),0x80,0x1u));
 assert(Find(words.data(),w.SizeWords(),0x81,0));
 assert(Find(words.data(),w.SizeWords(),0x8E,0)); // RGBA8
 assert(Find(words.data(),w.SizeWords(),0x2B9,0xA0000001u));
 assert(Find(words.data(),w.SizeWords(),0x242,1));
 assert(Find(words.data(),w.SizeWords(),0x200,0x18000000u>>3));
 assert(Find(words.data(),w.SizeWords(),0x203,0x08080000u));
 assert(w.SizeWords()%4==0);
 {
   Writer small(words.data(),8);
   assert(!Append(small,r));
   assert(small.Failed());
 }
 {
   Writer bad(words.data(),words.size());
   assert(!Append(bad,{0x20000001u,0x20080000u}));
   assert(bad.SizeWords()==0);
   assert(!Append(bad,{0x17F00000u,0x20080000u}));
   assert(!Append(bad,{0x20000000u,0x2FFFFFF8u}));
 }
 std::puts("PICA sprite setup: texture+VBO register layout and bounds passed");
}
