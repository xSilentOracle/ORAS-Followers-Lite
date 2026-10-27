# PICA sprite compositor v2 — work in progress (NO INSTALLABLE PLUGIN)

The target remains your original `mmo_pokes_3d.bin` for all 721 species, drawn as their existing multi-direction animated sprites by the **game's GPU**, while Tico renders Omega Ruby at 3×. No trainer texture swap and no modification of Tico.

## What works so far

- Local pack parsing, lead-species selection, and follower tracking exist from the previous ORAS-MMO adaptation.
- CPU-only RGB565-to-tiled RGBA8 staging is now in `lite/PicaFrameCodec.hpp/.cpp`.
- All 400×240 scene coordinates fit inside one padded 512×256 texture, so the previous 128×128 cropping is gone.
- Transparent pixels and partial off-screen clipping are tested by `tests/test_pica_codec.cpp`. See the **Validate PICA sprite codec** GitHub Action.

## What is blocked

The initial PICA attempt called `C3D_FrameBegin`, `C2D_SceneBegin`, and `C3D_FrameEnd` **inside Omega Ruby's nn::gx command-submission hook**. The user reported an orange screen and Switch Lite shutdown, though their Switch recovered. That path must not be run again.

The old `GpuFollower.cpp` is deliberately guarded with a compile-time `#error` on this branch, and its old installable-plugin build workflow is disabled. There is **no safe new .3gx file** from this branch.

The next engineering task is a queue-safe GPU integration which leaves Omega Ruby's command queue ownership, VRAM target and synchronization intact. The converter passing tests does not establish this. Only after verifying a safe integration with emulator instrumentation (and code review) would compiling a test plugin make sense.

Neither a trainer model reskin nor a game-specific static texture replacement is planned for this route. Keep the existing working native-render trainer test if useful for comparison, and do not install the failed PICA .3gx.
