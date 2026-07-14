#pragma once
#include "level.h"

/* ══════════════════════════════════════════════════════════
   BOSS LEVEL GENERATION
   Ports of CavesBossLevel.java, CityBossLevel.java,
   HallsBossLevel.java, PrisonBossLevel.java, SewerBossLevel.java
   (build() + decorate()). By analogy to level_generate():
   same RNG-seeding/return-value contract (0 = ok, -1 = failed).

   Scope note: only *generation* is ported here. Each original
   XBossLevel also overrides press()/drop() to seal the arena
   door when the hero walks in and re-open it once the boss's
   key is dropped, plus createItems()'s Bones.get() call. None
   of that is wired up - there's no mob/combat/Bones system yet
   for it to hook into. See the comment block at the top of
   level_boss.c for the specifics of what's deferred.
   ══════════════════════════════════════════════════════════ */

/* True for a theme's boss floor. Every 5th floor, 0-indexed:
   depths 4, 9, 14, 19, 24 (i.e. (depth+1)%5==0). This mirrors
   dungeon.c's get_theme() thresholds (5/10/15/20) and matches
   level.c's shop_on_level(), which already puts shops on 5, 10,
   15, 20 - the floor right after each boss. If your intended
   floor count per theme ever changes, this is the one place to
   update. */
int level_is_boss_depth(int depth);

/* Generates a boss level for `theme` into *lvl. Dispatches to the
   matching per-theme builder below. Called from level_generate()
   when level_is_boss_depth(depth) is true, so dungeon.c needs no
   changes - it already just calls level_generate(). */
int level_generate_boss(Level *lvl, uint32_t seed, DungeonTheme theme, int depth);

/* ── Caves arena box — CavesBossLevel.ROOM_LEFT/RIGHT/TOP/BOTTOM ──
   Shared with level_decorate.c (decorate_caves_boss's sign placement
   uses the same box), so it lives here rather than being duplicated. */
#define CAVES_ARENA_LEFT   (LEVEL_W / 2 - 2)
#define CAVES_ARENA_RIGHT  (LEVEL_W / 2 + 2)
#define CAVES_ARENA_TOP    (LEVEL_H / 2 - 2)
#define CAVES_ARENA_BOTTOM (LEVEL_H / 2 + 2)