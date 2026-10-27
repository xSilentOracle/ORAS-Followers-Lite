// SPDX-License-Identifier: MIT
#include "PicaTargetGuard.hpp"
namespace PicaTargetGuard {
namespace {
constexpr uint32_t kVramVirtual=0x1F000000u,kVramPhysical=0x18000000u;
constexpr uint32_t kVramEnd=0x1F600000u;
constexpr uint16_t kDepthLoc=0x11Cu,kColorLoc=0x11Du;
constexpr uint16_t kFbDim=0x11Eu,kColorFormat=0x117u;
constexpr uint16_t kColorRead=0x112u,kColorWrite=0x113u;
bool Known(const PicaStateAudit::Shadow& s,uint16_t reg) {
 return s.known_bits[reg]==0xFFFFFFFFu;
}
bool Translate(uint32_t virtualAddress,uint32_t& physical) {
 if(virtualAddress<kVramVirtual || virtualAddress>=kVramEnd ||
    (virtualAddress&7u))return false;
 physical=kVramPhysical + virtualAddress-kVramVirtual;
 return true;
}
}
Result MatchTop(const GxQueuePlan::Decision& pending,
                const PicaStateAudit::Shadow& s){
 Result r{};
 if(!pending.safe_position){r.failure=Failure::NotTopTransfer;return r;}
 constexpr uint16_t kRequired[]={kDepthLoc,kColorLoc,kFbDim,kColorFormat,
                                   kColorRead,kColorWrite};
 for(uint16_t reg:kRequired){
    if(!Known(s,reg)){r.failure=Failure::MissingRegisters;return r;}
 }
 if(!Translate(pending.render_color,r.color_physical) ||
    (s.values[kColorLoc]<<3)!=r.color_physical){
    r.failure=Failure::MismatchedColor;return r;
 }
 if(!Translate(pending.render_depth,r.depth_physical) ||
    (s.values[kDepthLoc]<<3)!=r.depth_physical){
    r.failure=Failure::MismatchedDepth;return r;
 }
 // The visible 240x400 top target uses a 256-texel Morton tile stride.
 constexpr uint32_t kExpectedDimensions=0x01000000u|((400u-1u)<<12)|240u;
 if(s.values[kFbDim]!=kExpectedDimensions){
    r.failure=Failure::BadDimensions;return r;
 }
 // PICA GPU_RGBA8 color format value is 0 and 4 bytes per pixel = size 2.
 if(s.values[kColorFormat]!=2u){
    r.failure=Failure::BadColorFormat;return r;
 }
 if((s.values[kColorRead]&0xFu)!=0xFu ||
    (s.values[kColorWrite]&0xFu)!=0xFu){
    r.failure=Failure::BadColorMask;return r;
 }
 r.matches=true;r.failure=Failure::None;
 return r;
}
} // namespace PicaTargetGuard
