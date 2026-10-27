// SPDX-License-Identifier: MIT
#include "PicaPackets.hpp"
#include <cstring>

namespace PicaPackets {
PacketInfo DecodeHeader(uint32_t word) {
    PacketInfo info{};
    info.reg=uint16_t(word & 0x3FFu);
    info.mask=uint8_t((word >> 16) & 0xFu);
    info.parameters=uint16_t(((word >> 20) & 0xFFu) + 1u);
    info.incremental=(word & 0x80000000u)!=0;
    info.valid=info.mask!=0u;
    return info;
}
Writer::Writer(uint32_t* output,size_t capacity_words):
    data_(output),capacity_(capacity_words) {
    if(!output || !capacity_words)failed_=true;
}
bool Writer::Write(uint16_t reg,uint32_t value,uint8_t mask) {
    return WriteMany(reg,&value,1,false,mask);
}
bool Writer::WriteChunk(uint16_t reg,const uint32_t* vals,size_t count,
                        bool incremental,uint8_t mask) {
    if(failed_ || finalized_ || !vals || count==0 || count>256 ||
       !mask || (mask & ~0xFu)!=0 || reg>0x3FFu ||
       (incremental && uint32_t(reg)+count>0x400u)){
        failed_=true;return false;
    }
    const size_t cost=2+(count-1)+((count-1)&1u);
    if(cost > capacity_-size_){failed_=true;return false;}
    data_[size_++]=vals[0];
    data_[size_++]=Header(reg,mask,uint8_t(count-1),incremental);
    if(count>1) {
        std::memcpy(&data_[size_],vals+1,(count-1)*sizeof(uint32_t));
        size_+=count-1;
        if((count-1)&1u)data_[size_++]=0; // libctru GPUCMD_AddInternal alignment
    }
    return true;
}
bool Writer::WriteMany(uint16_t reg,const uint32_t* values,size_t count,
                       bool incremental,uint8_t mask){
    if(!values || !count || finalized_ || failed_){failed_=true;return false;}
    while(count){
        const size_t n=count>256?256:count;
        if(!WriteChunk(reg,values,n,incremental,mask))return false;
        values+=n;count-=n;
        if(incremental)reg=uint16_t(reg+n);
    }
    return true;
}
bool Writer::Finalize(){
    if(failed_ || finalized_)return false;
    if(!Write(kFinalize,0x12345678u))return false;
    if(size_&3u){
        if(!Write(kFinalize,0x12345678u))return false;
    }
    if(size_&3u){failed_=true;return false;}
    finalized_=true;
    return true;
}
} // namespace PicaPackets
