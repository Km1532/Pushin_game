#ifndef WORLD_UI_H
#define WORLD_UI_H

#include "types.h"

void openLvlSubmenu(struct Adventurer *hero);
void openShopMenu(struct Adventurer *hero);
void openDialogue(struct Adventurer *hero);
int saveGameToFile(const struct Adventurer *hero, const char *filename);
int loadGameFromFile(struct Adventurer *hero, const char *filename);

#endif