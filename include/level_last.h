#pragma once
#include "level.h"

/* ══════════════════════════════════════════════════════════
   EPILOGUE LEVELS
   Ports of LastShopLevel.java (the shop level after the Halls
   boss, before the Amulet chamber) and LastLevel.java (the
   Amulet chamber itself).

   Neither is a "boss level" - no fight happens on either, so
   these are deliberately separate from level_boss.c rather than
   folded into it.

   Deliberately NOT ported (no source for it, or nothing to hook
   it into yet):
     - createItems()'s Bones.get() call on LastShopLevel, same as
       every other level ported so far - no "previous character's
       remains" system exists yet.
     - The Imp NPC and its quest (Imp.Quest.isCompleted() in the
       original) - there's no NPC or quest-tracking system in this
       port. `quest_completed` below is a plain parameter standing
       in for that check; pass 0 until you have something to check,
       or wire it up to whatever you build.
     - The shopkeeper NPC that a real SHOP room would normally get:
       paint_shop() already calls place_shopkeeper(), which is a
       stub (see level_paint.c) - so a completed-quest shop here
       will have real items to buy, just no shopkeeper standing in
       it, same as every other shop in this port right now.
   ══════════════════════════════════════════════════════════ */

/* LastShopLevel.java. `quest_completed` mirrors Imp.Quest.isCompleted()
   - true assigns the level's largest leftover room as a real SHOP
   (stocked via the existing paint_shop()/place_shop_items()) and
   places a sign in the entrance room; false leaves that room as a
   plain STANDARD room and skips the sign. Same 0/-1 contract as
   level_generate()/level_generate_boss(). */
int level_generate_last_shop(Level *lvl, uint32_t seed, int depth, int quest_completed);

/* LastLevel.java - small (7x7), fixed layout, always succeeds.
   Drops the Amulet of Yendor on the central pedestal.

   Not wired into level_generate()'s depth dispatch the way the
   boss levels and LastShopLevel are: LastShopLevel fits cleanly at
   depth 25, the one floor MAX_FLOORS=26 has spare past the Halls
   boss at 24, but LastLevel has no floor left to sit at without
   either raising MAX_FLOORS to 27 or triggering it some other way
   (e.g. off the back of LastShopLevel's exit, outside the normal
   floors[] array, since you'd never need to come back up from it).
   That's a real call for you to make rather than one I should guess
   at, so this is exposed as a plain function - call it from wherever
   you decide the ending should be triggered. */
int level_generate_last(Level *lvl, uint32_t seed, int depth);