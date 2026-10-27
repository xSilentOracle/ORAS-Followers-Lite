// SPDX-License-Identifier: MIT
// Read-only Pokémon model archive indexer for Omega Ruby / Alpha Sapphire.
// No GPU commands, game hooks, or speculative game-function addresses.
#pragma once
#include <cstddef>
#include <cstdint>
namespace NativePokemonIndex {
constexpr uint16_t kNationalDexCount=721;
constexpr uint16_t kPackFilesPerVariant=8;
constexpr uint16_t kFirstVariantFile=3;
struct SpeciesEntry {
 uint16_t first_variant;
 uint8_t variant_count;
 uint8_t flags;  // bit 1: female variant; bit 2: forms available
};
struct Request {
 uint16_t species; // National Dex number, 1-721
 uint8_t form;
 bool female;
 bool shiny;
};
struct ModelFiles {
 uint32_t model;
 uint32_t texture;
 uint32_t pack_start;
 bool valid;
};
ModelFiles Resolve(const SpeciesEntry* entries,size_t count,Request request,
                   uint32_t archive_file_count);
} // namespace NativePokemonIndex
