#pragma once
#include "common.h"
#include "level.h"

#define HERO_W 12
#define HERO_H 15
//#define HERO_SPACING 1
#define TILESET_HERO_COLS    21
#define TILESET_HERO_ROWS     7
#define TILESET_HERO_W       12
#define TILESET_HERO_H       15

#define HERO_ARMOR_TIER 6

/* ── Hero class ───────────────────────────────────────
   Order MUST match ASSET_HERO_BY_CLASS in assets.h — the enum value
   is used directly as an index into that array and into
   RenderCtx.heroes[]. */
typedef enum {
    HERO_WARRIOR = 0,
    HERO_MAGE,
    HERO_ROGUE,
    HERO_HUNTRESS,
    HERO_CLASS_COUNT
} HeroClass;
 
/* Which way the sprite is drawn. The art is drawn facing right;
   FACE_LEFT is achieved with a horizontal flip at draw time, not a
   separate set of frames. */
typedef enum {
    FACE_RIGHT = 0,
    FACE_LEFT  = 1
} Facing;
 
/* ── Animation state ──────────────────────────────────
   Every class (and every armor-tier row on the sheet) shares the same
   21-column layout:
     0-2   idle
     3-8   run    (6-frame cycle)
     9-13  die    (frame 13 doubles as the "fallen" resting pose)
     14    operate (search/interact)
     15-16 read   (2-frame alternate)
     17-20 attack
   See HERO_CLIPS below for the exact per-animation playback order. */
typedef enum {
    ANIM_IDLE = 0,
    ANIM_RUN,
    ANIM_ATTACK,
    ANIM_DIE,
    ANIM_OPERATE,
    ANIM_READ,
    ANIM_COUNT
} AnimState;
 
typedef struct {
    const uint8_t *frames; /* column indices into the sheet, in play order */
    int count;             /* number of entries in frames[]                */
    int fps;                /* playback speed, frames per second            */
    int loop;               /* 1 = repeat, 0 = play once and hold last frame */
} AnimClip;
 
/* Defined once in player.c; render.c reads it to pick the source rect. */
extern const AnimClip HERO_CLIPS[ANIM_COUNT];

typedef struct {
    int x, y;              /* current tile position                     */
    HeroClass cls;         /* which sprite sheet to draw                */
    Facing    facing;      /* left/right — drives the horizontal flip   */
    AnimState anim;        /* animation currently playing               */
    int       frame;       /* index into HERO_CLIPS[anim].frames        */
    float     frame_clock; /* seconds accumulated toward the next frame */
    int       anim_done;   /* 1 once a non-looping clip has played out  */
} Player;
 
/* Place player at level entrance */
void player_init(Player *p, const Level *lvl, HeroClass cls);
 
/* Try to move by (dx,dy). Opens doors on contact.
   Returns 1 if the move succeeded, 0 if blocked. */
int  player_move(Player *p, Level *lvl, int dx, int dy);


/* Advance the current animation by dt seconds. Call once per frame,
   regardless of whether the player moved that frame. */
void player_animate(Player *p, float dt);
 
/* Immediately switch to and (re)start a given animation, e.g. from
   combat code: player_play_anim(&p, ANIM_ATTACK); */
void player_play_anim(Player *p, AnimState anim);

void player_step_anim(Player *p);