#include "map_combat.h"
#include "hero.h"
#include "io_utils.h"
#include "world_ui.h"
#include <stdio.h>
#include <stdlib.h>

static void fillTemplateMap(char target[MAP_ROWS][MAP_COLS]) {
    const char templateMap[MAP_ROWS][MAP_COLS] = {
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
        {'#', ' ', ' ', ' ', 'O', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', 'O', ' ', ' ', ' ', '#'},
        {'#', ' ', '#', '#', '#', ' ', '#', '#', ' ', '#', ' ', '#', '#', '#', ' ', '#', '#', ' ', ' ', '#'},
        {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
        {'#', '#', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', '#'},
        {'#', ' ', ' ', ' ', ' ', 'O', ' ', ' ', ' ', 'O', ' ', ' ', ' ', 'O', ' ', ' ', ' ', ' ', ' ', '#'},
        {'#', ' ', '#', '#', '#', ' ', '#', '#', ' ', '#', ' ', '#', '#', '#', ' ', '#', '#', ' ', ' ', '#'},
        {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
        {'#', ' ', ' ', ' ', 'O', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'O', ' ', ' ', ' ', ' ', ' ', '#'},
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
    };

    for (int i = 0; i < MAP_ROWS; i++) {
        for (int j = 0; j < MAP_COLS; j++) {
            target[i][j] = templateMap[i][j];
        }
    }
}

static int isCellOccupied(int x, int y,
                        const char baseMap[MAP_ROWS][MAP_COLS],
                        const struct Adventurer *hero,
                        const struct Enemy enemies[],
                        int enemyCount,
                        const struct HeartDrop hearts[],
                        int heartCount) {
    if (x <= 0 || x >= MAP_ROWS - 1 || y <= 0 || y >= MAP_COLS - 1) {
        return 1;
    }
    if (baseMap[x][y] != ' ') {
        return 1;
    }
    if (hero->x == x && hero->y == y) {
        return 1;
    }
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].isAlive && enemies[i].x == x && enemies[i].y == y) {
            return 1;
        }
    }
    for (int i = 0; i < heartCount; i++) {
        if (hearts[i].isActive && hearts[i].x == x && hearts[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static void spawnRandomHeartOnMap(const char baseMap[MAP_ROWS][MAP_COLS],
                                  const struct Adventurer *hero,
                                  const struct Enemy enemies[],
                                  int enemyCount,
                                  struct HeartDrop hearts[],
                                  int heartCount) {
    int slot = -1;
    for (int i = 0; i < heartCount; i++) {
        if (!hearts[i].isActive) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        return;
    }

    int freeX[MAP_ROWS * MAP_COLS];
    int freeY[MAP_ROWS * MAP_COLS];
    int freeCount = 0;

    for (int r = 1; r < MAP_ROWS - 1; r++) {
        for (int c = 1; c < MAP_COLS - 1; c++) {
            if (!isCellOccupied(r, c, baseMap, hero, enemies, enemyCount, hearts, heartCount)) {
                freeX[freeCount] = r;
                freeY[freeCount] = c;
                freeCount++;
            }
        }
    }

    if (freeCount > 0) {
        int idx = rand() % freeCount;
        hearts[slot].x = freeX[idx];
        hearts[slot].y = freeY[idx];
        hearts[slot].isActive = 1;
        printf("[+] A heart [H] appeared at (%d, %d)! Go pick it up!\n",
               hearts[slot].x, hearts[slot].y);
    }
}

static void drawGameMap(char mapBuffer[MAP_ROWS][MAP_COLS],
                        const struct Adventurer *hero,
                        const struct Enemy enemies[],
                        int enemyCount,
                        const struct HeartDrop hearts[],
                        int heartCount,
                        int levelNumber) {
    fillTemplateMap(mapBuffer);

    for (int i = 0; i < heartCount; i++) {
        if (hearts[i].isActive &&
            hearts[i].x >= 0 && hearts[i].x < MAP_ROWS &&
            hearts[i].y >= 0 && hearts[i].y < MAP_COLS) {
            mapBuffer[hearts[i].x][hearts[i].y] = 'H';
        }
    }

    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].isAlive &&
            enemies[i].x >= 0 && enemies[i].x < MAP_ROWS &&
            enemies[i].y >= 0 && enemies[i].y < MAP_COLS) {
            mapBuffer[enemies[i].x][enemies[i].y] = 'D';
        }
    }

    if (hero->x >= 0 && hero->x < MAP_ROWS && hero->y >= 0 && hero->y < MAP_COLS) {
        mapBuffer[hero->x][hero->y] = 'K';
    }

    printf("\n================ PUSHEEN'S RESCUE MISSION (LVL %d) ================\n", levelNumber);
    for (int i = 0; i < MAP_ROWS; i++) {
        for (int j = 0; j < MAP_COLS; j++) {
            putchar(mapBuffer[i][j]);
            putchar(' ');
        }
        putchar('\n');
    }
    fputs("[K] - Knight | [D] - Enemy | [H] - Heart | [O] - Pillar | [#] - Wall\n", stdout);
}

static int getAdjacentEnemyIndex(const struct Adventurer *hero, const struct Enemy enemies[], int enemyCount) {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].isAlive) {
            int manhattanDist = abs(hero->x - enemies[i].x) + abs(hero->y - enemies[i].y);
            if (manhattanDist == 1) {
                return i;
            }
        }
    }
    return -1;
}

static int executeHeroSkill(struct Adventurer *hero,
                            struct Enemy *target,
                            int skillChoice,
                            int *tempDefenseBonus) {
    int rawDamage = 0;

    switch (skillChoice) {
        case 1:
            rawDamage = hero->strength + (rand() % 6);
            break;
        case 2:
            if ((rand() % 100) < 20) {
                fputs(">> Cat Claw missed!\n", stdout);
                return 0;
            }
            rawDamage = hero->strength + 10 + (rand() % 8);
            break;
        case 3:
            *tempDefenseBonus += 8;
            hero->score += 15;
            fputs(">> Guard Stance active (+8 Defense this turn)!\n", stdout);
            return 0;
        case 4:
            hero->hpCurrent += 22;
            if (hero->hpCurrent > hero->hpMax) hero->hpCurrent = hero->hpMax;
            printf(">> Purr Heal restored HP! Current HP: %d/%d\n", hero->hpCurrent, hero->hpMax);
            return 0;
        case 5:
            rawDamage = hero->strength + 4 + (rand() % 6);
            hero->coins += 10;
            fputs(">> Slipper Strike! +10 coins.\n", stdout);
            break;
        default:
            rawDamage = hero->strength;
            break;
    }

    int finalDamage = rawDamage - target->defense;
    if (finalDamage < 1) {
        finalDamage = 1;
    }

    target->hp -= finalDamage;
    if (target->hp < 0) {
        target->hp = 0;
    }

    printf(">> %-20s | Damage: -%2d (enemy DEF: %d) | Enemy HP: %3d/%3d ",
           hero->name, finalDamage, target->defense, target->hp, target->maxHp);
    printHpBar(target->hp, target->maxHp);
    putchar('\n');
    gainHeroXp(hero, 25);
    return 1;
}

static void handleEnemyDefeatOrCounterattack(struct Adventurer *hero,
                                            struct Enemy *target,
                                            int *activeEnemies,
                                            int *levelHearts,
                                            int scoreReward,
                                            int tempDefenseBonus,
                                            const char baseMap[MAP_ROWS][MAP_COLS],
                                            const struct Enemy enemies[],
                                            int enemyCount,
                                            struct HeartDrop hearts[],
                                            int heartCount) {
    if (target->hp == 0) {
        target->isAlive = 0;
        (*activeEnemies)--;
        hero->defeatedEnemiesCount++;
        hero->score += scoreReward;
        hero->coins += 15;
        printf(">> Enemy '%s' defeated! +%d score, +15 coins (%d left).\n",
               target->name, scoreReward, *activeEnemies);
        gainHeroXp(hero, 50);
        spawnRandomHeartOnMap(baseMap, hero, enemies, enemyCount, hearts, heartCount);
        updateQuestsProgress(hero);
        return;
    }

    int totalHeroDef = hero->defense + tempDefenseBonus;
    int rawEnemyDmg = target->attack + (rand() % 5);
    int enemyDmg = rawEnemyDmg - totalHeroDef;
    if (enemyDmg < 1) {
        enemyDmg = 1;
    }

    hero->hpCurrent -= enemyDmg;

    if (hero->hpCurrent <= 0) {
        (*levelHearts)--;
        printf("<< Enemy '%s' dealt %d damage! Knight lost 1 heart!\n",
               target->name, enemyDmg);
        if (*levelHearts > 0) {
            hero->hpCurrent = hero->hpMax;
            fputs("   A heart revived the Knight! ", stdout);
            printLevelHearts(*levelHearts, LEVEL_MAX_HEARTS);
            putchar('\n');
        } else {
            hero->hpCurrent = 0;
        }
    } else {
        printf("<< %-20s | Hit: -%2d HP (your DEF: %d) | Knight HP: %3d/%3d ",
               target->name, enemyDmg, totalHeroDef, hero->hpCurrent, hero->hpMax);
        printHpBar(hero->hpCurrent, hero->hpMax);
        putchar('\n');
    }
}

static void performAttackOnEnemy(struct Adventurer *hero,
                                 struct Enemy *target,
                                 int *activeEnemies,
                                 int *levelHearts,
                                 int scoreReward,
                                 const char baseMap[MAP_ROWS][MAP_COLS],
                                 const struct Enemy enemies[],
                                 int enemyCount,
                                 struct HeartDrop hearts[],
                                 int heartCount) {
    char line[INPUT_BUF_SIZE];
    int skillChoice = 1;
    int tempDefenseBonus = 0;

    fputs("\n--- CHOOSE ATTACK / SKILL (5 OPTIONS) ---\n", stdout);
    fputs("1. Quick Strike\n", stdout);
    fputs("2. Cat Claw (heavy strike, 80% accuracy)\n", stdout);
    fputs("3. Guard Stance (+8 Defense)\n", stdout);
    fputs("4. Purr Heal (+22 HP)\n", stdout);
    fputs("5. Slipper Strike (+10 coins)\n", stdout);
    fputs("Choice (1-5) + Enter: ", stdout);

    if (readLine(line, sizeof(line))) {
        int parsed = 0;
        if (sscanf(line, "%d", &parsed) == 1 && parsed >= 1 && parsed <= 5) {
            skillChoice = parsed;
        }
    }

    (void)executeHeroSkill(hero, target, skillChoice, &tempDefenseBonus);
    handleEnemyDefeatOrCounterattack(hero, target, activeEnemies, levelHearts,
                                     scoreReward, tempDefenseBonus, baseMap,
                                     enemies, enemyCount, hearts, heartCount);
}

static int fightBossInLair(struct Adventurer *hero, int *levelHearts) {
    char line[INPUT_BUF_SIZE];
    struct Enemy boss = {0, 0, 180, 180, 22, 6, 1, "Lord of the Dog Lair"};

    fputs("\n====================================================================\n", stdout);
    fputs(" LEVEL 2 BOSS FIGHT: 5000+ score and Pusheen LVL 2 reached!\n", stdout);
    fputs(" The lair gate is open - battle the Boss to rescue Pusheen!\n", stdout);
    fputs("====================================================================\n", stdout);
    waitForEnter("Press [Enter] to start the Boss fight...");

    while (*levelHearts > 0 && boss.hp > 0) {
        int tempDef = 0;
        int skillChoice = 1;

        printf("\n--- BOSS BATTLE: %s ---\n", boss.name);
        printf("Boss HP: %3d/%3d ", boss.hp, boss.maxHp);
        printHpBar(boss.hp, boss.maxHp);
        printf(" | DEF: %d\n", boss.defense);

        fputs("Hearts: ", stdout);
        printLevelHearts(*levelHearts, LEVEL_MAX_HEARTS);
        printf(" | Knight HP: %3d/%3d ", hero->hpCurrent, hero->hpMax);
        printHpBar(hero->hpCurrent, hero->hpMax);
        printf(" | DEF: %d\n", hero->defense);

        fputs("Choose attack / skill (1-5) + Enter: ", stdout);
        if (readLine(line, sizeof(line))) {
            int parsed = 0;
            if (sscanf(line, "%d", &parsed) == 1 && parsed >= 1 && parsed <= 5) {
                skillChoice = parsed;
            }
        }

        (void)executeHeroSkill(hero, &boss, skillChoice, &tempDef);
        if (boss.hp <= 0) {
            boss.hp = 0;
            boss.isAlive = 0;
            break;
        }

        int totalHeroDef = hero->defense + tempDef;
        int rawBossDmg = boss.attack + (rand() % 6);
        int bossDmg = rawBossDmg - totalHeroDef;
        if (bossDmg < 1) bossDmg = 1;

        hero->hpCurrent -= bossDmg;
        if (hero->hpCurrent <= 0) {
            (*levelHearts)--;
            printf("<< %s dealt %d damage! Knight lost 1 heart!\n", boss.name, bossDmg);
            if (*levelHearts > 0) {
                hero->hpCurrent = hero->hpMax;
                fputs("   A heart revived the Knight! ", stdout);
                printLevelHearts(*levelHearts, LEVEL_MAX_HEARTS);
                putchar('\n');
            } else {
                hero->hpCurrent = 0;
            }
        } else {
            printf("<< %s counterattacks: -%2d HP (your DEF: %d) | Knight HP: %3d/%3d\n",
                   boss.name, bossDmg, totalHeroDef, hero->hpCurrent, hero->hpMax);
        }
    }

    if (*levelHearts <= 0) {
        fputs("\n====================================================================\n", stdout);
        fputs(" The Lair Boss defeated the Knight... Try again!\n", stdout);
        fputs("====================================================================\n", stdout);
        return 0;
    }

    hero->score += 1000;
    hero->coins += 100;
    gainHeroXp(hero, 150);
    fputs("\n====================================================================\n", stdout);
    fputs(" BOSS DEFEATED! PRINCESS PUSHEEN RESCUED FROM THE LAIR!\n", stdout);
    fputs(" Rewards: +1000 score, +100 coins, and +150 XP!\n", stdout);
    fputs("====================================================================\n", stdout);
    waitForEnter("Press [Enter] to return to the menu...");
    return 1;
}

static void collectHeartIfPresent(struct Adventurer *hero,
                                  struct HeartDrop hearts[],
                                  int heartCount,
                                  int *levelHearts) {
    for (int i = 0; i < heartCount; i++) {
        if (hearts[i].isActive && hearts[i].x == hero->x && hearts[i].y == hero->y) {
            hearts[i].isActive = 0;
            hero->hpCurrent = hero->hpMax;
            hero->score += 50;
            hero->heartsCollectedCount++;
            if (*levelHearts < LEVEL_MAX_HEARTS) {
                (*levelHearts)++;
            }
            printf("\n[<3] Picked up a heart! HP restored (%d/%d), +50 score! Hearts: ",
                   hero->hpCurrent, hero->hpMax);
            printLevelHearts(*levelHearts, LEVEL_MAX_HEARTS);
            putchar('\n');
            updateQuestsProgress(hero);
        }
    }
}

int playRescueLevel(struct Adventurer *hero, int levelNumber) {
    char line[INPUT_BUF_SIZE];
    char mapBuffer[MAP_ROWS][MAP_COLS];
    char baseMap[MAP_ROWS][MAP_COLS];
    fillTemplateMap(baseMap);

    if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
        int tmpHearts = LEVEL_MAX_HEARTS;
        return fightBossInLair(hero, &tmpHearts);
    }

    fputs("\n====================================================================\n", stdout);
    fputs(" STORY: Villainous dogs kidnapped Princess Pusheen!\n", stdout);
    fputs(" Our brave Cat Knight came to save her,\n", stdout);
    fputs(" but must first defeat the villain dogs along the path!\n", stdout);
    fputs("====================================================================\n", stdout);
    waitForEnter("Press [Enter] to continue to the map...");

    int hpScale = (levelNumber == 2) ? 15 : 0;
    int atkScale = (levelNumber == 2) ? 3 : 0;
    int defScale = (levelNumber == 2) ? 2 : 0;
    int scoreReward = (levelNumber == 2) ? 250 : 200;

    struct Enemy enemies[MAX_ENEMIES] = {
        {3, 5, 40 + hpScale, 40 + hpScale, 10 + atkScale, 2 + defScale, 1, "Street Dog #1"},
        {5, 15, 50 + hpScale, 50 + hpScale, 12 + atkScale, 3 + defScale, 1, "Street Dog #2"},
        {1, 12, 45 + hpScale, 45 + hpScale, 11 + atkScale, 2 + defScale, 1, "Mean Dog"},
        {7, 10, 65 + hpScale, 65 + hpScale, 15 + atkScale, 4 + defScale, 1, "Bulldog Guard"},
        {8, 16, 55 + hpScale, 55 + hpScale, 13 + atkScale, 3 + defScale, 1, "Grumpy Hound"}
    };
    int activeEnemies = MAX_ENEMIES;

    struct HeartDrop hearts[MAX_HEART_DROPS];
    for (int i = 0; i < MAX_HEART_DROPS; i++) {
        hearts[i].x = -1;
        hearts[i].y = -1;
        hearts[i].isActive = 0;
    }

    int levelHearts = LEVEL_MAX_HEARTS;
    hero->hpCurrent = hero->hpMax;
    hero->x = 1;
    hero->y = 1;

    printf("\n=== LEVEL %d: PUSHEEN'S RESCUE MISSION ===\n", levelNumber);
    fputs("Move: w/a/s/d | Attack: f | Shop: b | Save result: k | Menu: q + Enter\n", stdout);

    while (levelHearts > 0 && activeEnemies > 0) {
        drawGameMap(mapBuffer, hero, enemies, MAX_ENEMIES, hearts, MAX_HEART_DROPS, levelNumber);

        printf("\nHearts: ");
        printLevelHearts(levelHearts, LEVEL_MAX_HEARTS);
        printf(" | HP: %3d/%3d ", hero->hpCurrent, hero->hpMax);
        printHpBar(hero->hpCurrent, hero->hpMax);
        printf(" | Pusheen LVL:%d/2 | Score: %05d/%d | Coins: %d\n",
               hero->pusheenLevel, hero->score, BOSS_UNLOCK_SCORE, hero->coins);

        if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
            return fightBossInLair(hero, &levelHearts);
        }

        int nearbyIdx = getAdjacentEnemyIndex(hero, enemies, MAX_ENEMIES);
        if (nearbyIdx != -1) {
            printf("\n[!] Enemy '%s' is adjacent (1 tile away)! HP: %d/%d, DEF: %d ",
                   enemies[nearbyIdx].name, enemies[nearbyIdx].hp,
                   enemies[nearbyIdx].maxHp, enemies[nearbyIdx].defense);
            printHpBar(enemies[nearbyIdx].hp, enemies[nearbyIdx].maxHp);
            fputs(" - press [f] + Enter to ATTACK!\n", stdout);
        }

        fputs("Keys: [w/a/s/d] move | [f] ATTACK | [b] shop | [k] save result | [q] menu + Enter: ", stdout);
        if (!readLine(line, sizeof(line))) {
            fprintf(stderr, "\nInput read error from stdin.\n");
            return 0;
        }

        char cmd = '\0';
        if (sscanf(line, " %c", &cmd) != 1) {
            fputs("Map refreshed. Use w, a, s, d, f, b, k, or q + Enter.\n", stdout);
            continue;
        }

        if (cmd == 'q' || cmd == 'Q') {
            fputs("Returning to menu...\n", stdout);
            return 0;
        }

        if (cmd == 'b' || cmd == 'B') {
            openShopMenu(hero);
            if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
                return fightBossInLair(hero, &levelHearts);
            }
            continue;
        }

        if (cmd == 'k' || cmd == 'K') {
            saveGameToFile(hero, SAVE_FILE_NAME);
            continue;
        }

        if (cmd == 'f' || cmd == 'F') {
            if (nearbyIdx != -1) {
                performAttackOnEnemy(hero, &enemies[nearbyIdx], &activeEnemies, &levelHearts,
                                     scoreReward, baseMap, enemies, MAX_ENEMIES,
                                     hearts, MAX_HEART_DROPS);
                if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
                    return fightBossInLair(hero, &levelHearts);
                }
            } else {
                fputs("No enemy within 1 tile to attack! Step right next to one.\n", stdout);
            }
            continue;
        }

        int newX = hero->x;
        int newY = hero->y;

        if (cmd == 'w' || cmd == 'W') {
            newX--;
        } else if (cmd == 's' || cmd == 'S') {
            newX++;
        } else if (cmd == 'a' || cmd == 'A') {
            newY--;
        } else if (cmd == 'd' || cmd == 'D') {
            newY++;
        } else {
            fprintf(stderr, "Unknown key '%c'. Use w, a, s, d, f, b, k, or q.\n", cmd);
            continue;
        }

        if (newX < 0 || newX >= MAP_ROWS || newY < 0 || newY >= MAP_COLS ||
            baseMap[newX][newY] == '#' || baseMap[newX][newY] == 'O') {
            fputs("Blocked by wall or pillar! Choose another direction.\n", stdout);
            continue;
        }

        int hitEnemyIdx = -1;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].isAlive && enemies[i].x == newX && enemies[i].y == newY) {
                hitEnemyIdx = i;
                break;
            }
        }

        if (hitEnemyIdx != -1) {
            printf("Enemy '%s' is on that tile! Press [f] + Enter to ATTACK (or anything else to hold): ",
                   enemies[hitEnemyIdx].name);
            if (!readLine(line, sizeof(line))) {
                return 0;
            }
            char strikeChoice = '\0';
            if (sscanf(line, " %c", &strikeChoice) == 1 && (strikeChoice == 'f' || strikeChoice == 'F')) {
                performAttackOnEnemy(hero, &enemies[hitEnemyIdx], &activeEnemies, &levelHearts,
                                     scoreReward, baseMap, enemies, MAX_ENEMIES,
                                     hearts, MAX_HEART_DROPS);
                if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
                    return fightBossInLair(hero, &levelHearts);
                }
            } else {
                fputs("Knight held position in front of the enemy.\n", stdout);
            }
        } else {
            hero->x = newX;
            hero->y = newY;
            collectHeartIfPresent(hero, hearts, MAX_HEART_DROPS, &levelHearts);
            if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
                return fightBossInLair(hero, &levelHearts);
            }
        }
    }

    if (levelHearts <= 0) {
        fputs("\n====================================================================\n", stdout);
        fputs(" All 3 hearts are gone... Try again!\n", stdout);
        fputs("====================================================================\n", stdout);
        return 0;
    }

    int bonus = (levelHearts * 100) + hero->hpCurrent;
    hero->score += bonus;
    updateQuestsProgress(hero);

    if (hero->score >= BOSS_UNLOCK_SCORE && hero->pusheenLevel >= BOSS_UNLOCK_PUSHEEN_LVL) {
        return fightBossInLair(hero, &levelHearts);
    }

    fputs("\n====================================================================\n", stdout);
    printf(" CONGRATULATIONS! YOU CLEARED LEVEL %d!\n", levelNumber);
    fputs(" Enemies defeated, but Pusheen is held in the Boss Lair!\n", stdout);
    printf(" (Current: %d/%d score, Pusheen LVL %d/2)\n",
           hero->score, BOSS_UNLOCK_SCORE, hero->pusheenLevel);
    fputs("====================================================================\n", stdout);
    waitForEnter("Press [Enter] to return to the menu...");
    return 1;
}