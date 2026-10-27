// SPDX-License-Identifier: MIT
#include "PicaDrawArrays.hpp"
namespace PicaDrawArrays {
bool AppendTriangles(PicaPackets::Writer& w,uint32_t first,uint32_t count){
    if(count==0||count>0xFFFFu||count%3u!=0||first>0xFFFFu)return false;
    // Mirrors devkitPro/citro3d/source/drawArrays.c; no GPU queue calls.
    return w.Write(0x025E,0,0x2) && // GPUREG_PRIMITIVE_CONFIG / triangles
           w.Write(0x025F,1) && // RESTART_PRIMITIVE
           w.Write(0x0227,0x80000000u) && // INDEXBUFFER_CONFIG
           w.Write(0x0228,count) && // NUMVERTICES
           w.Write(0x022A,first) && // VERTEX_OFFSET
           w.Write(0x0253,1,0x1) && // GEOSTAGE_CONFIG2
           w.Write(0x0245,0,0x1) && // START_DRAW_FUNC0
           w.Write(0x022E,1) && // DRAWARRAYS trigger
           w.Write(0x0245,1,0x1) && // restore config mode
           w.Write(0x0253,0,0x1) && // exit array draw mode
           w.Write(0x0231,1); // invalidate post-vertex cache
}
}
