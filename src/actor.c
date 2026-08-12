#include "actor.h"
#include <string.h>
#include <float.h>

/* ══════════════════════════════════════════════════════════
   Static state 
   ══════════════════════════════════════════════════════════ */
static Actor *all[MAX_ACTORS];
static int all_count = 0;

static Actor *ids[MAX_ACTORS + 1];

static Actor *current = NULL;

static float now = 0.0f;

static int (*stop_condition)(void);
/* ══════════════════════════════════════════════════════════
   Actor methods   
   ══════════════════════════════════════════════════════════ */
void actor_init(Actor *a, const ActorVTable *vt) {
    a->vt   = vt;
    a->time = 0.0f;
    a->id   = 0;
}

int actor_id(Actor *a) {
    if (a->id > 0) return a->id;

    int max = 0;
    for (int i = 0; i < all_count; i++) {
        if (all[i]->id > max) max = all[i]->id;
    }
    a->id = max + 1;
    return a->id;
}

void actor_spend(Actor *a, float time) {
    a->time += time;
}

void actor_postpone(Actor *a, float time) {
    if (a->time < now + time) {
        a->time = now + time;
    }
}

float actor_cooldown(const Actor *a) {//DO I REALLY NEED COOLDOWN? (BREAK MOBS AFTER CHANGING LEVELS)
    return a->time - now;
}

void actor_deactivate(Actor *a) {
    a->time = FLT_MAX;
}

void actor_next(Actor *a) {
    if (current == a) current = NULL;
}

/* ══════════════════════════════════════════════════════════
   Internal helpers 
   ══════════════════════════════════════════════════════════ */
static int actor_add_at(Actor *a, float time) {
    for (int i = 0; i < all_count; i++) {
        if (all[i] == a) return 1;
    }
    if (all_count >= MAX_ACTORS) return 0;

    a->time += time;

    if (a->id > 0 && a->id <= MAX_ACTORS) {
        ids[a->id] = a;
    }

    all[all_count++] = a;

    if (a->vt->on_add) a->vt->on_add(a);

    return 1;
}

/* ══════════════════════════════════════════════════════════
   Static API
   ══════════════════════════════════════════════════════════ */
void actor_clear(void) {
    now = 0.0f;
    all_count = 0;
    memset(ids, 0, sizeof(ids));
    current = NULL;
}

void actor_fix_time(void) {
    float min = FLT_MAX;
    for (int i = 0; i < all_count; i++) {
        if (all[i]->time < min) min = all[i]->time;
    }
    for (int i = 0; i < all_count; i++) {
        all[i]->time -= min;
    }
    now = 0.0f;
}

int actor_add(Actor *a) {
    return actor_add_at(a, now);
}

int actor_add_delayed(Actor *a, float delay) {
    return actor_add_at(a, now + delay);
}

static void actor_detach(Actor *a, int notify) {
    if (current == a)
        current = NULL;

    if (!a) return;

    for (int i = 0; i < all_count; i++) {
        if (all[i] == a) {
            all[i] = all[all_count - 1];
            all_count--;
            break;
        }
    }

    if (notify && a->vt->on_remove) a->vt->on_remove(a);

    if (a->id > 0 && a->id <= MAX_ACTORS) {
        ids[a->id] = NULL;
    }
}

void actor_remove(Actor *a) {
    actor_detach(a, 1);
}

void actor_unregister(Actor *a) {
    actor_detach(a, 0);
}

Actor *actor_find_by_id(int id) {
    if (id <= 0 || id > MAX_ACTORS) return NULL;
    return ids[id];
}

void actor_process(void) {
    if (current != NULL) return;

    int do_next;
    do {
        now = FLT_MAX;
        current = NULL;

        for (int i = 0; i < all_count; i++) {
            Actor *a = all[i];
            if (a->time < now) {
                now = a->time;
                current = a;
            }
        }

        if (current != NULL) {
            if (current->vt->is_blocked && current->vt->is_blocked(current)) {
                current = NULL;
                return;
            }

            do_next = current->vt->act(current);

            if (do_next && stop_condition && stop_condition()) {
                do_next = 0;
                current = NULL;
            }
        } else {
            do_next = 0;
        }

    } while (do_next);
}

void actor_set_stop_condition(int (*should_stop)(void)) {
    stop_condition = should_stop;
}

/* ══════════════════════════════════════════════════════════
   Debug / iteration
   ══════════════════════════════════════════════════════════ */
int actor_count(void) {
    return all_count;
}

Actor *actor_at_index(int i) {
    if (i < 0 || i >= all_count) return NULL;
    return all[i];
}