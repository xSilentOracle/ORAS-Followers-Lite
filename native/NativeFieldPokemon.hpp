// SPDX-License-Identifier: MIT
// Game-owned native field-model IDs from Sango Plugin's ORAS model enum.
// Only IDs with an explicitly verified species mapping are allowed.
// This is a native-rendering PLAYTEST table, NOT the 721 battle model loader.
#pragma once
#include <3ds.h>
namespace NativeFieldPokemon {
constexpr u16 kDefault = 0x1ED; // Zigzagoon
inline u16 ModelForSpecies(u16 species) {
 switch(species) {
 case 448: return 0x1C7; // Lucario
 case 150: return 0x1D5; // Mewtwo
 case 383: return 0x1D9; // Groudon
 case 382: return 0x1DB; // Kyogre
 case 384: return 0x1DD; // Rayquaza
 case 380: return 0x1DF; // Latias
 case 381: return 0x1E0; // Latios
 case 67: return 0x1E3; // Machoke
 case 352: return 0x1E7; // Kecleon
 case 377: return 0x1E8; // Regirock
 case 378: return 0x1E9; // Regice
 case 379: return 0x1EA; // Registeel
 case 261: return 0x1EB; // Poochyena
 case 300: return 0x1EC; // Skitty
 case 263: return 0x1ED; // Zigzagoon
 case 320: return 0x1EE; // Wailmer
 case 311: return 0x1EF; // Plusle
 case 312: return 0x1F0; // Minun
 case 307: return 0x1F1; // Meditite
 case 285: return 0x1F2; // Shroomish
 case 287: return 0x1F3; // Slakoth
 case 315: return 0x1F4; // Roselia
 case 276: return 0x1F5; // Taillow
 case 720: return 0x1F6; // Hoopa
 case 293: return 0x1F7; // Whismur
 case 309: return 0x1F8; // Electrike
 case 334: return 0x1F9; // Altaria
 default:return 0;
 }
}
inline bool IsAllowedModel(u16 model) {
 // Strict allow-list: NEVER feed unrecognized model IDs to this game's
 // native scene manager. The Mewtwo etc. field resources belong to game.
 switch(model) {
 case 0x1C7:case 0x1D5:case 0x1D9:case 0x1DB:case 0x1DD:
 case 0x1DF:case 0x1E0:case 0x1E3:case 0x1E7:case 0x1E8:
 case 0x1E9:case 0x1EA:case 0x1EB:case 0x1EC:case 0x1ED:
 case 0x1EE:case 0x1EF:case 0x1F0:case 0x1F1:case 0x1F2:
 case 0x1F3:case 0x1F4:case 0x1F5:case 0x1F6:case 0x1F7:
 case 0x1F8:case 0x1F9:return true;
 default:return false;
 }
}
} // namespace NativeFieldPokemon
