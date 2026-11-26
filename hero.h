#ifndef HERO_H
#define HERO_H

#include "types.h"

struct Adventurer* createAdventurer(const char *nameStr);
int addItemToInventory(struct Adventurer *hero,
                       const char *itemName,
                       const char *itemType,
                       const char *itemDesc,
                       int price);
int removeItemFromInventory(struct Adventurer *hero, int index);
void clearInventory(struct Adventurer *hero);
void freeAdventurer(struct Adventurer *hero);
void gainHeroXp(struct Adventurer *hero, int amount);
void updateQuestsProgress(struct Adventurer *hero);
void showQuestsMenu(const struct Adventurer *hero);

#endif