#include "mob_bestiary.h"
#include "rng.h"

/* ══════════════════════════════════════════════════════════
   Random.chances(float[])
   ══════════════════════════════════════════════════════════ */
static int weighted_pick(const float *weights, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) sum += weights[i];

    float roll = rng_float() * sum;
    float running = 0.0f;
    for (int i = 0; i < n; i++) {
        running += weights[i];
        if (roll < running) return i;
    }
    return n - 1;
}

/* ══════════════════════════════════════════════════════════
   Bestiary.mobClass(depth)
   ══════════════════════════════════════════════════════════ */
MobKind bestiary_pick(int depth) {
    switch (depth + 1) {
    case 1: {
        static const MobKind k[] = { MOB_RAT };
        static const float   w[] = { 1 };
        return k[weighted_pick(w, 1)];
    }
    case 2: {
        static const MobKind k[] = { MOB_RAT, MOB_GNOLL };
        static const float   w[] = { 1, 1 };
        return k[weighted_pick(w, 2)];
    }
    case 3: {
        static const MobKind k[] = { MOB_RAT, MOB_GNOLL, MOB_CRAB, MOB_SWARM };
        static const float   w[] = { 1, 2, 1, 0.02f };
        return k[weighted_pick(w, 4)];
    }
    case 4: {
        static const MobKind k[] = { MOB_RAT, MOB_GNOLL, MOB_CRAB, MOB_SWARM, MOB_SKELETON, MOB_THIEF };
        static const float   w[] = { 1, 2, 3, 0.02f, 0.01f, 0.01f };
        return k[weighted_pick(w, 6)];
    }
    case 5:
        return MOB_GOO; /* boss */

    case 6: {
        static const MobKind k[] = { MOB_SKELETON, MOB_THIEF, MOB_SWARM, MOB_SHAMAN };
        static const float   w[] = { 4, 2, 1, 0.2f };
        return k[weighted_pick(w, 4)];
    }
    case 7: {
        static const MobKind k[] = { MOB_SKELETON, MOB_SHAMAN, MOB_THIEF, MOB_SWARM };
        static const float   w[] = { 3, 1, 1, 1 };
        return k[weighted_pick(w, 4)];
    }
    case 8: {
        static const MobKind k[] = { MOB_SKELETON, MOB_SHAMAN, MOB_GNOLL, MOB_THIEF, MOB_SWARM, MOB_BAT };
        static const float   w[] = { 3, 2, 1, 1, 1, 0.02f };
        return k[weighted_pick(w, 6)];
    }
    case 9: {
        static const MobKind k[] = { MOB_SKELETON, MOB_SHAMAN, MOB_THIEF, MOB_SWARM, MOB_BAT, MOB_BRUTE };
        static const float   w[] = { 3, 3, 1, 1, 0.02f, 0.01f };
        return k[weighted_pick(w, 6)];
    }
    case 10:
        return MOB_TENGU; /* boss */

    case 11: {
        static const MobKind k[] = { MOB_BAT, MOB_BRUTE };
        static const float   w[] = { 1, 0.2f };
        return k[weighted_pick(w, 2)];
    }
    case 12: {
        static const MobKind k[] = { MOB_BAT, MOB_BRUTE, MOB_SPINNER };
        static const float   w[] = { 1, 1, 0.2f };
        return k[weighted_pick(w, 3)];
    }
    case 13: {
        static const MobKind k[] = { MOB_BAT, MOB_BRUTE, MOB_SHAMAN, MOB_SPINNER, MOB_ELEMENTAL };
        static const float   w[] = { 1, 3, 1, 1, 0.02f };
        return k[weighted_pick(w, 5)];
    }
    case 14: {
        static const MobKind k[] = { MOB_BAT, MOB_BRUTE, MOB_SHAMAN, MOB_SPINNER, MOB_ELEMENTAL, MOB_MONK };
        static const float   w[] = { 1, 3, 1, 4, 0.02f, 0.01f };
        return k[weighted_pick(w, 6)];
    }
    case 15:
        return MOB_DM300; /* boss */

    case 16: {
        static const MobKind k[] = { MOB_ELEMENTAL, MOB_WARLOCK, MOB_MONK };
        static const float   w[] = { 1, 1, 0.2f };
        return k[weighted_pick(w, 3)];
    }
    case 17: {
        static const MobKind k[] = { MOB_ELEMENTAL, MOB_MONK, MOB_WARLOCK };
        static const float   w[] = { 1, 1, 1 };
        return k[weighted_pick(w, 3)];
    }
    case 18: {
        static const MobKind k[] = { MOB_ELEMENTAL, MOB_MONK, MOB_GOLEM, MOB_WARLOCK };
        static const float   w[] = { 1, 2, 1, 1 };
        return k[weighted_pick(w, 4)];
    }
    case 19: {
        static const MobKind k[] = { MOB_ELEMENTAL, MOB_MONK, MOB_GOLEM, MOB_WARLOCK, MOB_SUCCUBUS };
        static const float   w[] = { 1, 2, 3, 1, 0.02f };
        return k[weighted_pick(w, 5)];
    }
    case 20:
        return MOB_KING; /* boss */

    /* case 21 LastShopLevel */

    case 22: {
        static const MobKind k[] = { MOB_SUCCUBUS, MOB_EYE };
        static const float   w[] = { 1, 1 };
        return k[weighted_pick(w, 2)];
    }
    case 23: {
        static const MobKind k[] = { MOB_SUCCUBUS, MOB_EYE, MOB_SCORPIO };
        static const float   w[] = { 1, 2, 1 };
        return k[weighted_pick(w, 3)];
    }
    case 24: {
        static const MobKind k[] = { MOB_SUCCUBUS, MOB_EYE, MOB_SCORPIO };
        static const float   w[] = { 1, 2, 3 };
        return k[weighted_pick(w, 3)];
    }
    case 25:
        return MOB_YOG; /* boss */

    default:
        return MOB_EYE;
    }
}

MobKind bestiary_pick_mutable(int depth) {
    MobKind kind = bestiary_pick(depth);
    if (rng_int(30) == 0) {
        switch (kind) {
        case MOB_RAT:     return MOB_ALBINO;
        case MOB_THIEF:   return MOB_BANDIT;
        case MOB_BRUTE:   return MOB_SHIELDED;
        case MOB_MONK:    return MOB_SENIOR;
        case MOB_SCORPIO: return MOB_ACIDIC;
        default:          return kind;
        }
    }
    return kind;
}

int bestiary_mob_count(int depth) {
    return 2 + ((depth + 1) % 5) + rng_int(3);
}