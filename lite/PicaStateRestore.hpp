// SPDX-License-Identifier: MIT
#pragma once
#include "PicaStateAudit.hpp"
#include <cstddef>
#include <cstdint>
namespace PicaStateRestore {
// Restore only side-effect-free PICA scalar register values. Fail closed for
// shaders, draw calls, framebuffer operations, unknown registers or malformed
// packets. The result is a private CPU list; this NEVER submits to the GPU.
struct Result {
  bool ready{};
  size_t words{};
  size_t modified_registers{};
};
Result Build(const PicaStateAudit::Shadow& before,
             const uint32_t* planned,const size_t planned_words,
             uint32_t* output,size_t output_capacity_words);
} // namespace PicaStateRestore
