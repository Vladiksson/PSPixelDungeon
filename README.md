Recreation of Pixel Dungeon for PSP-1000 (of course, it requires custom firmware) in C using the PSPDEV SDK. (Check out the original one: https://github.com/watabou/pixel-dungeon)

Since I've seen a somewhat similar project for PS Vita, I'm making mine public too. It's not finished yet, and there is a lot of broken or sloppy stuff that I'm going to rewrite over time. I've been working on it as a hobby since July 2026, in my free time, and I don't have any big ambitions for it - I'm just having fun coding this bullshit.

In its current state, you can fully go through all dungeon floors and get the Amulet of Yendor at the end.

It will be easier to list the major stuff I haven't done yet:

1. There is no save system at all.
2. There is no sound at all.
3. There is no interaction with NPCs yet (quests, shops, etc.).
4. Right now, mobs and bosses are really primitive (they only use melee attacks).
5. Not all particles have been ported yet.
6. Not all items are functional yet (scrolls, food, rings... for example).
7. Items don't have enchantments and spawn in a stupidly random way.
8. There is no interaction with traps or plants yet.
... Maybe I forgot to mention something important, whatever...

One more thing to know - right now, ALL game resources are loaded when the game starts, so on real hardware it takes around 30 seconds. (IT'S TEMPORARY, I'LL REDO IT WHEN I'M IN THE MOOD FOR IT)

If, for some reason, you want to compile it yourself, you will need the PSPDEV SDK. (You might want to check out this guide: https://pspdev.github.io/)

This creation is distributed under the GNU General Public License v3.0. (The game's assets are under this license too.)

Controls:

On the start screen, just tap the Start button to begin (I know, very logical).

Then use Square, Cross, Circle, or Triangle to choose a hero, in the order shown on screen (It's temporary, I'll redo it after adding the save system).

Movement is controlled with the D-pad everywhere.

Cross - attack a KNOWN target.
Square - quick slot.
Circle - search.
Triangle - wait when tapped / rest after holding.

L - map zoom.
R - targeting.

SELECT - inventory.
START - info.

In inventory:
Cross - equip/unequip, set quick slot.
Square - consume.
Circle - enter targeting mode for throwing/zapping.
Triangle - drop.
L, R - switch between inventory pages.
START - item description.

In targeting mode:
R - exit targeting mode.
Circle - exit targeting mode (return to inventory).
Square - zap with wand.
Cross - throw.

Special thanks to my svåger for giving me his old PSP a year ago. Without it, this project wouldn't exist!