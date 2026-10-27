// Validates GPUCMD_AddInternal layout from devkitPro libctru/source/gpu/gpu.c
#include "../lite/PicaPackets.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>
using namespace PicaPackets;
int main(){
    assert(Header(0x23,0xF,0,false)==0x000F0023u);
    assert(Header(0x23,0xF,3,true)==0x803F0023u);
    auto h=DecodeHeader(Header(0x11A,0x5,2,true));
    assert(h.valid&&h.reg==0x11A&&h.mask==5&&h.parameters==3&&h.incremental);
    assert(!DecodeHeader(Header(0x11A,0,0,false)).valid);
    std::array<uint32_t,300> out{};
    Writer writer(out.data(),out.size());
    assert(writer.Write(0x80,0xAABBCCDD));
    assert(out[0]==0xAABBCCDD&&out[1]==0x000F0080);
    uint32_t three[3]={1,2,3};
    assert(writer.WriteMany(0x81,three,3,true));
    assert(out[2]==1&&out[3]==0x802F0081u);
    assert(out[4]==2&&out[5]==3); // 3 params: no trailing pad
    uint32_t two[2]={4,5};
    assert(writer.WriteMany(0x87,two,2,false,1));
    assert(out[6]==4&&out[7]==0x00110087u);
    assert(out[8]==5&&out[9]==0); // 2 params: one padding word
    assert(writer.Finalize());
    assert(writer.SizeWords()%4==0);
    assert(out[10]==0x12345678u&&out[11]==0x000F0010u);
    assert(writer.SizeWords()==12);
    assert(!writer.Write(0x90,1));
    assert(!writer.Finalize());
    {
        std::array<uint32_t,2> small{};
        Writer w(small.data(),small.size());
        assert(w.Write(0x80,1));
        assert(!w.Write(0x81,2)); // never overwrite guard
        assert(w.Failed()&&small[0]==1);
    }
    {
        std::array<uint32_t,600> large{};
        Writer w(large.data(),large.size());
        std::array<uint32_t,257> vals{};
        for(size_t i=0;i<vals.size();++i)vals[i]=uint32_t(i);
        assert(w.WriteMany(0x40,vals.data(),257,true));
        assert(large[0]==0 && large[1]==Header(0x40,15,255,true));
        // first packet: 256 words + header + a 1-word aligner
        assert(large[257]==0); // pad due to 256 params
        assert(large[258]==256 && large[259]==Header(0x140,15,0,true));
        assert(w.Finalize());
        assert(w.SizeWords()%4==0);
    }
    {
        std::array<uint32_t,64> data{};
        Writer w(data.data(),data.size());
        uint32_t v[2]={1,2};
        assert(!w.WriteMany(0x3FF,v,2,true)); // would exceed 0x3ff
        assert(w.Failed());
    }
    {
        std::array<uint32_t,64> data{};
        Writer w(data.data(),data.size());
        assert(w.Write(0x20,1,0x1));
        assert(w.Write(0x21,2,0x5));
        assert(w.Finalize()); // PICA command-list ends aligned to 4 words
        assert(w.SizeWords()%4==0);
        assert(w.SizeBytes()==w.SizeWords()*4);
    }
    std::puts("PICA packet writer: exact libctru wire format and bounds tests passed");
}
