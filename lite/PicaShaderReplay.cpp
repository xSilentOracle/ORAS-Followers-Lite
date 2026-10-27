// SPDX-License-Identifier: MIT
#include "PicaShaderReplay.hpp"
namespace PicaShaderReplay {
Result Build(const PicaShaderMirror::Snapshot& s,
             uint32_t* output,size_t capacity_words) {
    Result result{};
    const auto report=PicaShaderMirror::Inspect(s);
    if(!output || !capacity_words || !report.replay_candidate)return result;
    PicaPackets::Writer w(output,capacity_words);
    if(!w.Write(0x02CB,0))return result;
    for(size_t i=0;i<report.code_words;i+=128) {
        const size_t n=report.code_words-i>128?128:report.code_words-i;
        if(!w.WriteMany(0x02CC,s.code+i,n,false))return result;
    }
    if(!w.Write(0x02BF,1))return result;
    if(!w.Write(0x02D5,0))return result;
    if(!w.WriteMany(0x02D6,s.descriptors,report.descriptor_words,false))
        return result;
    if(!w.Finalize())return result;
    result.ready=true;
    result.words=w.SizeWords();
    result.code_words=report.code_words;
    result.descriptor_words=report.descriptor_words;
    // Replaying these instructions is necessary but NOT sufficient to
    // restore the game: entrypoints, output map, uniforms, geometry program,
    // TEV, blending, queues and render targets still need matching state.
    return result;
}
} // namespace PicaShaderReplay
