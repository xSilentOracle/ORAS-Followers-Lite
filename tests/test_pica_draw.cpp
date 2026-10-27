#include "../lite/PicaDrawArrays.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>
using namespace PicaPackets;
using namespace PicaDrawArrays;
int main(){
 std::array<uint32_t,128> memory{};
 Writer w(memory.data(),memory.size());
 assert(AppendTriangles(w,0,6));
 assert(w.Finalize());
 const uint16_t regs[11] = {0x25E,0x25F,0x227,0x228,0x22A,0x253,
                            0x245,0x22E,0x245,0x253,0x231};
 const uint32_t vals[11] = {0,1,0x80000000u,6,0,1,0,1,1,0,1};
 const uint8_t masks[11] = {2,15,15,15,15,1,1,15,1,1,15};
 for(int i=0;i<11;i++){
     assert(memory[2*i]==vals[i]);
     assert(memory[2*i+1]==Header(regs[i],masks[i],0,false));
 }
 assert(w.SizeWords()%4==0);
 {
  Writer small(memory.data(),4);
  assert(!AppendTriangles(small,0,6));
  assert(small.Failed());
 }
 {
  Writer valid(memory.data(),128);
  assert(!AppendTriangles(valid,0,0));
  assert(!AppendTriangles(valid,0,4));
  assert(!AppendTriangles(valid,0,0x10002));
 }
 std::puts("PICA draw arrays: exact 11-register GPU_TRIANGLES sequence passed");
}
