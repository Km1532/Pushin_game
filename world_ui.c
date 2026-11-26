#include "world_ui.h"
#include "hero.h"
#include "io_utils.h"
#include "map_combat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct ShopCatalogEntry g_Catalog[SHOP_ITEMS_COUNT] = {
    {"Golden Slipper", "Weapon", "Legendary close-combat slipper", 40},
    {"Cat Dagger", "Weapon", "Sharp dagger for fast strikes", 55},
    {"Sausage Sword", "Weapon", "Heavy and tasty knight sword", 70},
    {"Silver Claw", "Weapon", "Reinforced weapon against tough foes", 85},
    {"Royal Greatsword", "Weapon", "Grand blade of Pusheen's defender", 110},

    {"Milk Drink", "Potion", "Restores energy and alertness", 25},
    {"Catnip Brew", "Potion", "Boosts battle spirit", 35},
    {"Shield Potion", "Potion", "Hardens armor and resilience", 45},
    {"Strength Potion", "Potion", "Increases fighting power", 50},
    {"Heart Elixir", "Potion", "Rare drink from Pusheen's valley", 75},

    {"Princess Star", "Artifact", "Symbol of Pusheen's rescue", 60},
    {"Purr Medallion", "Artifact", "Ancient feline family talisman", 75},
    {"Yarn Ball", "Artifact", "Guides through dark paths", 50},
    {"Bell Charm", "Artifact", "Rings before dangerous fights", 65},
    {"Pusheen Blanket", "Artifact", "Warm relic from the princess home", 95}
};

void openLvlSubmenu(struct Adventurer *hero) {
    char line[INPUT_BUF_SIZE];
    int lvlChoice = -1;

    do {
        fputs("\n--- LEVEL SELECT ---\n", stdout);
        fputs("1. Level 1 (Start / Replay)\n", stdout);
        fputs("2. Level 2 (or Boss Lair at 5000+ score and Pusheen LVL 2)\n", stdout);
        fputs("3. DLC (Coming soon - in development)\n", stdout);
        fputs("0. Back to main menu\n", stdout);
        fputs("Your choice + Enter: ", stdout);

        if (!readLine(line, sizeof(line))) {
            fprintf(stderr, "\nInput read error from stdin.\n");
            return;
        }

        if (sscanf(line, "%d", &lvlChoice) != 1) {
            fprintf(stderr, "Invalid input '%s'. Enter a number (0-3).\n", line);
            continue;
        }

        switch (lvlChoice) {
            case 1:
                playRescueLevel(hero, 1);
                return;
            case 2:
                playRescueLevel(hero, 2);
                return;
            case 3:
                fputs("\n[DLC] Coming soon - in development!\n", stdout);
                break;
            case 0:
                fputs("Returning to main menu...\n", stdout);
                return;
            default:
                fprintf(stderr, "Unknown level choice: %d\n", lvlChoice);
                break;
        }
    } while (lvlChoice != 0);
}

void openShopMenu(struct Adventurer *hero) {
    char line[INPUT_BUF_SIZE];
    int choice = -1;

    do {
        fputs("\n=== SHOP ===\n", stdout);
        printf("Coins: %d | Pusheen LVL: %d/%d | Inventory: %d item(s)\n",
               hero->coins, hero->pusheenLevel, BOSS_UNLOCK_PUSHEEN_LVL, hero->invCount);
        fputs("1. Buy item (3 types, 5 items each)\n", stdout);
        fputs("2. Sell item from inventory\n", stdout);
        fputs("3. Upgrade Pusheen to Level 2 (60 coins)\n", stdout);
        fputs("0. Exit shop\n", stdout);
        fputs("Your choice + Enter: ", stdout);

        if (!readLine(line, sizeof(line)) || sscanf(line, "%d", &choice) != 1) {
            fprintf(stderr, "Invalid input in shop.\n");
            continue;
        }

        if (choice == 1) {
            fputs("\n--- ITEM CATALOG ---\n", stdout);
            for (int i = 0; i < SHOP_ITEMS_COUNT; i++) {
                printf("%2d. [%-8s] %-20s - %3d coins (%s)\n",
                       i + 1, g_Catalog[i].type, g_Catalog[i].name,
                       g_Catalog[i].price, g_Catalog[i].description);
            }
            fputs("Enter item number (1-15) to buy (0 - back) + Enter: ", stdout);
            int itemIdx = 0;
            if (readLine(line, sizeof(line)) && sscanf(line, "%d", &itemIdx) == 1) {
                if (itemIdx == 0) continue;
                itemIdx--;
                if (itemIdx >= 0 && itemIdx < SHOP_ITEMS_COUNT) {
                    if (hero->coins >= g_Catalog[itemIdx].price) {
                        hero->coins -= g_Catalog[itemIdx].price;
                        if (strcmp(g_Catalog[itemIdx].type, "Weapon") == 0) {
                            hero->strength += 3;
                        } else if (strcmp(g_Catalog[itemIdx].type, "Potion") == 0) {
                            hero->hpCurrent += 20;
                            if (hero->hpCurrent > hero->hpMax) hero->hpCurrent = hero->hpMax;
                        }
                        addItemToInventory(hero, g_Catalog[itemIdx].name, g_Catalog[itemIdx].type,
                                           g_Catalog[itemIdx].description, g_Catalog[itemIdx].price);
                        printf("Bought '%s'! Remaining coins: %d\n",
                               g_Catalog[itemIdx].name, hero->coins);
                    } else {
                        fprintf(stderr, "Not enough coins for '%s'!\n", g_Catalog[itemIdx].name);
                    }
                } else {
                    fprintf(stderr, "Invalid item number.\n");
                }
            }
        } else if (choice == 2) {
            if (hero->invCount == 0) {
                fputs("Inventory is empty - nothing to sell.\n", stdout);
                continue;
            }
            fputs("\n--- YOUR INVENTORY (SELL) ---\n", stdout);
            for (int i = 0; i < hero->invCount; i++) {
                int sellPrice = hero->inventory[i].price / 2;
                if (sellPrice < 5) sellPrice = 5;
                printf("%d. [%-8s] %-20s - sell price: %d coins\n",
                       i + 1, hero->inventory[i].type, hero->inventory[i].name, sellPrice);
            }
            fputs("Enter item number to sell (0 - back) + Enter: ", stdout);
            int sellIdx = 0;
            if (readLine(line, sizeof(line)) && sscanf(line, "%d", &sellIdx) == 1) {
                if (sellIdx == 0) continue;
                sellIdx--;
                if (sellIdx >= 0 && sellIdx < hero->invCount) {
                    int sellPrice = hero->inventory[sellIdx].price / 2;
                    if (sellPrice < 5) sellPrice = 5;
                    printf("Sold '%s' for %d coins!\n", hero->inventory[sellIdx].name, sellPrice);
                    hero->coins += sellPrice;
                    removeItemFromInventory(hero, sellIdx);
                } else {
                    fprintf(stderr, "Invalid item number.\n");
                }
            }
        } else if (choice == 3) {
            const int upgradeCost = 60;
            if (hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
                printf("Pusheen is already at Level %d!\n", hero->pusheenLevel);
            } else if (hero->coins >= upgradeCost) {
                hero->coins -= upgradeCost;
                hero->pusheenLevel = BOSS_UNLOCK_PUSHEEN_LVL;
                printf("Pusheen upgraded to Level %d! Remaining coins: %d\n",
                       hero->pusheenLevel, hero->coins);
            } else {
                fprintf(stderr, "Not enough coins to upgrade Pusheen (need %d, have %d).\n",
                        upgradeCost, hero->coins);
            }
        }
    } while (choice != 0);
}

void openDialogue(struct Adventurer *hero) {
    char line[INPUT_BUF_SIZE];
    int step1 = 0, step2 = 0;

    fputs("\n=== DIALOGUE WITH OLD CAT ===\n", stdout);
    fputs("Old Cat: 'Greetings, Knight! I heard dogs took Pusheen. What is your plan?'\n", stdout);
    fputs("1. 'I will charge straight into battle against the dogs!'\n", stdout);
    fputs("2. 'I will gather supplies and prepare first.'\n", stdout);
    fputs("3. 'Tell me about the villains.'\n", stdout);
    fputs("4. 'Give me a tip about hearts.'\n", stdout);
    fputs("5. 'Just passing by.'\n", stdout);
    fputs("Your choice (1-5) + Enter: ", stdout);

    if (!readLine(line, sizeof(line)) || sscanf(line, "%d", &step1) != 1) {
        step1 = 5;
    }

    if (step1 == 1) {
        fputs("\nOld Cat: 'Brave! Pick your combat style:'\n", stdout);
        fputs("1. 'Strike with full force!' (Ending #1)\n", stdout);
        fputs("2. 'Stay careful in close combat.' (Ending #2)\n", stdout);
        fputs("Your choice (1-2) + Enter: ", stdout);
        (void)(readLine(line, sizeof(line)) && sscanf(line, "%d", &step2) == 1);

        if (step2 == 1) {
            hero->strength += 2;
            hero->score += 50;
            fputs("\n[ENDING 1/5 - FIERCE STYLE]: Old Cat granted +2 Strength and +50 score!\n", stdout);
        } else {
            hero->defense += 2;
            hero->score += 40;
            fputs("\n[ENDING 2/5 - STEADY GUARD]: Old Cat taught defense (+2 Defense, +40 score)!\n", stdout);
        }
    } else if (step1 == 2) {
        hero->coins += 25;
        fputs("\n[ENDING 3/5 - SMART PREP]: Old Cat tossed +25 coins for the shop!\n", stdout);
    } else if (step1 == 3) {
        hero->score += 30;
        fputs("\n[ENDING 4/5 - LAIR TALE]: You learned the Boss holds Pusheen in the lair!\n", stdout);
    } else if (step1 == 4) {
        hero->score += 25;
        fputs("\n[ENDING 5/5 - HEART SECRET]: Defeating enemies spawns a heart [H] on the map!\n", stdout);
    } else {
        fputs("\n[ENDING 5/5 - QUIET CHAT]: Old Cat nodded and wished you luck.\n", stdout);
    }
}

int saveGameToFile(const struct Adventurer *hero, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: failed to open '%s' for writing.\n", filename);
        return 0;
    }

    char safeName[NAME_BUF_SIZE];
    snprintf(safeName, sizeof(safeName), "%s", hero->name);
    sanitizeToken(safeName);

    fprintf(fp, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            safeName,
            hero->hpCurrent,
            hero->hpMax,
            hero->strength,
            hero->defense,
            hero->coins,
            hero->score,
            hero->xp,
            hero->heroLevel,
            hero->pusheenLevel,
            hero->x,
            hero->y,
            hero->defeatedEnemiesCount,
            hero->heartsCollectedCount,
            hero->quest1Done,
            hero->quest2Done,
            hero->quest3Done);

    fprintf(fp, "%d\n", hero->invCount);
    for (int i = 0; i < hero->invCount; i++) {
        char nTok[64], tTok[32], dTok[96];
        snprintf(nTok, sizeof(nTok), "%s", hero->inventory[i].name);
        snprintf(tTok, sizeof(tTok), "%s", hero->inventory[i].type);
        snprintf(dTok, sizeof(dTok), "%s", hero->inventory[i].description);
        sanitizeToken(nTok);
        sanitizeToken(tTok);
        sanitizeToken(dTok);
        fprintf(fp, "%s %s %s %d\n", nTok, tTok, dTok, hero->inventory[i].price);
    }

    fclose(fp);
    printf("[SAVED] Result and game saved to '%s'!\n", filename);
    return 1;
}

int loadGameFromFile(struct Adventurer *hero, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: failed to open '%s' for reading.\n", filename);
        return 0;
    }

    char loadedName[NAME_BUF_SIZE];
    int hpCur, hpM, str, def, coins, score, xp, hLvl, pLvl, px, py, defCount, hCount, q1, q2, q3;

    int fieldsRead = fscanf(fp, "%49s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                            loadedName, &hpCur, &hpM, &str, &def, &coins, &score,
                            &xp, &hLvl, &pLvl, &px, &py, &defCount, &hCount, &q1, &q2, &q3);
    if (fieldsRead != 17) {
        fprintf(stderr, "Error: corrupted save file format in '%s'.\n", filename);
        fclose(fp);
        return 0;
    }

    int invTotal = 0;
    if (fscanf(fp, "%d", &invTotal) != 1 || invTotal < 0 || invTotal > 200) {
        fprintf(stderr, "Error: invalid item count in '%s'.\n", filename);
        fclose(fp);
        return 0;
    }

    char *newName = malloc(strlen(loadedName) + 1);
    if (newName == NULL) {
        fprintf(stderr, "Error: out of memory while loading hero name.\n");
        fclose(fp);
        return 0;
    }
    strcpy(newName, loadedName);
    for (size_t i = 0; newName[i] != '\0'; i++) {
        if (newName[i] == '_') newName[i] = ' ';
    }

    free(hero->name);
    hero->name = newName;
    hero->hpCurrent = hpCur;
    hero->hpMax = hpM;
    hero->strength = str;
    hero->defense = def;
    hero->coins = coins;
    hero->score = score;
    hero->xp = xp;
    hero->heroLevel = hLvl;
    hero->pusheenLevel = pLvl;
    hero->x = px;
    hero->y = py;
    hero->defeatedEnemiesCount = defCount;
    hero->heartsCollectedCount = hCount;
    hero->quest1Done = q1;
    hero->quest2Done = q2;
    hero->quest3Done = q3;

    clearInventory(hero);
    for (int i = 0; i < invTotal; i++) {
        char nTok[64], tTok[32], dTok[96];
        int price = 0;
        if (fscanf(fp, "%63s %31s %95s %d", nTok, tTok, dTok, &price) == 4) {
            sanitizeToken(nTok);
            sanitizeToken(tTok);
            sanitizeToken(dTok);
            addItemToInventory(hero, nTok, tTok, dTok, price);
        }
    }

    fclose(fp);
    printf("[LOADED] Game loaded from '%s'!\n", filename);
    return 1;
}