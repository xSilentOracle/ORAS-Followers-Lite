// SPDX-License-Identifier: MIT
#include "NativeBillboardStage.hpp"
#include <cstring>
namespace NativeBillboardStage {
static inline uint32_t ExpandColor(uint16_t p) {
 if(!p) return 0;
 const uint32_t r5=(p>>11)&31,g6=(p>>5)&63,b5=p&31;
 const uint32_t r8=(r5<<3)|(r5>>2),g8=(g6<<2)|(g6>>4),b8=(b5<<3)|(b5>>2);
 return (r8<<24)|(g8<<16)|(b8<<8)|0xFFu;
}
bool ConvertRgb565ToRgba(const uint16_t* src,int stride,int x0,int y0,int x1,int y1,
 uint32_t* dst,Rect& rect) {
 rect={0,0,0,0};
 if(!dst)return false;
 std::memset(dst,0,(size_t)kTextureSize*kTextureSize*sizeof(uint32_t));
 if(!src||stride<kScreenWidth||x1<=x0||y1<=y0)return false;
 if(x0<0)x0=0; if(y0<0)y0=0;
 if(x1>kScreenWidth)x1=kScreenWidth;
 if(y1>kScreenHeight)y1=kScreenHeight;
 if(x0>=x1||y0>=y1)return false;
 if(x1-x0>kTextureSize) {x0+=(x1-x0-kTextureSize)/2;x1=x0+kTextureSize;}
 if(y1-y0>kTextureSize) {y0+=(y1-y0-kTextureSize)/2;y1=y0+kTextureSize;}
 const int w=x1-x0,h=y1-y0;
 for(int y=0;y<h;++y) {
  const uint16_t* row=src+(size_t)(y0+y)*stride+x0;
  uint32_t* out=dst+(size_t)y*kTextureSize;
  for(int x=0;x<w;++x)out[x]=ExpandColor(row[x]);
 }
 rect={x0,y0,w,h};return true;
}
}