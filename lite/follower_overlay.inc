
bool OnFollowerOnlyFrame(const Screen &screen) {
    if (!screen.IsTop || s_hold) return false;
    s_busy = true;
    const u64 now = osGetTime();
    Game::LocalState me = Game::Snapshot();
    Game::RefreshFlags(me);
    Game::RefreshPos(me);
    if (!me.valid || me.inBattle || me.playerState != Game::STATE_FREE || me.inApp) {
        s_trail[0].init = false;
        s_busy = false;
        return false;
    }
    const Stats::Mine mine = Stats::GetMine();
    if (!mine.okParty || mine.leadSpecies < 1 || mine.leadSpecies > 721) {
        s_trail[0].init = false;
        s_busy = false;
        return false;
    }
    Game::CameraState cam;
    if (!Game::ReadCamera(cam)) {
        s_busy = false;
        return false;
    }
    R3D::Target target;
    if (!MakeTarget(screen, target)) {
        s_busy = false;
        return false;
    }
    s_cam = &cam;
    s_zFresh = false;
    s_nWantR = 0;
    const float *M = cam.viewProj.m;
    const float rl = sqrtf(M[0] * M[0] + M[2] * M[2]);
    const float fl = sqrtf(M[12] * M[12] + M[14] * M[14]);
    s_camAxes = rl > 1e-6f && fl > 1e-6f;
    if (s_camAxes) {
        s_camRight[0] = M[0] / rl;
        s_camRight[1] = M[2] / rl;
        s_camFwd[0] = M[12] / fl;
        s_camFwd[1] = M[14] / fl;
    }
    static u64 last = 0;
    static u16 lastSpecies = 0;
    float dt = last && now > last ? (now - last) / 1000.f : 0.033f;
    last = now;
    if (dt > 0.10f) dt = 0.10f;
    if (dt < 0.f) dt = 0.f;
    if (lastSpecies != mine.leadSpecies) {
        s_trail[0].init = false;
        s_trail[0].radSp = 0;
        lastSpecies = mine.leadSpecies;
    }
    Trail &trail = s_trail[0];
    TrailStep(trail, me.x, me.y, me.z, me.yaw, me.zone, dt);
    bool drew = false;
    if (!trail.place) {
        PushOut(trail.fx, trail.fy, me.x, me.y, POKE_SPACE, 1.f);
        drew = DrawFollower(target, cam, trail, mine.leadSpecies, now,
                            nullptr, 0, true);
    }
    s_cam = nullptr;
    s_busy = false;
    return drew;
}
