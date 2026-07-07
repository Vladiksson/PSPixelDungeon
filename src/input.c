#include "input.h"

/* ── PSP path ─────────────────────────────────────────────────────
   On PSP (real hardware + PPSSPP), SDL keyboard events don't exist.
   All button input goes through sceCtrl — the PSP controller API.
   We read it directly with sceCtrlReadBufferPositive() every frame.

   PSP_CTRL_* bitmasks (from <pspctrl.h>):
     PSP_CTRL_UP      = 0x00000010
     PSP_CTRL_DOWN    = 0x00000040
     PSP_CTRL_LEFT    = 0x00000080
     PSP_CTRL_RIGHT   = 0x00000020
     PSP_CTRL_CROSS   = 0x00004000   
     PSP_CTRL_CIRCLE  = 0x00002000
     PSP_CTRL_LTRIGGER= 0x00000100
     PSP_CTRL_RTRIGGER= 0x00000200
     PSP_CTRL_START   = 0x00000008
     PSP_CTRL_SELECT  = 0x00000001
─────────────────────────────────────────────────────────────────── */

#include <pspctrl.h>

void input_init(InputState *inp) {
    inp->held = inp->pressed = inp->released = inp->quit = 0;
    /* Allow the PSP controller to be sampled every vblank */
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

void input_update(InputState *inp) {
    /* Drain SDL events so the OS stays happy (handles SDL_QUIT) */
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
        if (ev.type == SDL_QUIT) inp->quit = 1;

    /* Read PSP controller state */
    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);
    unsigned int btns = pad.Buttons;

    uint32_t prev = inp->held;
    inp->held = 0;

    if (btns & PSP_CTRL_UP)       inp->held |= BTN_UP;
    if (btns & PSP_CTRL_DOWN)     inp->held |= BTN_DOWN;
    if (btns & PSP_CTRL_LEFT)     inp->held |= BTN_LEFT;
    if (btns & PSP_CTRL_RIGHT)    inp->held |= BTN_RIGHT;
    if (btns & PSP_CTRL_CROSS)    inp->held |= BTN_A;
    if (btns & PSP_CTRL_CIRCLE)   inp->held |= BTN_B;
    if (btns & PSP_CTRL_LTRIGGER) inp->held |= BTN_L;
    if (btns & PSP_CTRL_RTRIGGER) inp->held |= BTN_R;
    if (btns & PSP_CTRL_START)    inp->held |= BTN_START;
    if (btns & PSP_CTRL_SELECT)   inp->held |= BTN_SELECT;

    inp->pressed  = inp->held & ~prev;
    inp->released = ~inp->held & prev;

    /* Home button quits */
    if (btns & PSP_CTRL_HOME) inp->quit = 1;
}
