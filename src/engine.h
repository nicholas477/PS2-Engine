#pragma once
#include "camera.h"
#include "tamtypes.h"

namespace engine
{
extern camera* _camera;
void init();
void run();
u32 get_frame_counter();
}; // namespace engine