/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Naomi Peori <naomi@peori.ca>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <kernel.h>

#include "engine.h"
#include "input.h"

int main(int argc, char *argv[]) {

  engine::init();

  engine::run();

  // Sleep
  SleepThread();

  // End program.
  return 0;
}
