#include "hero.h"
#include "io_utils.h"
#include "map_combat.h"
#include "types.h"
#include "world_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void printHeroStatsAndInventory(const struct Adventurer *hero) {
    printf("\n=== STATS AND INVENTORY ===\n");
    printf("Name: %s\n", hero->name);
    printf("Hero Level: %d/%d | XP: %d | Pusheen LVL: %d/%d\n",
           hero->heroLevel, MAX_HERO_LEVEL, hero->xp, hero->pusheenLevel, BOSS_UNLOCK_PUSHEEN_LVL);
    printf("HP: %d/%d | Strength (ATK): %d | Defense (DEF): %d\n",
           hero->hpCurrent, hero->hpMax, hero->strength, hero->defense);
    printf("Coins: %d | Score: %05d (Boss at %d score and Pusheen LVL 2)\n",
           hero->coins, hero->score, BOSS_UNLOCK_SCORE);
    printf("Coordinates: (%d, %d)\n", hero->x, hero->y);

    fputs("Inventory:\n", stdout);
    if (hero->invCount == 0) {
        fputs("  (empty)\n", stdout);
    } else {
        for (int i = 0; i < hero->invCount; i++) {
            printf("  %d. [%s] %s - %s (%d coins)\n",
                   i + 1,
                   hero->inventory[i].type,
                   hero->inventory[i].name,
                   hero->inventory[i].description,
                   hero->inventory[i].price);
        }
    }
}

static void handleSaveLoadSubmenu(struct Adventurer *hero) {
    char line[INPUT_BUF_SIZE];
    int subChoice = -1;

    fputs("\n--- SAVE AND LOAD ---\n", stdout);
    fputs("1. Save result / game to file\n", stdout);
    fputs("2. Load game from file\n", stdout);
    fputs("0. Back\n", stdout);
    fputs("Your choice + Enter: ", stdout);

    if (!readLine(line, sizeof(line)) || sscanf(line, "%d", &subChoice) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return;
    }

    if (subChoice == 1) {
        saveGameToFile(hero, SAVE_FILE_NAME);
    } else if (subChoice == 2) {
        loadGameFromFile(hero, SAVE_FILE_NAME);
    }
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    char heroName[NAME_BUF_SIZE] = "Cat Knight";
    if (argc >= 2 && argv[1][0] != '\0') {
        snprintf(heroName, sizeof(heroName), "%s", argv[1]);
        trimNewlineOrSpaces(heroName);
    }

    struct Adventurer *hero = createAdventurer(heroName);
    if (hero == NULL) {
        fprintf(stderr, "Critical error: failed to create hero.\n");
        return EXIT_FAILURE;
    }

    addItemToInventory(hero, "Golden Slipper", "Weapon", "Legendary close-combat slipper", 40);
    addItemToInventory(hero, "Milk Drink", "Potion", "Restores energy and alertness", 25);
    addItemToInventory(hero, "Princess Star", "Artifact", "Symbol of Pusheen's rescue", 60);

    char line[INPUT_BUF_SIZE];
    int mainChoice = -1;

    do {
        printf("\n=== MAIN MENU ===\n");
        printf("Hero: %s | LVL %d | Pusheen LVL %d/2 | Score: %05d/%d | Coins: %d\n",
               hero->name, hero->heroLevel, hero->pusheenLevel,
               hero->score, BOSS_UNLOCK_SCORE, hero->coins);
        fputs("1. Level\n", stdout);
        fputs("2. Stats and Inventory\n", stdout);
        fputs("3. Save Result / Load\n", stdout);
        fputs("4. Shop\n", stdout);
        fputs("5. Dialogue\n", stdout);
        fputs("6. Quests\n", stdout);
        fputs("0. Exit\n", stdout);
        fputs("Your choice + Enter: ", stdout);

        if (!readLine(line, sizeof(line))) {
            fprintf(stderr, "\nInput read error from stdin.\n");
            break;
        }

        if (sscanf(line, "%d", &mainChoice) != 1) {
            fprintf(stderr, "Invalid input '%s'. Enter a number (0-6).\n", line);
            continue;
        }

        switch (mainChoice) {
            case 1:
                openLvlSubmenu(hero);
                break;
            case 2:
                printHeroStatsAndInventory(hero);
                break;
            case 3:
                handleSaveLoadSubmenu(hero);
                break;
            case 4:
                openShopMenu(hero);
                break;
            case 5:
                openDialogue(hero);
                break;
            case 6:
                showQuestsMenu(hero);
                break;
            case 0:
                fputs("Exiting game. See you next time!\n", stdout);
                break;
            default:
                fprintf(stderr, "Unknown menu option: %d\n", mainChoice);
                break;
        }
    } while (mainChoice != 0);

    freeAdventurer(hero);
    return EXIT_SUCCESS;
}