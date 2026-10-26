# Native renderer proof of concept (NOT a Pokémon follower yet)

This branch builds a separate `OrasNativeGpuTest.3gx` plugin to test whether Omega Ruby's own field-model renderer preserves Dekopon/Tico's 3× internal rendering.

**What it does:** Uses ORAS-MMO's existing guarded `GameSystem_Update` hook to create one additional in-game character (Brendan model ID 172). It follows a delayed trail behind the player. No direct CPU framebuffer writes or 3GX OSD callback are used.

**What it does not do:** Render Pokémon sprites, load `mmo_pokes_3d.bin`, inject Pokémon battle models into the field renderer, change Tico, or guarantee 3× scaling. Rendering correctness on Tico has not been tested.

**Compatibility:** Original project identifies the native hook addresses for a specific US Omega Ruby build (v1.0); it performs a build signature check and refuses to install the hook if the game binary differs. Other regional builds are unsupported. The plugin uses a native NPC model which may collide with the player's character, and could crash or create unexpected behavior. Back up your save before testing; avoid saving during tests. No assurance of compatibility with all game states.

To test, remove any other .3gx file from the Omega Ruby plugin directory before installing the test file. Your `mmo_pokes_3d.bin` is not required by this test. Remove the test plugin to restore your previous behavior.

The real end goal needs the 721-species sprites transformed into something the game's PICA renderer can display (e.g. billboards inserted into the game's field rendering pipeline), or in-game support for loading Pokémon models as field models. **Neither of those render paths has been implemented yet.**

Source: MIT-licensed original from SamuTecYT/ORAS-MMO; modified build workflow on this branch.
