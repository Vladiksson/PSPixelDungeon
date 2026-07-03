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
     PSP_CTRL_CROSS   = 0x00004000   (the X button)
     PSP_CTRL_CIRCLE  = 0x00002000
     PSP_CTRL_LTRIGGER= 0x00000100
     PSP_CTRL_RTRIGGER= 0x00000200
     PSP_CTRL_START   = 0x00000008
     PSP_CTRL_SELECT  = 0x00000001
─────────────────────────────────────────────────────────────────── */

#ifdef PSP
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

/* ── Desktop path ─────────────────────────────────────────────────
   When building for PC (testing without PPSSPP / real hardware),
   SDL keyboard and gamepad events are used instead.
─────────────────────────────────────────────────────────────────── */
#else   /* !PSP */

void input_init(InputState *inp) {
    inp->held = inp->pressed = inp->released = inp->quit = 0;
    SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    /* Open first gamepad if present (lets you test with a controller) */
    if (SDL_NumJoysticks() > 0)
        SDL_JoystickOpen(0);
}

static Button scancode_to_btn(SDL_Scancode sc) {
    switch (sc) {
        case SDL_SCANCODE_UP:    case SDL_SCANCODE_W: return BTN_UP;
        case SDL_SCANCODE_DOWN:  case SDL_SCANCODE_S: return BTN_DOWN;
        case SDL_SCANCODE_LEFT:  case SDL_SCANCODE_A: return BTN_LEFT;
        case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_D: return BTN_RIGHT;
        case SDL_SCANCODE_Z:                          return BTN_A;
        case SDL_SCANCODE_X:                          return BTN_B;
        case SDL_SCANCODE_Q:                          return BTN_L;
        case SDL_SCANCODE_E:                          return BTN_R;
        case SDL_SCANCODE_RETURN:                     return BTN_START;
        case SDL_SCANCODE_BACKSPACE:                  return BTN_SELECT;
        default:                                      return BTN_NONE;
    }
}

/* SDL2 gamepad button → our Button (for desktop controller testing) */
static Button joybutton_to_btn(uint8_t jb) {
    switch (jb) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:    return BTN_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  return BTN_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  return BTN_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return BTN_RIGHT;
        case SDL_CONTROLLER_BUTTON_A:          return BTN_A;
        case SDL_CONTROLLER_BUTTON_B:          return BTN_B;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return BTN_L;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return BTN_R;
        case SDL_CONTROLLER_BUTTON_START:      return BTN_START;
        case SDL_CONTROLLER_BUTTON_BACK:       return BTN_SELECT;
        default:                               return BTN_NONE;
    }
}

void input_update(InputState *inp) {
    inp->pressed  = 0;
    inp->released = 0;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                inp->quit = 1;
                break;

            case SDL_KEYDOWN: {
                if (ev.key.repeat) break;
                if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    inp->quit = 1; break;
                }
                Button b = scancode_to_btn(ev.key.keysym.scancode);
                if (b) { inp->held |= b; inp->pressed |= b; }
                break;
            }
            case SDL_KEYUP: {
                Button b = scancode_to_btn(ev.key.keysym.scancode);
                if (b) { inp->held &= ~b; inp->released |= b; }
                break;
            }

            case SDL_CONTROLLERBUTTONDOWN: {
                Button b = joybutton_to_btn(ev.cbutton.button);
                if (b) { inp->held |= b; inp->pressed |= b; }
                break;
            }
            case SDL_CONTROLLERBUTTONUP: {
                Button b = joybutton_to_btn(ev.cbutton.button);
                if (b) { inp->held &= ~b; inp->released |= b; }
                break;
            }
        }
    }
}

#endif  /* PSP */