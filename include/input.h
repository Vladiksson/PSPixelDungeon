#pragma once
#include <SDL2/SDL.h>

/* ── Virtual buttons ─────────────────────────────────────────── */
typedef enum {
    BTN_NONE   = 0,
    BTN_UP     = 1 << 0,
    BTN_DOWN   = 1 << 1,
    BTN_LEFT   = 1 << 2,
    BTN_RIGHT  = 1 << 3,
    BTN_A      = 1 << 4,   /* Cross  */
    BTN_B      = 1 << 5,   /* Circle */
    BTN_X      = 1 << 6,   /* Triangle */
    BTN_Y      = 1 << 7,   /* Square */
    BTN_L      = 1 << 8,   /* L */
    BTN_R      = 1 << 9,   /* R */
    BTN_START  = 1 << 10,
    BTN_SELECT = 1 << 11,
} Button;

typedef struct {
    uint32_t held;      /* buttons currently held              */
    uint32_t pressed;   /* buttons that went down this frame   */
    uint32_t released;  /* buttons that went up   this frame   */
    int      quit;
} InputState;

/* Call once at startup (sets PSP ctrl sampling mode on hardware) */
void input_init   (InputState *inp);
/* Call once per frame before reading state */
void input_update (InputState *inp);

static inline int btn_held    (const InputState *i, Button b) { return (i->held     & b) != 0; }
static inline int btn_pressed (const InputState *i, Button b) { return (i->pressed  & b) != 0; }
static inline int btn_released(const InputState *i, Button b) { return (i->released & b) != 0; }