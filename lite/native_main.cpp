// Prototype: confirm Omega Ruby's own PICA rendering retains Dekopon HD scaling.
// Deliberately renders a *trainer* model from the game, NOT a Pokemon follower.
// Does not touch Screen.GetFramebuffer() or any 3DS framebuffer pixels.
#include "GameHook.hpp"
#include "GameState.hpp"
#include "Players.hpp"
#include "Protocol.hpp"
#include "Log.hpp"
#include <3ds.h>
#include <CTRPluginFramework.hpp>
#include <cstring>

using namespace CTRPluginFramework;
static volatile bool s_running = true;
void UiSendSimple(u8, u8, u8, const char*) {}

struct History {
    float x, y, z;
    u16 zone;
    bool ok;
};
static void NativeFollowerThread(void*) {
    History samples[24] = {};
    int next = 0, count = 0;
    u16 lastZone = 0xFFFF;
    while (s_running) {
        const u64 now = osGetTime();
        Game::Update(now);
        Game::LocalState me = Game::Snapshot();
        Game::RefreshFlags(me);
        Game::RefreshPos(me);
        const bool valid = me.valid && !me.inBattle &&
            me.playerState == Game::STATE_FREE && !me.inApp && me.ride == 0;
        MMO::RemoteView view = {};
        if (!valid || lastZone != me.zone) {
            memset(samples, 0, sizeof(samples));
            next = count = 0;
            lastZone = me.zone;
        }
        if (valid) {
            samples[next] = {me.x, me.y, me.z, me.zone, true};
            next = (next + 1) % 24;
            if (count < 24) count++;
            // Approx. 400ms following trail. The game's native actor handles its own animation.
            const int delay = count > 12 ? 12 : count - 1;
            const int index = (next + 24 - 1 - delay) % 24;
            const History& h = samples[index];
            view.valid = h.ok && h.zone == me.zone;
            view.id = 1;
            view.model = MMO_MODEL_BRENDAN;
            view.x = h.x; view.y = h.y; view.z = h.z;
            view.zone = me.zone;
            view.yaw = me.yaw;
            view.speed = me.speed;
            view.state = me.moveState;
        }
        GameHook::FeedViews(&view, 1, now);
        svcSleepThread(33000000LL);
    }
    const MMO::RemoteView blank = {};
    GameHook::FeedViews(&blank, 1, osGetTime());
}
namespace CTRPluginFramework {
void PatchProcess(FwkSettings&) {}
int main(void) {
    Log::Init();
    const bool hooked = GameHook::Install();
    Log::Write("Native-3D GPU test, native engine hook: %d", (int)hooked);
    // No OSD::Run callback: the 3GX never modifies the 3DS framebuffer.
    PluginMenu *menu = new PluginMenu("ORAS Native Draw Test", 0, 1, 0,
        "Rendering-path test. An NPC trainer (not a Pokemon) follows you; native game renderer only.");
    menu->SynchronizeWithFrame(true);
    Thread th = hooked ? threadCreate(NativeFollowerThread, nullptr, 0x8000, 0x31, -2, false) : nullptr;
    menu->Run();
    s_running = false;
    if (th) {
        threadJoin(th, 3000000000ULL);
        threadFree(th);
    }
    delete menu;
    return 0;
}
}