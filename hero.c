#include "hero.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Adventurer* createAdventurer(const char *nameStr) {
    struct Adventurer *hero = malloc(sizeof(struct Adventurer));
    if (hero == NULL) {
        fprintf(stderr, "Error: failed to allocate memory for Adventurer.\n");
        return NULL;
    }

    hero->name = malloc(strlen(nameStr) + 1);
    if (hero->name == NULL) {
        fprintf(stderr, "Error: failed to allocate memory for hero name.\n");
        free(hero);
        return NULL;
    }
    strcpy(hero->name, nameStr);

    hero->hpMax = 100;
    hero->hpCurrent = hero->hpMax;
    hero->strength = 28;
    hero->defense = 6;
    hero->coins = 200;
    hero->score = 0;
    hero->xp = 0;
    hero->heroLevel = 1;
    hero->pusheenLevel = 1;
    hero->x = 1;
    hero->y = 1;

    hero->defeatedEnemiesCount = 0;
    hero->heartsCollectedCount = 0;
    hero->quest1Done = 0;
    hero->quest2Done = 0;
    hero->quest3Done = 0;

    hero->invCapacity = 8;
    hero->invCount = 0;
    hero->inventory = calloc((size_t)hero->invCapacity, sizeof(struct Item));
    if (hero->inventory == NULL) {
        fprintf(stderr, "Error: failed to allocate memory for inventory.\n");
        free(hero->name);
        free(hero);
        return NULL;
    }

    return hero;
}

int addItemToInventory(struct Adventurer *hero,
                       const char *itemName,
                       const char *itemType,
                       const char *itemDesc,
                       int price) {
    if (hero == NULL) return 0;

    if (hero->invCount == hero->invCapacity) {
        int newCap = hero->invCapacity * 2;
        struct Item *tmp = realloc(hero->inventory, (size_t)newCap * sizeof(struct Item));
        if (tmp == NULL) {
            fprintf(stderr, "Warning: failed to expand inventory.\n");
            return 0;
        }
        hero->inventory = tmp;
        hero->invCapacity = newCap;
    }

    char *nCopy = malloc(strlen(itemName) + 1);
    char *tCopy = malloc(strlen(itemType) + 1);
    char *dCopy = malloc(strlen(itemDesc) + 1);
    if (nCopy == NULL || tCopy == NULL || dCopy == NULL) {
        fprintf(stderr, "Warning: out of memory for item '%s'.\n", itemName);
        free(nCopy);
        free(tCopy);
        free(dCopy);
        return 0;
    }

    strcpy(nCopy, itemName);
    strcpy(tCopy, itemType);
    strcpy(dCopy, itemDesc);

    hero->inventory[hero->invCount].name = nCopy;
    hero->inventory[hero->invCount].type = tCopy;
    hero->inventory[hero->invCount].description = dCopy;
    hero->inventory[hero->invCount].price = price;
    hero->invCount++;
    return 1;
}

int removeItemFromInventory(struct Adventurer *hero, int index) {
    if (hero == NULL || index < 0 || index >= hero->invCount) {
        return 0;
    }
    free(hero->inventory[index].name);
    free(hero->inventory[index].type);
    free(hero->inventory[index].description);

    for (int i = index; i < hero->invCount - 1; i++) {
        hero->inventory[i] = hero->inventory[i + 1];
    }
    hero->invCount--;
    return 1;
}

void clearInventory(struct Adventurer *hero) {
    if (hero == NULL) return;
    for (int i = 0; i < hero->invCount; i++) {
        free(hero->inventory[i].name);
        free(hero->inventory[i].type);
        free(hero->inventory[i].description);
    }
    hero->invCount = 0;
}

void freeAdventurer(struct Adventurer *hero) {
    if (hero == NULL) return;
    clearInventory(hero);
    free(hero->inventory);
    free(hero->name);
    free(hero);
}

static int xpThresholdForLevel(int level) {
    return level * 100;
}

static void checkAndApplyHeroLevelUp(struct Adventurer *hero) {
    while (hero->heroLevel < MAX_HERO_LEVEL && hero->xp >= xpThresholdForLevel(hero->heroLevel)) {
        hero->xp -= xpThresholdForLevel(hero->heroLevel);
        hero->heroLevel++;
        hero->hpMax += 20;
        hero->hpCurrent = hero->hpMax;
        hero->strength += 5;
        hero->defense += 2;
        printf("\n*** HERO LEVEL UP: LVL %d/%d! ***\n", hero->heroLevel, MAX_HERO_LEVEL);
        printf("    Max HP: +20 | Strength: +5 | Defense: +2\n");
    }
}

void gainHeroXp(struct Adventurer *hero, int amount) {
    if (amount <= 0) return;
    hero->xp += amount;
    printf("    (+%d XP)\n", amount);
    checkAndApplyHeroLevelUp(hero);
}

void updateQuestsProgress(struct Adventurer *hero) {
    if (!hero->quest1Done && hero->defeatedEnemiesCount >= 3) {
        hero->quest1Done = 1;
        hero->score += 150;
        hero->coins += 30;
        fputs("\n[QUEST #1 COMPLETED] Defeat 3 enemies! +150 score, +30 coins.\n", stdout);
    }
    if (!hero->quest2Done && hero->heartsCollectedCount >= 1) {
        hero->quest2Done = 1;
        hero->score += 100;
        hero->coins += 20;
        fputs("\n[QUEST #2 COMPLETED] Collect a heart [H] on the map! +100 score, +20 coins.\n", stdout);
    }
    if (!hero->quest3Done && hero->score >= 500) {
        hero->quest3Done = 1;
        hero->score += 200;
        hero->coins += 40;
        fputs("\n[QUEST #3 COMPLETED] Reach 500+ score! +200 score, +40 coins.\n", stdout);
    }
}

void showQuestsMenu(const struct Adventurer *hero) {
    fputs("\n=== QUESTS ===\n", stdout);
    printf("1. [%s] Defeat 3 enemies (Progress: %d/3) - [Combat]\n",
           hero->quest1Done ? "DONE" : "IN PROGRESS", hero->defeatedEnemiesCount);
    printf("2. [%s] Collect a heart [H] on the map (Progress: %d/1) - [Map]\n",
           hero->quest2Done ? "DONE" : "IN PROGRESS", hero->heartsCollectedCount);
    printf("3. [%s] Reach 500 score (Current: %d/500) - [Score]\n",
           hero->quest3Done ? "DONE" : "IN PROGRESS", hero->score);
}