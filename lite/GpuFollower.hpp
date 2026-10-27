#pragma once
#include <3ds.h>
#include "Render3D.hpp"
namespace GpuFollower {
bool Init();
bool Install();
void Fini();
void ScratchTarget(R3D::Target &t);
void Publish(bool drew);
void Clear();
}
