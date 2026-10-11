#pragma once
#include "render.h"
#include "input.h"
#include "dungeon.h"
#include "ui.h"

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_TARGETING,
    GAME_STATE_INVENTORY,
    GAME_STATE_POPUP
} GameState;

typedef enum {
    POPUP_NONE,
    POPUP_ITEM_DESC,
    POPUP_STATS
} PopupType;

typedef struct {
    uint32_t next_ms;
    int held;
    int blocked;
} MoveRepeat;

typedef enum {
    TARGET_ACTION_QUICK_SLOT,
    TARGET_ACTION_INVENTORY
} TargetAction;

typedef struct {
    Dungeon dungeon;
    Player player;
    Camera cam;
    Fog *fog;
    Level *lvl;

    InventoryUI inv;

    MoveRepeat move_repeat;
    MoveRepeat ui_move_repeat;

    uint32_t rest_next_ms; //TEST
    int rest_pending;

    GameState state;
    PopupType popup;
    GameState popup_return_state;

    TargetAction target_action;
    int throw_bag;
    int throw_slot;
} Game;

void game_init(Game *game);
void game_input(Game *game, InputState *inp, uint32_t now);
void game_update(Game *game, RenderCtx *rctx, float dt);
void game_render(Game *game, RenderCtx *rctx);