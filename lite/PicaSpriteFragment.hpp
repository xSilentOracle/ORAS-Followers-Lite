// SPDX-License-Identifier: MIT
#pragma once
#include "PicaPackets.hpp"
// CPU-only fragment pipeline: sample RGBA8 from texture unit 0 and blend
// using source alpha. Needs full restoration of ORAS graphics state before
// use in a real GX hook (NOT implemented).
namespace PicaSpriteFragment {
bool Append(PicaPackets::Writer& writer);
}
