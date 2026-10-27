# All 721 species — experimental PICA compositor

**Status: Cross-compiled, not validated on Tico or a console. May crash.**

This experimental variant aims to display **any lead Pokémon species ID 1–721**, using the existing `mmo_pokes_3d.bin` pre-rendered walking/idle animation pack. The binary intentionally has no MMO server, chat or multiplayer features.

It is NOT a verified working implementation. In particular, the PICA200 drawing library initializes its own GPU command queue inside a game that already has one, and this could cause a deadlock, display corruption or a crash. Compile success does not establish that the injected command order is correct.

### Mechanism

1. Read the local lead Pokémon from ORAS game memory and advance the existing trailing/animation logic.
2. Decode a directional animation using the original ORAS-MMO `Follower3D` code.
3. Render it into a private **RAM scratch buffer**, not the game's framebuffer.
4. Convert opaque RGB565 pixels into a tiled RGBA8 texture; zeros remain transparent.
5. Intercept the game's top-screen GPU transfer, submit an alpha-blended PICA quad on the field-render target, and allow the game's display transfer to proceed.

No Nintendo game assets or individual model pack are committed to this repository. Pack format is the existing MMO3D3 / MMO3D2.

### Safe trial

- Make a backup of your game save and retain the previously working `OrasNativeGpuTest.3gx`.
- **Only install one .3gx** in the Omega Ruby plugin folder at any given time.
- Put `OrasPicaPokemonExperimental.3gx` alongside `mmo_pokes_3d.bin` in `luma/plugins/000400000011C400/` in Tico's emulated 3DS SD.
- Start Omega Ruby with 3x enabled, test in an unimportant area, do not save until you are satisfied there is no corruption.
- If it freezes, distorts graphics or falls back to 1x, disable this plugin and restore the old one. Your other games need no changes.

**The module intentionally doesn't modify Tico, but installing a .3gx that hooks GPU commands carries game stability risks.** Expects the specific US ORAS executable for which the original game hook verifies its signature.

### Current engineering limitations

- Textures larger than 128x128 are cropped in the prototype.
- RGB565 transparency and tile swizzling are compiled but not visually validated.
- The game's own GPU command queue and Citro3D queue may conflict.
- Camera orientation and billboard timing may be wrong in-game.
- Does not presently sample the game's depth buffer, so a follower may overlap objects in front.
- The 3GX OSD callback returns false, and renders only into a private scratch buffer.
- No test on Tico/Dekopon has been run yet. Do not call it a release.

Sources derived from ORAS-MMO by SamuTecYT under the MIT license, plus citro2d/citro3d from devkitPro under their respective licenses. This branch is separate from the known-good native trainer test and the main branch.
