// Host-side tests: no access to 3DS GPU or game ROM needed.
#include "../lite/PicaFrameCodec.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
using namespace PicaFrameCodec;
static uint32_t Expected(uint16_t c){
 if(!c)return 0;
 uint32_t r=((c>>11)&31u)*255u/31u,g=((c>>5)&63u)*255u/63u,b=(c&31u)*255u/31u;
 return (r<<24)|(g<<16)|(b<<8)|0xffu;
}
int main(){
 std::vector<uint16_t> screen(ScreenWidth*ScreenHeight);
 std::vector<uint32_t> tex(TexturePixels);
 std::mt19937 rng(721);
 for(auto& c:screen)c=uint16_t(rng());
 auto result=Encode(screen.data(),ScreenWidth,{0,0,400,240},tex.data(),tex.size());
 assert(result.visible);
 assert(result.bounds.right==400&&result.bounds.bottom==240);
 for(int y=0;y<256;y++)for(int x=0;x<512;x++)
    assert(ReadTiled(tex.data(),x,y)==(x<400&&y<240?Expected(screen[y*400+x]):0));
 result=Encode(screen.data(),ScreenWidth,{-70,-90,50,40},tex.data(),tex.size());
 assert(result.visible&&result.bounds.left==0&&result.bounds.top==0);
 for(int y=0;y<256;y++)for(int x=0;x<512;x++)
    assert(ReadTiled(tex.data(),x,y)==(x<50&&y<40?Expected(screen[y*400+x]):0));
 result=Encode(screen.data(),ScreenWidth,{500,240,530,260},tex.data(),tex.size());
 assert(!result.visible);
 result=Encode(screen.data(),ScreenWidth,{0,0,400,240},tex.data(),tex.size()-1);
 assert(!result.visible);
 std::puts("PICA frame codec: bounds, opacity, 512x256 swizzle tests passed");
}
