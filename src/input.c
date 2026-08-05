#include "input.h"
#include <pspctrl.h>

void input_init(InputState *inp) {
    inp->held = inp->pressed = inp->released = inp->quit = 0;
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
}

void input_update(InputState *inp) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
        if (ev.type == SDL_QUIT) inp->quit = 1;

    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);
    unsigned int btns = pad.Buttons;

    uint32_t prev = inp->held;
    inp->held = 0;

    if (btns & PSP_CTRL_UP)       inp->held |= BTN_UP;
    if (btns & PSP_CTRL_DOWN)     inp->held |= BTN_DOWN;
    if (btns & PSP_CTRL_LEFT)     inp->held |= BTN_LEFT;
    if (btns & PSP_CTRL_RIGHT)    inp->held |= BTN_RIGHT;
    if (btns & PSP_CTRL_CROSS)    inp->held |= BTN_CROSS;
    if (btns & PSP_CTRL_CIRCLE)   inp->held |= BTN_CIRCLE;
    if (btns & PSP_CTRL_TRIANGLE) inp->held |= BTN_TRIANGLE;
    if (btns & PSP_CTRL_SQUARE)   inp->held |= BTN_SQUARE;
    if (btns & PSP_CTRL_LTRIGGER) inp->held |= BTN_L;
    if (btns & PSP_CTRL_RTRIGGER) inp->held |= BTN_R;
    if (btns & PSP_CTRL_START)    inp->held |= BTN_START;
    if (btns & PSP_CTRL_SELECT)   inp->held |= BTN_SELECT;

    inp->pressed  = inp->held & ~prev;
    inp->released = ~inp->held & prev;
}

void input_get_move(InputState *inp, int *dx, int *dy) {
    *dx = 0;
    *dy = 0;

    if (btn_held(inp, BTN_LEFT))
        *dx = -1;
    else if (btn_held(inp, BTN_RIGHT))
        *dx = 1;

    if (btn_held(inp, BTN_UP))
        *dy = -1;
    else if (btn_held(inp, BTN_DOWN))
        *dy = 1;
}