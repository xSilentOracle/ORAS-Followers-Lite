// SPDX-License-Identifier: MIT
#include "PicaVertexUpload.hpp"
namespace PicaVertexUpload {
namespace {
constexpr uint16_t GEOSTAGE_CONFIG=0x0229,GEOSTAGE_CONFIG2=0x0253;
constexpr uint16_t VSH_COM_MODE=0x0244,VSH_ENTRYPOINT=0x02BA;
constexpr uint16_t VSH_OUTMAP_MASK=0x02BD,VSH_OUTMAP_TOTAL1=0x024A;
constexpr uint16_t VSH_OUTMAP_TOTAL2=0x0251;
constexpr uint16_t CODETRANSFER_CONFIG=0x02CB,CODETRANSFER_DATA=0x02CC;
constexpr uint16_t CODETRANSFER_END=0x02BF;
constexpr uint16_t OPDESCS_CONFIG=0x02D5,OPDESCS_DATA=0x02D6;
constexpr uint16_t OUTMAP_TOTAL=0x004F,OUTATTR_MODE=0x0064;
constexpr uint16_t OUTATTR_CLOCK=0x006F;
constexpr uint16_t PRIMITIVE_CONFIG=0x025E,GSH_MISC0=0x0252;
constexpr uint16_t GSH_MISC1=0x0254,GSH_INPUTBUFFER_CONFIG=0x0289;
}
bool Append(PicaPackets::Writer& w,const Shader& s){
    if(!s.instructions||!s.instruction_words||s.instruction_words>512||
       !s.descriptors||!s.descriptor_words||s.descriptor_words>4096||
       s.main_offset>=s.instruction_words||
       s.outmap[0]<1||s.outmap[0]>7)return false;
    if(!w.Write(GEOSTAGE_CONFIG,0,0x3)||!w.Write(GEOSTAGE_CONFIG2,0,0x3)||
       !w.Write(VSH_COM_MODE,0,0x1))return false;
    // libctru shaderProgramUploadDvle:
    if(!w.Write(CODETRANSFER_CONFIG,0))return false;
    for(size_t off=0;off<s.instruction_words;off+=128){
       const size_t n=(s.instruction_words-off)>128?128:s.instruction_words-off;
       if(!w.WriteMany(CODETRANSFER_DATA,s.instructions+off,n,false))return false;
    }
    if(!w.Write(CODETRANSFER_END,1)||!w.Write(OPDESCS_CONFIG,0))return false;
    if(!w.WriteMany(OPDESCS_DATA,s.descriptors,s.descriptor_words,false))
       return false;
    if(!w.Write(VSH_ENTRYPOINT,0x7FFF0000u|(s.main_offset&0xFFFFu))||
       !w.Write(VSH_OUTMAP_MASK,s.outmap_mask)||
       !w.Write(VSH_OUTMAP_TOTAL1,s.outmap[0]-1u)||
       !w.Write(VSH_OUTMAP_TOTAL2,s.outmap[0]-1u)||
       !w.Write(PRIMITIVE_CONFIG,s.outmap[0]-1u,0x1)||
       !w.WriteMany(OUTMAP_TOTAL,s.outmap,8,true)||
       !w.Write(OUTATTR_MODE,s.outmap_mode)||
       !w.Write(OUTATTR_CLOCK,s.outmap_clock))return false;
    // Geometry stage off. Does not touch the original game shader unless the
    // resulting command list is later actually submitted (NOT enabled).
    if(!w.Write(GEOSTAGE_CONFIG,0,0xAu)||
       !w.Write(GSH_MISC0,0)||!w.Write(GSH_MISC1,0)||
       !w.Write(GSH_INPUTBUFFER_CONFIG,0xA0000000u))return false;
    return true;
}
} // namespace PicaVertexUpload
