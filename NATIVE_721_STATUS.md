# Native Pokémon overworld models: 721-species development

Goal: draw the lead Pokémon with Omega Ruby's **own native 3D overworld renderer**, preserving Dekopon/Tico's 3× resolution and not modifying any other games. The existing user-created mmo_pokes_3d.bin is optional if the game's native Pokémon models can be loaded.

David Darras's Sango Plugin implements loading Pokémon models directly into the overworld scene, using Pokémon model and texture bundles from the game archive (archive ID 8), and the game's H3D resource and scene managers: https://github.com/David-Darras/sango-plugin/blob/master/lib/src/pokemon/model_loader.cc. Its documented target is Alpha Sapphire v1.4 **only**.

User's verified follower test is Omega Ruby USA v1.0, using a signature-checked game-specific native actor hook. Sango v1.4 absolute function pointers **must not** be used in OR v1.0 without reverse engineering and confirming correct signatures.

## Implemented on this branch

- native/NativePokemonIndex.hpp/.cpp: read-only archive selection by species (1–721), gender, form and shiny flag, with bounds validation.
- native/test_native_pokemon_index.cpp: synthetic 721-species archive-index tests, run on GitHub Actions.
- Base branch native-game-render-poc contains the *already verified on user's Switch* native trainer actor follower that retains 3×.

## Not implemented / not yet safe for play-testing

- Native OR v1.0 archive, allocator, H3D resource and scene-management function pointers.
- Pokémon model and texture resource registration, movement, animation, scale and cleanup.
- Validation with actual OR v1.0 code executable / emulator trace.

No new .3gx is offered from this branch. In particular, the earlier PICA experiment crashed the user's Switch and is not safe to install.