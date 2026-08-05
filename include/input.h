#pragma once
#include <SDL2/SDL.h>

/* ── Virtual buttons ─────────────────────────────────────────── */
typedef enum {
    BTN_NONE     = 0,
    BTN_UP       = 1 << 0,
    BTN_DOWN     = 1 << 1,
    BTN_LEFT     = 1 << 2,
    BTN_RIGHT    = 1 << 3,
    BTN_CROSS    = 1 << 4,
    BTN_CIRCLE   = 1 << 5,
    BTN_TRIANGLE = 1 << 6,
    BTN_SQUARE   = 1 << 7,
    BTN_L        = 1 << 8,
    BTN_R        = 1 << 9,
    BTN_START    = 1 << 10,
    BTN_SELECT   = 1 << 11,
} Button;

typedef struct {
    uint32_t held;      /* buttons currently held              */
    uint32_t pressed;   /* buttons that went down this frame   */
    uint32_t released;  /* buttons that went up   this frame   */
    int      quit;
} InputState;

void input_init   (InputState *inp);
void input_update (InputState *inp);

static inline int btn_held    (const InputState *i, Button b) { return (i->held     & b) != 0; }
static inline int btn_pressed (const InputState *i, Button b) { return (i->pressed  & b) != 0; }
static inline int btn_released(const InputState *i, Button b) { return (i->released & b) != 0; }

void input_get_move(InputState *inp, int *dx, int *dy);