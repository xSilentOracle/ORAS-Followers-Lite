// SPDX-License-Identifier: MIT
#include "NativeBillboardStage.hpp"
#include <cassert>
#include <vector>
using namespace NativeBillboardStage;
int main() {
 std::vector<uint16_t> frame(kScreenWidth*kScreenHeight,0);
 std::vector<uint32_t> tex(kTextureSize*kTextureSize,0xDEADBEEF);
 Rect r{};
 frame[10*kScreenWidth+20]=0xF800;
 frame[10*kScreenWidth+21]=0x07E0;
 frame[11*kScreenWidth+20]=0x001F;
 assert(ConvertRgb565ToRgba(frame.data(),kScreenWidth,20,10,22,12,tex.data(),r));
 assert(r.x==20&&r.y==10&&r.w==2&&r.h==2);
 assert(tex[0]==0xFF0000FFu&&tex[1]==0x00FF00FFu);
 assert(tex[kTextureSize]==0x0000FFFFu&&tex[kTextureSize+1]==0);
 assert(ConvertRgb565ToRgba(frame.data(),kScreenWidth,-50,-50,21,11,tex.data(),r));
 assert(r.x==0&&r.y==0&&r.w==21&&r.h==11);
 assert(tex[10*kTextureSize+20]==0xFF0000FFu);
 assert(ConvertRgb565ToRgba(frame.data(),kScreenWidth,0,0,400,240,tex.data(),r));
 assert(r.w==256&&r.h==240&&r.x==72);
 assert(!ConvertRgb565ToRgba(frame.data(),kScreenWidth,500,500,510,510,tex.data(),r));
 assert(!r.w&&!r.h&&!tex[0]);
 assert(!ConvertRgb565ToRgba(frame.data(),399,0,0,1,1,tex.data(),r));
 return 0;
}