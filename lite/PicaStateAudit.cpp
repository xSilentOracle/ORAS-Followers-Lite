// SPDX-License-Identifier: MIT
#include "PicaStateAudit.hpp"
#include <cstring>

namespace PicaStateAudit {
namespace {
using Callback=bool (*)(void*,uint16_t,uint8_t,uint32_t);
struct WalkerContext {
    void* ptr{};
    Callback cb{};
    size_t packets{};
};
bool Walk(const uint32_t* words,size_t size,WalkerContext& ctx) {
    if(!words || !size || size>0x20000 || (size&1u))return false;
    size_t pos=0;
    while(pos<size){
        if(pos+2>size)return false;
        const uint32_t first=words[pos],header=words[pos+1];
        const auto decoded=PicaPackets::DecodeHeader(header);
        if(!decoded.valid || decoded.reg>0x3FFu ||
           (decoded.incremental &&
            uint32_t(decoded.reg)+decoded.parameters>0x400u))return false;
        const size_t tail=decoded.parameters-1;
        const size_t cost=2+tail+(tail&1u);
        if(cost>size-pos)return false;
        // A padding word is written for an EVEN number of parameters.
        if((tail&1u) && words[pos+2+tail]!=0u)return false;
        for(size_t n=0;n<decoded.parameters;n++){
            const uint16_t reg=decoded.incremental ?
                uint16_t(decoded.reg+n):decoded.reg;
            const uint32_t value=(n==0)?first:words[pos+1+n];
            if(ctx.cb && !ctx.cb(ctx.ptr,reg,decoded.mask,value))return false;
        }
        ++ctx.packets;
        pos+=cost;
    }
    return pos==size;
}
uint32_t ExpandMask(uint8_t mask) {
    uint32_t out=0;
    for(int b=0;b<4;b++)
        if(mask & (1u<<b))out|=(0xFFu<<(8*b));
    return out;
}
bool IsSideEffect(uint16_t reg) {
    // Vertex command, buffer flush/invalidation, draw and cache triggers.
    switch(reg) {
        case 0x0010: // FINALIZE
        case 0x0110: case 0x0111: // framebuffer cache ops
        case 0x022E: case 0x022F: // draw arrays/elements
        case 0x0231: // post-vertex cache invalidation
        case 0x025F: // restart primitive
        case 0x02BF: // vertex shader code-transfer trigger
        case 0x029F: // fragment shader (?) reserved trigger
            return true;
        default:return false;
    }
}
bool IsShaderUpload(uint16_t reg) {
    // Shader program and operand descriptor memory is not in 0x400 register
    // slots, so shadow values cannot restore its contents.
    return (reg>=0x02CB && reg<=0x02D7) ||
           (reg>=0x0280 && reg<=0x028F);
}
struct UpdateContext {Shadow* shadow;};
bool Update(void* ptr,uint16_t reg,uint8_t mask,uint32_t value) {
    auto& state=*static_cast<UpdateContext*>(ptr)->shadow;
    const uint32_t m=ExpandMask(mask);
    state.values[reg]=(state.values[reg]&~m)|(value&m);
    state.known_bits[reg]|=m;
    return true;
}
struct AuditContext {
    const Shadow* shadow{};
    Report report{};
};
bool Audit(void* ptr,uint16_t reg,uint8_t mask,uint32_t) {
    auto& a=*static_cast<AuditContext*>(ptr);
    const uint32_t requested=ExpandMask(mask);
    if((a.shadow->known_bits[reg]&requested)!=requested) {
        if(a.report.all_registers_known)
            a.report.first_unknown_register=reg;
        a.report.all_registers_known=false;
    }
    if(IsSideEffect(reg))a.report.has_irreversible_effects=true;
    if(IsShaderUpload(reg))a.report.needs_shader_reupload=true;
    return true;
}
}
bool Observe(Shadow& shadow,const uint32_t* words,size_t count) {
    Shadow updated=shadow; // abort entire update on invalid packet streams
    UpdateContext update{&updated};
    WalkerContext ctx{&update,&Update,0};
    if(!Walk(words,count,ctx))return false;
    shadow=updated;
    return true;
}
Report AnalyzeRestore(const Shadow& prior,const uint32_t* words,size_t count){
    AuditContext info{};
    info.shadow=&prior;
    info.report.all_registers_known=true;
    WalkerContext ctx{&info,&Audit,0};
    const bool valid=Walk(words,count,ctx);
    info.report.parsed=valid;
    info.report.packet_count=ctx.packets;
    info.report.word_count=count;
    // A PICA shader upload and a draw cannot be reversed by a register
    // snapshot. A current register shadow doesn't capture execution state.
    info.report.restorable=valid && info.report.all_registers_known &&
        !info.report.needs_shader_reupload &&
        !info.report.has_irreversible_effects;
    return info.report;
}
} // namespace PicaStateAudit
