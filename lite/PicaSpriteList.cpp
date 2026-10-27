// SPDX-License-Identifier: MIT
#include "PicaSpriteList.hpp"
#include "PicaDrawArrays.hpp"
#include "PicaSpriteFragment.hpp"
namespace PicaSpriteList {
BuildResult Build(uint32_t* dst,size_t capacity,
                  const PicaVertexUpload::Shader& shader,
                  const PicaSpriteSetup::Resources& res) {
    BuildResult out{};
    PicaPackets::Writer w(dst,capacity);
    if (!PicaVertexUpload::Append(w,shader))return out;
    if (!PicaSpriteSetup::Append(w,res))return out;
    if (!PicaSpriteFragment::Append(w))return out;
    if (!PicaDrawArrays::AppendTriangles(w,0,6))return out;
    if (!w.Finalize())return out;
    out.built=true;
    out.words=w.SizeWords();
    // Explicit fail-closed: a packet may encode perfectly and still crash a
    // console if inserted while the game owns the GPU command queue.
    out.safe_to_submit=false;
    return out;
}
}
