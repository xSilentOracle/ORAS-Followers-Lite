# Native model texture injection: development branch

The previous Citro2D/PICA hook experiment caused an orange-screen crash on the user's Switch Lite. **That code must not be run again.** This branch starts from the *working native trainer follower* instead of the failed GPU-hook branch.

The ORAS-MMO project's existing `SkinTex.cpp` demonstrates updating model textures in game memory, while `GameHook.cpp` demonstrates spawning and moving native 3D overworld actors without losing 3x upscaling.

The goal is a **dedicated flat/billboard field-model asset** rendered by Omega Ruby's normal graphics engine, animated by changing only its texture from frames decoded from `mmo_pokes_3d.bin`. Replacing a trainer's skin alone cannot change its mesh/silhouette. A unique texture and model are essential to avoid changing the protagonist too.

## Implemented here

`lite/NativeBillboardStage.cpp` provides **CPU-only** conversion of a sprite from a private 400x240 RGB565 scratch surface to logical 256x256 RGBA8 with alpha. Includes clipping/cropping and no runtime GPU command hooks. `lite/test_native_billboard.cpp` validates pixel colors, transparency, bounds, and cropping. The existing Follower3D pack decoder supplies all species; no rebuild of the 721-species pack is needed.

## Still unimplemented (do not install yet)

- A suitable **independent planar BCH field model** with a unique texture, installed via ORAS game assets or loaded into game-owned resources;
- safe injection of the converted sprite pixels into that **specific model's** GPU texture, without modifying the trainer's shared texture;
- the game's texture tiling/layout, cache invalidation, image-frame timing, and proper UV/camera orientation;
- confirmation on Tico without crashes.

The RGBA8 output here is **linear logical pixels**, not a ready-to-upload GPU texture. This is a development-stage source component **not a working all-Pokémon .3gx**. Do not install a new binary until those remaining pieces are completed and validated.

Run the isolated host tests:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -O2 lite/NativeBillboardStage.cpp lite/test_native_billboard.cpp -o stage-test
./stage-test
```
