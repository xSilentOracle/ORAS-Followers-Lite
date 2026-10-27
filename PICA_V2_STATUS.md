# PICA sprite compositor v2 — work in progress (NO INSTALLABLE PLUGIN)

The target remains your original `mmo_pokes_3d.bin` for all 721 species, drawn as their existing multi-direction animated sprites by the **game's GPU**, while Tico renders Omega Ruby at 3×. No trainer texture swap and no modification of Tico.

## What works so far

- Local pack parsing, lead-species selection, and follower tracking exist from the previous ORAS-MMO adaptation.
- CPU-only RGB565-to-tiled RGBA8 staging is now in `lite/PicaFrameCodec.hpp/.cpp`.
- All 400×240 scene coordinates fit inside one padded 512×256 texture, so the previous 128×128 cropping is gone.
- Transparent pixels and partial off-screen clipping are tested by `tests/test_pica_codec.cpp`. See the **Validate PICA sprite codec** GitHub Action.

## What is blocked

The initial PICA attempt called `C3D_FrameBegin`, `C2D_SceneBegin`, and `C3D_FrameEnd` **inside Omega Ruby's nn::gx command-submission hook**. The user reported an orange screen and Switch Lite shutdown, though their Switch recovered. That path must not be run again.

The old `GpuFollower.cpp` was replaced with a CPU-only scratch target and PICA swizzled RGBA8 preparation; it makes **no GPU commands and no framebuffer writes**. The old GPU-hook build workflow stays disabled. The separate `build-v2-staging.yml` now compiles the complete plugin as a **non-displaying developer integration build**; a successful compile is not a playable follower release.

The next engineering task is a queue-safe GPU integration which leaves Omega Ruby's command queue ownership, VRAM target and synchronization intact. The converter passing tests and the GPU-disabled .3gx building successfully do not establish this. Only after verifying a safe integration with emulator instrumentation (and code review) would compiling a test plugin make sense.

Neither a trainer model reskin nor a game-specific static texture replacement is planned for this route. Keep the existing working native-render trainer test if useful for comparison, and do not install the failed PICA .3gx.

## New GPU queue-ordering work

- `lite/GxQueuePlan.hpp/.cpp` recognizes the game's GX memory-fill, command-list and top-screen transfer operations and rejects malformed or duplicate transfer candidates. The ordering contract matches the command IDs and eight-word packet layout documented by 3dbrew and handled by Tico's `src/video_core/gpu.cpp`.
- `lite/GxQueueBridge.hpp/.cpp` is a **host-only, non-installed** model of inserting a type-1 command-list packet immediately before the game's original top-screen transfer. It preserves the original game command even if the synthetic packet fails. This does not generate any PICA draw commands.
- `tests/test_gx_queue_plan.cpp` and `tests/test_gx_queue_bridge.cpp` cover request ordering, VRAM boundaries, alignment, duplicate frames and original-command fallback; GitHub Actions validates both without executing any GPU operations.
- The cross-compiler workflow also compiles both queue-model components with the 3DS ARM toolchain to check ABI/build compatibility. Runtime GPU submission remains **disabled**.

### Specific blockers before a real sprite-rendering plugin

1. Generate a PICA200 command-list containing a textured quad, valid shader state, blending, scissoring and the current field render target without taking ownership of Citra/Tico's or Omega Ruby's rendering queue.
2. Prove the packet buffer is GPU-addressable and cache-coherent; ensure the game's queue has room and preserves command ordering when a sprite packet is inserted.
3. Restore all modified PICA render state before ORAS renders the lower screen; un-restored render state is a crash and corruption risk.
4. Validate the sequence using an emulator GPU trace and confirm 3× surfaces are retained, **before** any Switch install recommendation.

**No new release .3gx is available yet. Do not install any earlier crashed `OrasPicaPokemonExperimental.3gx`.**

## 2026-10-26: Native PICA list development

I replaced the unsafe Citro3D-in-GX-hook approach with an offline native PICA command-list implementation. Files added:

- `PicaPackets.hpp/.cpp`: bounded PICA register packet writer, including masked, incremental and non-incremental writes and correct 16-byte finalization. Validated against libctru's `GPUCMD_AddInternal` implementation.
- `SpriteQuad.hpp/.cpp`: builds two textured triangles in ORAS's 256x512 rotated 3DS render-target coordinates, using the sprite's original 400x240 logical frame bounds.
- `sprite_shader.v.pica`: pass-through PICA vertex shader. Verified that source assembles using pinned upstream devkitPro Picasso commit `d522455ea59cd5cb9219c699b93eb7750039233c`.
- `PicaVertexUpload.hpp/.cpp`: emits PICA shader-code/operand/outmap commands derived from libctru's implementation.
- `PicaSpriteSetup.hpp/.cpp`: emits transparent RGBA8 texture sampling and 6-vertex input bindings, matching citro3d's `texture.c`, `attribs.c` and `buffers.c`.
- `PicaDrawArrays.hpp/.cpp`: emits the native 11-register draw sequence matching `C3D_DrawArrays(GPU_TRIANGLES)`.
- `PicaSpriteFragment.hpp/.cpp`: alpha blending and pass-through TEV source texture stage based on citro3d.
- `PicaSpriteList.hpp/.cpp`: composes the offline shader, texture, fragment and triangle draw packets in one list. **Explicitly returns `safe_to_submit=false`.**

Each module has a host-side test in the `tests/` directory with a separate GitHub Action. The ARM 3GX compile workflow includes all modules. **No code on this branch sends the composed PICA commands to the real GPU.**

### Critical unresolved blocker

The game's current GPU state must be preserved. A sprite draw list modifies shader code, TEV stages, vertex buffers, alpha/depth/color state, and texture bindings. The plugin needs a **verified context-save/restore strategy** or a truly isolated game render pass before it can run. The previous test crashed the Switch because it entered a separate renderer from inside the game's GPU submission hook; the new code avoids that, but correctness of live GPU queue submission and state restoration is not yet established. We do not have an ORAS/Tico GPU trace from the affected Switch and have not validated runtime behavior. Do not install a newly built GPU-output-enabled .3gx based on this branch until these issues are solved and verified.

## Integrated build milestone (October 26, 2026)

**Compiled cross-platform on GitHub Actions:** [Run 40448322000](https://github.com/xSilentOracle/ORAS-Followers-Lite/actions/runs/40448322000).

The formerly detached modules are now linked into the same .3gx. The GpuFollower::Publish callback uses the existing 721-species decoder's offscreen RGB565 sprite, converts it to a full-size 512×256 PICA tile texture in GPU-addressable linear memory, prepares six quad vertices, and generates a bounded PICA200 draw list with a real vertex shader assembled by pinned Picasso. Each frame's sprite texture and vertices are cache-flushed. **The generated command list is NOT submitted** to the game's GPU. Consequently this artifact cannot display Pokémon and is not offered as a release.

Key corrections and validation:

- SpriteQuad.cpp now uses Omega Ruby's **240×400 viewport**, with a **256-pixel tiled row stride**. The earlier 256×512 viewport was incorrect; tested full-screen corner mapping in tests/test_sprite_quad.cpp.
- GxQueuePlan::EncodeProcessList emits the full 0x01000101 GX header matching upstream libctru, not the shorthand command ID.
- PicaTargetGuard verifies the game's render-target addresses, expected dimensions, color format and color masks; it never changes the render target.
- PicaStateAudit and PicaStateRestore parse GPU writes, validate masks and capture reversible register bytes. They refuse to claim shader uploads and draw side effects can be reversed by register writes alone.
- PicaShaderMirror and PicaShaderReplay capture observed shader instruction and operand uploads and emit a replay list, but do not prove full shader-state restoration.
- Synthetic host tests pass. They do not reproduce Tico's actual Omega Ruby GPU command stream or prove the emulator accepts an injected sprite draw.

**Current blocker:** Integrate a real top-screen GX submission hook with verified stream-specific shader capture, original render-target checks, and full GPU-state restoration. Validate in an emulator harness before risking a Switch install. The earlier Citro3D-in-hook build triggered a console crash and must never be reused. Until that work is verified, GpuFollower::Install deliberately performs no GPU hook.
