#include "../lite/PicaFrameCodec.hpp"
#include "../lite/SpriteQuad.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace SpriteQuad;
using namespace PicaFrameCodec;
int main() {
    SpriteInfo sprite{};
    sprite.visible=true;
    sprite.bounds={0,0,400,240};
    sprite.u0=0; sprite.v0=0;
    sprite.u1=400.f/512.f; sprite.v1=240.f/256.f;
    Triangles vertices{};
    assert(Build(sprite,vertices));
    // The (0,0) screen corner maps to (239,0) in the rotated PICA canvas.
    assert(std::abs(vertices[0].x-(2.f*(239.5f/256.f)-1.f))<1e-5f);
    assert(std::abs(vertices[0].y-(1.f-1.f/512.f))<1e-5f);
    assert(vertices[0].v==1.f&&vertices[0].u==0.f);
    assert(vertices[1].x==vertices[0].x);
    assert(vertices[1].y<vertices[0].y);
    assert(vertices[1].u>vertices[0].u);
    assert(vertices[2].x<vertices[0].x);
    assert(vertices[2].y==vertices[0].y);
    assert(vertices[2].v<vertices[0].v);
    assert(vertices[3].x==vertices[1].x&&vertices[3].y==vertices[1].y);
    assert(vertices[5].x==vertices[2].x&&vertices[5].y==vertices[2].y);
    // Large-sprite full-frame coordinate range: never crop here.
    assert(vertices[4].u == 400.f/512.f);
    sprite.bounds={220,100,270,155};
    sprite.u0=220.f/512.f;sprite.u1=270.f/512.f;
    sprite.v0=100.f/256.f;sprite.v1=155.f/256.f;
    assert(Build(sprite,vertices));
    assert(vertices[0].x>vertices[2].x);
    assert(vertices[0].u==sprite.u0);
    sprite.visible=false;
    assert(!Build(sprite,vertices));
    sprite.visible=true;sprite.bounds={300,100,299,101};
    assert(!Build(sprite,vertices));
    sprite.bounds={0,0,401,240};
    assert(!Build(sprite,vertices));
    std::puts("PICA sprite geometry: rotated 3DS canvas and UV tests passed");
}
