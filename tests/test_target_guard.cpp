#include "../lite/PicaTargetGuard.hpp"
#include <cassert>
#include <cstdio>
using namespace PicaTargetGuard;
using namespace PicaStateAudit;
int main(){
 GxQueuePlan::Decision d{};
 d.safe_position=true;
 d.render_color=0x1F001000u;
 d.render_depth=0x1F201000u;
 Shadow gpu{};
 auto known=[&](uint16_t reg,uint32_t value){
  gpu.values[reg]=value;gpu.known_bits[reg]=0xFFFFFFFFu;
 };
 known(0x11C,0x18201000u>>3);
 known(0x11D,0x18001000u>>3);
 known(0x11E,0x01000000u|((400u-1u)<<12)|240u);
 known(0x117,2u);
 known(0x112,0xFu);known(0x113,0xFu);
 auto result=MatchTop(d,gpu);
 assert(result.matches&&result.failure==Failure::None);
 assert(result.color_physical==0x18001000u);
 assert(result.depth_physical==0x18201000u);
 d.safe_position=false;
 assert(MatchTop(d,gpu).failure==Failure::NotTopTransfer);
 d.safe_position=true;
 gpu.values[0x11D]^=8;
 assert(MatchTop(d,gpu).failure==Failure::MismatchedColor);
 gpu.values[0x11D]^=8;
 gpu.values[0x11C]^=8;
 assert(MatchTop(d,gpu).failure==Failure::MismatchedDepth);
 gpu.values[0x11C]^=8;
 gpu.values[0x11E]=0;
 assert(MatchTop(d,gpu).failure==Failure::BadDimensions);
 gpu.values[0x11E]=0x01000000u|((400u-1u)<<12)|240u;
 gpu.values[0x117]=0;
 assert(MatchTop(d,gpu).failure==Failure::BadColorFormat);
 gpu.values[0x117]=2;
 gpu.values[0x113]=0;
 assert(MatchTop(d,gpu).failure==Failure::BadColorMask);
 gpu.values[0x113]=0xF;
 gpu.known_bits[0x117]=0;
 assert(MatchTop(d,gpu).failure==Failure::MissingRegisters);
 std::puts("PICA target guard: physical mapping, viewport, format and mask checks passed");
}
