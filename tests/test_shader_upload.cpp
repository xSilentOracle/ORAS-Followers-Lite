#include "../lite/PicaVertexUpload.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaPackets;
using namespace PicaVertexUpload;
static bool Contains(const uint32_t* words,size_t count,uint16_t reg,uint32_t param){
 for(size_t i=0;i+1<count;i+=2){
   if(words[i]==param && (words[i+1]&0x3FFu)==reg)return true;
   const auto h=DecodeHeader(words[i+1]);
   i+=(h.parameters-1)+((h.parameters-1)&1u);
 }
 return false;
}
int main(){
 std::array<uint32_t,5000> bytes{};
 std::array<uint32_t,8> code{};
 std::array<uint32_t,5> desc{};
 for(unsigned i=0;i<code.size();++i)code[i]=0x1000u+i;
 for(unsigned i=0;i<desc.size();++i)desc[i]=0x2000u+i;
 Shader s{};
 s.instructions=code.data();s.instruction_words=code.size();
 s.descriptors=desc.data();s.descriptor_words=desc.size();
 s.main_offset=0;s.outmap_mask=0x3u;
 s.outmap[0]=2u;s.outmap[1]=0x03020100u;s.outmap[2]=0x1F1F0004u;
 for(int i=3;i<8;i++)s.outmap[i]=0x1F1F1F1Fu;
 Writer writer(bytes.data(),bytes.size());
 assert(Append(writer,s));
 assert(writer.Finalize());
 assert(writer.SizeWords() > 35);
 assert(Contains(bytes.data(),writer.SizeWords(),0x02CB,0));
 assert(Contains(bytes.data(),writer.SizeWords(),0x02BF,1));
 assert(Contains(bytes.data(),writer.SizeWords(),0x02BA,0x7FFF0000u));
 assert(Contains(bytes.data(),writer.SizeWords(),0x02BD,0x3u));
 assert(Contains(bytes.data(),writer.SizeWords(),0x0289,0xA0000000u));
 {
  Writer small(bytes.data(),2);
  assert(!Append(small,s));
  assert(small.Failed());
 }
 {
  Writer w(bytes.data(),bytes.size());
  s.instruction_words=513;
  assert(!Append(w,s));
  assert(w.SizeWords()==0);
  s.instruction_words=code.size();
  s.main_offset=200;
  assert(!Append(w,s));
  s.main_offset=0;
  s.outmap[0]=8;
  assert(!Append(w,s));
 }
 std::puts("PICA vertex upload: shader, opcode, outmap and failure tests passed");
}
