#include "player.h"
 
/* ── hero animation frame tables ─────────────────────────
   Column indices refer to the shared 21-column layout every class
   (and every armor-tier row) follows on the hero sheets — see the
   comment above AnimState in player.h. */
static const uint8_t F_IDLE[]    = { 0,0,0,1,0,0,1,2 };
static const uint8_t F_RUN[] = { 3,4,5};  //ORIGINAL WAS static const uint8_t F_RUN[]     = { 3,4,5,6,7,8 };
static const uint8_t F_ATTACK[]  = { 17,18,19,20 };
static const uint8_t F_DIE[]     = { 9,10,11,12,13 };
static const uint8_t F_OPERATE[] = { 14 };
static const uint8_t F_READ[]    = { 15,16 };


/* fps/loop values are sensible defaults for this frame count, not
   pulled from any spec — tune freely to taste. */
const AnimClip HERO_CLIPS[ANIM_COUNT] = {
    { F_IDLE,    8, 2,  1 },  /* ANIM_IDLE    - slow idle/blink loop      */
    { F_RUN,     6, 12, 1 },  /* ANIM_RUN      */
    { F_ATTACK,  4, 12, 0 },  /* ANIM_ATTACK  - one swing, then back to idle */
    { F_DIE,     5, 8,  0 },  /* ANIM_DIE     - plays once and holds      */
    { F_OPERATE, 1, 1,  0 },  /* ANIM_OPERATE - single frame, brief hold  */
    { F_READ,    2, 4,  1 },  /* ANIM_READ    - 2-frame flicker loop      */
};
 


void player_init(Player *p, const Level *lvl, HeroClass cls){
    Point ep = idx_to_xy(lvl->entrance);
    p->x           = ep.x;
    p->y           = ep.y;
    p->cls         = cls;
    p->facing      = FACE_RIGHT;
    p->anim        = ANIM_IDLE;
    p->frame       = 0;
    p->frame_clock = 0.0f;
    p->anim_done   = 0;

}
 
void player_play_anim(Player *p, AnimState anim){
    if (p->anim == anim)
        return;

    p->anim = anim;
    p->frame = 0;
    p->frame_clock = 0.0f;
    p->anim_done = 0;
}

int player_move(Player *p, Level *lvl, int dx, int dy) {
    int nx = p->x + dx;
    int ny = p->y + dy;
    if (!in_bounds(nx, ny)) return 0;
 
    uint8_t tile = lvl->map[xy_to_idx(nx, ny)];
    if (!(gTileFlags[tile] & TF_PASSABLE)) return 0;
 
    /* Open closed doors on contact (PD behaviour) */
    if (tile == TILE_DOOR || tile == TILE_LOCKED_DOOR)
        lvl->map[xy_to_idx(nx, ny)] = TILE_OPEN_DOOR;
    
    /* Only flip on horizontal movement — keep prior facing on vertical-only
    steps, same convention as the original game. */
    if (dx != 0) p->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;

    p->x = nx;
    p->y = ny;
    return 1;
}

void player_animate(Player *p, float dt) {
    if (p->anim == ANIM_RUN)return;

    const AnimClip *clip = &HERO_CLIPS[p->anim];
    if (p->anim_done || clip->fps <= 0) return;
 
    p->frame_clock += dt;
    float step = 1.0f / (float)clip->fps;
 
    while (p->frame_clock >= step) {
        p->frame_clock -= step;
        p->frame++;
        if (p->frame >= clip->count) {
            if (clip->loop) {
                p->frame = 0;
            } else {
                p->frame = clip->count - 1; /* hold on the last frame */
                p->anim_done = 1;
                break;
            }
        }
    }
 
    if (p->anim_done) {
        switch (p->anim) {
            case ANIM_RUN:
            case ANIM_ATTACK:
            case ANIM_OPERATE:
                player_play_anim(p, ANIM_IDLE);
                break;
            default:
                break; /* ANIM_DIE holds its final frame on purpose */
        }
    }
}

void player_step_anim(Player *p){
    const AnimClip *clip = &HERO_CLIPS[ANIM_RUN];

    if (p->anim != ANIM_RUN) {
        p->anim = ANIM_RUN;
        p->frame = 0;
        p->frame_clock = 0.0f;
        p->anim_done = 0;
        return;
    }

    p->frame = (p->frame + 1) % clip->count;
}