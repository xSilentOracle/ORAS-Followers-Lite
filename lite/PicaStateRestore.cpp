// SPDX-License-Identifier: MIT
#include "PicaStateRestore.hpp"
#include "PicaPackets.hpp"
#include <cstdint>
namespace PicaStateRestore {
namespace {
struct Accum {
    uint8_t masks[0x400]{};
};
// This function runs only after PicaStateAudit::AnalyzeRestore has validated
// the entire plan, so the callback still does bounds checking independently.
bool Collect(Accum& dest,const uint32_t* list,size_t count){
    if(!list || (count&1u)||count>0x20000u)return false;
    for(size_t i=0;i<count;){
        if(i+2>count)return false;
        const auto h=PicaPackets::DecodeHeader(list[i+1]);
        const size_t tail=h.parameters-1u;
        const size_t words=2u+tail+(tail&1u);
        if(!h.valid || h.reg>0x3FFu ||
           (h.incremental && uint32_t(h.reg)+h.parameters>0x400u) ||
           words>count-i)return false;
        for(size_t j=0;j<h.parameters;j++){
            const uint16_t reg=h.incremental ? uint16_t(h.reg+j) : h.reg;
            dest.masks[reg]|=h.mask;
        }
        i+=words;
    }
    return true;
}
}
Result Build(const PicaStateAudit::Shadow& before,const uint32_t* planned,
             size_t n,uint32_t* output,size_t capacity){
    Result result{};
    if(!output || !capacity || !planned)return result;
    const auto report=PicaStateAudit::AnalyzeRestore(before,planned,n);
    if(!report.restorable)return result;
    Accum changed{};
    if(!Collect(changed,planned,n))return result;
    PicaPackets::Writer writer(output,capacity);
    // For side-effect-free config registers, final ordering is immaterial.
    // Apply the exact masked bytes from the original game state.
    for(int reg=0;reg<0x400;reg++){
        const uint8_t mask=changed.masks[reg];
        if(mask){
            if(!writer.Write(uint16_t(reg),before.values[reg],mask))
                return result;
            result.modified_registers++;
        }
    }
    if(!writer.Finalize())return Result{};
    result.ready=true;
    result.words=writer.SizeWords();
    return result;
}
} // namespace PicaStateRestore
