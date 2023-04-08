#include <kernel.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <stdio.h>
#include <tamtypes.h>

#include "libpad.h"

#include "input.h"

/*
 * Global var's
 */
// pad_dma_buf is provided by the user, one buf for each pad
// contains the pad's current state
static char padBuf[256] __attribute__((aligned(64)));

static char actAlign[6];
static int actuators;

static u32 old_pad = 0;
static u32 new_pad;
static struct padButtonStatus buttons;
static u32 paddata;

const struct padButtonStatus &get_button_status() { return buttons; }
const u32 &get_paddata() { return paddata; };

/*
 * loadModules()
 */
static void loadModules(void) {
  int ret;

  ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
  if (ret < 0) {
    printf("sifLoadModule sio failed: %d\n", ret);
    SleepThread();
  }

  ret = SifLoadModule("rom0:PADMAN", 0, NULL);
  if (ret < 0) {
    printf("sifLoadModule pad failed: %d\n", ret);
    SleepThread();
  }
}

/*
 * waitPadReady()
 */
static int waitPadReady(int port, int slot) {
  int state;
  int lastState;
  char stateString[16];

  state = padGetState(port, slot);
  lastState = -1;
  while ((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
    if (state != lastState) {
      padStateInt2String(state, stateString);
      printf("Please wait, pad(%d,%d) is in state %s\n", port, slot,
             stateString);
    }
    lastState = state;
    state = padGetState(port, slot);
  }
  // Were the pad ever 'out of sync'?
  if (lastState != -1) {
    printf("Pad OK!\n");
  }
  return 0;
}

/*
 * initializePad()
 */
static int initializePad(int port, int slot) {

  int ret;
  int modes;
  int i;

  waitPadReady(port, slot);

  // How many different modes can this device operate in?
  // i.e. get # entrys in the modetable
  modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
  printf("The device has %d modes\n", modes);

  if (modes > 0) {
    printf("( ");
    for (i = 0; i < modes; i++) {
      printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
    }
    printf(")");
  }

  printf("It is currently using mode %d\n",
         padInfoMode(port, slot, PAD_MODECURID, 0));

  // If modes == 0, this is not a Dual shock controller
  // (it has no actuator engines)
  if (modes == 0) {
    printf("This is a digital controller?\n");
    return 1;
  }

  // Verify that the controller has a DUAL SHOCK mode
  i = 0;
  do {
    if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
      break;
    i++;
  } while (i < modes);
  if (i >= modes) {
    printf("This is no Dual Shock controller\n");
    return 1;
  }

  // If ExId != 0x0 => This controller has actuator engines
  // This check should always pass if the Dual Shock test above passed
  ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
  if (ret == 0) {
    printf("This is no Dual Shock controller??\n");
    return 1;
  }

  printf("Enabling dual shock functions\n");

  // When using MMODE_LOCK, user cant change mode with Select button
  padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

  waitPadReady(port, slot);
  printf("infoPressMode: %d\n", padInfoPressMode(port, slot));

  waitPadReady(port, slot);
  printf("enterPressMode: %d\n", padEnterPressMode(port, slot));

  waitPadReady(port, slot);
  actuators = padInfoAct(port, slot, -1, 0);
  printf("# of actuators: %d\n", actuators);

  if (actuators != 0) {
    actAlign[0] = 0; // Enable small engine
    actAlign[1] = 1; // Enable big engine
    actAlign[2] = 0xff;
    actAlign[3] = 0xff;
    actAlign[4] = 0xff;
    actAlign[5] = 0xff;

    waitPadReady(port, slot);
    printf("padSetActAlign: %d\n", padSetActAlign(port, slot, actAlign));
  } else {
    printf("Did not find any actuators.\n");
  }

  waitPadReady(port, slot);

  return 1;
}

void init_inputs() {
  int ret;
  int port = 0;
  int slot = 0;

  SifInitRpc(0);

  printf("Initializing input");

  loadModules();

  padInit(0);

  printf("PortMax: %d\n", padGetPortMax());
  printf("SlotMax: %d\n", padGetSlotMax(port));

  if ((ret = padPortOpen(port, slot, padBuf)) == 0) {
    printf("padOpenPort failed: %d\n", ret);
    SleepThread();
  }

  if (!initializePad(port, slot)) {
    printf("pad initalization failed!\n");
    SleepThread();
  }
}

void read_inputs() {
  printf("Reading inputs...\n");

  int ret;
  int port = 0;
  int slot = 0;

  ret = padGetState(port, slot);

  while ((ret != PAD_STATE_STABLE) && (ret != PAD_STATE_FINDCTP1)) {
    if (ret == PAD_STATE_DISCONN) {
      printf("Pad(%d, %d) is disconnected\n", port, slot);
    }
    ret = padGetState(port, slot);
  }

  ret = padRead(port, slot, &buttons); // port, slot, buttons

  if (ret != 0) {
    paddata = 0xffff ^ buttons.btns;

    new_pad = paddata & ~old_pad;
    old_pad = paddata;

    // Directions
    if (new_pad & PAD_LEFT) {
      printf("LEFT\n");
    }
    if (new_pad & PAD_DOWN) {
      printf("DOWN\n");
    }
    if (new_pad & PAD_RIGHT) {
      printf("RIGHT\n");
      /*
             padSetMainMode(port, slot,
                            PAD_MMODE_DIGITAL, PAD_MMODE_LOCK));
      */
    }
    if (new_pad & PAD_UP) {
      printf("UP\n");
    }
    if (new_pad & PAD_START) {
      printf("START\n");
    }
    if (new_pad & PAD_R3) {
      printf("R3\n");
    }
    if (new_pad & PAD_L3) {
      printf("L3\n");
    }
    if (new_pad & PAD_SELECT) {
      printf("SELECT\n");
    }
    if (new_pad & PAD_SQUARE) {
      printf("SQUARE\n");
    }
    if (new_pad & PAD_CROSS) {
      padEnterPressMode(port, slot);
      printf("CROSS - Enter press mode\n");
    }
    if (new_pad & PAD_CIRCLE) {
      padExitPressMode(port, slot);
      printf("CIRCLE - Exit press mode\n");
    }
    if (new_pad & PAD_TRIANGLE) {
      // Check for the reason below..
      printf("TRIANGLE (press mode disabled, see code)\n");
    }
    if (new_pad & PAD_R1) {
      actAlign[0] = 1; // Start small engine
      padSetActDirect(port, slot, actAlign);
      printf("R1 - Start little engine\n");
    }
    if (new_pad & PAD_L1) {
      actAlign[0] = 0; // Stop engine 0
      padSetActDirect(port, slot, actAlign);
      printf("L1 - Stop little engine\n");
    }
    if (new_pad & PAD_R2) {
      printf("R2\n");
    }
    if (new_pad & PAD_L2) {
      printf("L2\n");
    }

    // Test the press mode
    if (buttons.triangle_p) {
      printf("TRIANGLE %d\n", buttons.triangle_p);
    }
    // Start little engine if we move right analogue stick right
    if (buttons.rjoy_h > 0xf0) {
      // Stupid check to see if engine is already running,
      // just to prevent overloading the IOP with requests
      if (actAlign[0] == 0) {
        actAlign[0] = 1;
        padSetActDirect(port, slot, actAlign);
      }
    }
  }
}