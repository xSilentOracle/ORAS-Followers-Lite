// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
#include "PicaVertexUpload.hpp"
#include "PicaSpriteSetup.hpp"
// Offline construction of the current PICA sprite command sequence.
// IMPORTANT: Not complete! Target, blending, texture combiner, raster state
// restoration and queue interoperability are still prerequisites.
namespace PicaSpriteList {
struct BuildResult {
    bool built{};
    size_t words{};
    bool safe_to_submit{}; // MUST remain false until full GPU state restore
};
BuildResult Build(uint32_t* dst,size_t capacity,
                  const PicaVertexUpload::Shader& shader,
                  const PicaSpriteSetup::Resources& resources);
} // namespace PicaSpriteList
