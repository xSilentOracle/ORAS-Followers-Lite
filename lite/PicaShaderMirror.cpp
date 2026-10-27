// SPDX-License-Identifier: MIT
#include "PicaShaderMirror.hpp"
#include "PicaPackets.hpp"
#include <cstring>
namespace PicaShaderMirror {
namespace {
constexpr uint16_t kCodeConfig=0x02CB,kCodeData=0x02CC;
constexpr uint16_t kCodeEnd=0x02BF,kDescConfig=0x02D5,kDescData=0x02D6;
bool Apply(Snapshot& s,uint16_t reg,uint32_t val){
 switch(reg) {
  case kCodeConfig:
    if(val>=kCodeCapacity)return false;
    if(val==0u){
      std::memset(s.code_seen,0,sizeof(s.code_seen));
      s.code_highwater=0;
      s.upload_started=true;
      s.upload_ended=false;
    }
    s.code_position_valid=true;
    s.code_cursor=val;
    return true;
  case kCodeData: {
    if(!s.code_position_valid || s.code_cursor>=kCodeCapacity)return false;
    const uint32_t idx=s.code_cursor++;
    s.code[idx]=val;s.code_seen[idx]=1;
    if(idx+1>s.code_highwater)s.code_highwater=uint16_t(idx+1);
    return true;
  }
  case kCodeEnd:
    if(val==1u && s.upload_started){
      s.upload_ended=true;
      ++s.generation;
    }
    return true;
  case kDescConfig:
    if(val>=kDescriptorCapacity)return false;
    if(val==0u){
      std::memset(s.descriptor_seen,0,sizeof(s.descriptor_seen));
      s.desc_highwater=0;
      s.descriptors_started=true;
    }
    s.desc_position_valid=true;
    s.desc_cursor=val;
    return true;
  case kDescData: {
    if(!s.desc_position_valid || s.desc_cursor>=kDescriptorCapacity)return false;
    const uint32_t idx=s.desc_cursor++;
    s.descriptors[idx]=val;s.descriptor_seen[idx]=1;
    if(idx+1>s.desc_highwater)s.desc_highwater=uint16_t(idx+1);
    return true;
  }
  default:return true;
 }
}
bool Parse(Snapshot& s,const uint32_t* words,size_t count){
 if(!words || !count || (count&1u) || count>0x20000)return false;
 for(size_t i=0;i<count;){
  if(i+2>count)return false;
  const uint32_t header=words[i+1];
  const auto x=PicaPackets::DecodeHeader(header);
  if(!x.valid || (x.incremental &&
      uint32_t(x.reg)+x.parameters>0x400u))return false;
  const size_t tail=x.parameters-1u;
  const size_t cost=2u+tail+(tail&1u);
  if(cost>count-i)return false;
  if((tail&1u) && words[i+2u+tail]!=0u)return false;
  for(size_t j=0;j<x.parameters;j++){
    const uint16_t reg=x.incremental?uint16_t(x.reg+j):x.reg;
    // Don't trust partially masked shader program words.
    if((reg==kCodeConfig||reg==kCodeData||reg==kCodeEnd||
        reg==kDescConfig||reg==kDescData) && x.mask!=15u)
      return false;
    const uint32_t value=j==0?words[i]:words[i+j+1];
    if(!Apply(s,reg,value))return false;
  }
  i+=cost;
 }
 return true;
}
} // namespace
bool Observe(Snapshot& dst,const uint32_t* packet,size_t count){
 Snapshot updated=dst;
 if(!Parse(updated,packet,count))return false;
 dst=updated;
 return true;
}
Report Inspect(const Snapshot& s){
 Report r{};
 r.code_words=s.code_highwater;
 r.descriptor_words=s.desc_highwater;
 r.generation=s.generation;
 r.complete_code_from_zero=s.upload_started && s.upload_ended &&
     s.code_highwater>0;
 for(size_t i=0;i<s.code_highwater;i++)
   if(!s.code_seen[i])r.complete_code_from_zero=false;
 r.complete_descriptors_from_zero=s.descriptors_started &&
     s.desc_highwater>0;
 for(size_t i=0;i<s.desc_highwater;i++)
   if(!s.descriptor_seen[i])r.complete_descriptors_from_zero=false;
 r.replay_candidate=r.complete_code_from_zero &&
     r.complete_descriptors_from_zero;
 // Even a replay candidate isn't a verified restore: shader geometry stage,
 // state registers and other shader bank slots still require validation.
 return r;
}
} // namespace PicaShaderMirror
