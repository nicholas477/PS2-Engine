#include "engine.h"
#include "gs.h"

#include <stdio.h>
#include <vector>

static std::vector<class tickable *> &get_tickables() {
  static std::vector<class tickable *> tickables;
  return tickables;
}

namespace engine {
static u32 frameCounter = 0;

void init() { gs::init(); }

static void tick() {

  if (frameCounter % 60 == 0) {
    printf("tick! framecounter: %d\n", frameCounter);
  }

  frameCounter++;
}

void run() {
  for (;;) {
    tick();
    gs::render();
  }
}
u32 get_frame_counter() { return frameCounter; }
} // namespace engine