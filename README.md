# ORAS Followers Lite (experimental)

A lightweight offline-only adaptation of [SamuTecYT/ORAS-MMO](https://github.com/SamuTecYT/ORAS-MMO), distributed under its MIT license.

Only the local lead Pokémon's follower is drawn. MMO networking, other players, chat, and MMO HUD are not started. Requires your own `mmo_pokes_3d.bin`, created from your game. **Do not upload your ROM or pack.**

**Experimental** — built with the public 3DS devkit toolchain. The renderer still writes to the 3DS framebuffer, which may interfere with Tico's 3× resolution and frame generation. Not tested on Switch.

The Actions workflow clones a pinned original upstream commit, overlays the modified entry point and renderer function, and builds `OrasFollowersLite.3gx`. Check Actions artifacts after a successful run.

This is not affiliated with Nintendo, GAME FREAK, or the original ORAS-MMO developer. Copyright and MIT licensing of upstream code remain with their owners.
