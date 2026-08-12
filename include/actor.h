#pragma once
#include <stdint.h>

/* ══════════════════════════════════════════════════════════
   port of Actor.java
   ══════════════════════════════════════════════════════════ */

#define MAX_ACTORS 128
#define ACTOR_TICK 1.0f

typedef struct Actor Actor;

typedef struct {
    int  (*act)(Actor *self);
    void (*on_add)(Actor *self);
    void (*on_remove)(Actor *self);
    int  (*is_blocked)(Actor *self);
} ActorVTable;

struct Actor {
    const ActorVTable *vt;
    float time;
    int   id;
};

/* ── Actor methods ──────────────────────────────────────────── */
void actor_init(Actor *a, const ActorVTable *vt);
int actor_id(Actor *a);

void  actor_spend(Actor *a, float time);
void  actor_postpone(Actor *a, float time);
float actor_cooldown(const Actor *a);
void  actor_deactivate(Actor *a); /* original: diactivate()*/

void actor_next(Actor *a);

/* ── Scheduler ──────────────────────────────────────────── */
int actor_add(Actor *a);
int actor_add_delayed(Actor *a, float delay);

void actor_remove(Actor *a);

void actor_unregister(Actor *a);

void actor_process(void);

void actor_set_stop_condition(int (*should_stop)(void));

/* ── Global state ─────────────────────────────────────────── */
void actor_clear(void);
void actor_fix_time(void);

Actor *actor_find_by_id(int id);

int    actor_count(void);
Actor *actor_at_index(int i); /* for iterating every registered Actor, e.g. from a future save system — mirrors what a caller would get from Actor.all() */