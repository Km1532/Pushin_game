#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAP_ROWS 10
#define MAP_COLS 20
#define MAX_ENEMIES 4
#define MAX_HEART_DROPS 8
#define LEVEL_MAX_HEARTS 3
#define INPUT_BUF_SIZE 128
#define NAME_BUF_SIZE 50

struct Item {
    char *name;
    char *description;
};

struct Adventurer {
    char *name;
    int hpCurrent;
    int hpMax;
    int strength;
    int coins;
    int score;
    int x;
    int y;

    struct Item *inventory;
    int invCount;
    int invCapacity;
};

struct Enemy {
    int x;
    int y;
    int hp;
    int maxHp;
    int attack;
    int isAlive;
    char name[30];
};

struct HeartDrop {
    int x;
    int y;
    int isActive;
};

static int readLine(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return 1;
}

static void waitForEnter(const char *prompt) {
    char dummy[INPUT_BUF_SIZE];
    fputs(prompt, stdout);
    (void)readLine(dummy, sizeof(dummy));
}

static void trimNewlineOrSpaces(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ')) {
        str[len - 1] = '\0';
        len--;
    }
}

static void printLevelHearts(int currentHearts, int maxHearts) {
    if (currentHearts < 0) currentHearts = 0;
    if (currentHearts > maxHearts) currentHearts = maxHearts;

    putchar('[');
    for (int i = 0; i < maxHearts; i++) {
        if (i < currentHearts) {
            fputs("<3", stdout);
        } else {
            fputs("--", stdout);
        }
        if (i < maxHearts - 1) {
            putchar(' ');
        }
    }
    printf("] (%d/%d)", currentHearts, maxHearts);
}

static void printHpBar(int currentHp, int maxHp) {
    if (currentHp < 0) currentHp = 0;
    if (maxHp <= 0) maxHp = 100;

    int segments = 5;
    int filled = (currentHp * segments + maxHp - 1) / maxHp;
    if (currentHp > 0 && filled == 0) {
        filled = 1;
    }
    if (filled > segments) {
        filled = segments;
    }

    putchar('[');
    for (int i = 0; i < segments; i++) {
        if (i < filled) {
            fputs("##", stdout);
        } else {
            fputs("..", stdout);
        }
        if (i < segments - 1) {
            putchar(' ');
        }
    }
    putchar(']');
}

struct Adventurer* createAdventurer(const char *nameStr) {
    struct Adventurer *hero = malloc(sizeof(struct Adventurer));
    if (hero == NULL) {
        fprintf(stderr, "Pomylka: ne vdalosya vydilyty pam'yat' dlya struktury Adventurer.\n");
        return NULL;
    }

    hero->name = malloc(strlen(nameStr) + 1);
    if (hero->name == NULL) {
        fprintf(stderr, "Pomylka: ne vdalosya vydilyty pam'yat' dlya imeni heroya.\n");
        free(hero);
        return NULL;
    }
    strcpy(hero->name, nameStr);

    hero->hpMax = 100;
    hero->hpCurrent = hero->hpMax;
    hero->strength = 35;
    hero->coins = 50;
    hero->score = 0;
    hero->x = 1;
    hero->y = 1;

    hero->invCapacity = 3;
    hero->invCount = 0;
    hero->inventory = calloc((size_t)hero->invCapacity, sizeof(struct Item));
    if (hero->inventory == NULL) {
        fprintf(stderr, "Pomylka: ne vdalosya vydilyty pam'yat' dlya inventaryu.\n");
        free(hero->name);
        free(hero);
        return NULL;
    }

    return hero;
}

int addItemToInventory(struct Adventurer *hero, const char *itemName, const char *itemDesc) {
    if (hero == NULL) return 0;

    if (hero->invCount == hero->invCapacity) {
        int newCap = hero->invCapacity * 2;
        struct Item *tmp = realloc(hero->inventory, (size_t)newCap * sizeof(struct Item));
        if (tmp == NULL) {
            fprintf(stderr, "Poperedzhennya: ne vdalosya rozshyryty inventar.\n");
            return 0;
        }
        hero->inventory = tmp;
        hero->invCapacity = newCap;
    }

    char *nCopy = malloc(strlen(itemName) + 1);
    char *dCopy = malloc(strlen(itemDesc) + 1);
    if (nCopy == NULL || dCopy == NULL) {
        fprintf(stderr, "Poperedzhennya: ne vystachaye pam'yati dlya predmeta '%s'.\n", itemName);
        free(nCopy);
        free(dCopy);
        return 0;
    }

    strcpy(nCopy, itemName);
    strcpy(dCopy, itemDesc);

    hero->inventory[hero->invCount].name = nCopy;
    hero->inventory[hero->invCount].description = dCopy;
    hero->invCount++;
    return 1;
}

void freeAdventurer(struct Adventurer *hero) {
    if (hero == NULL) return;
    for (int i = 0; i < hero->invCount; i++) {
        free(hero->inventory[i].name);
        free(hero->inventory[i].description);
    }
    free(hero->inventory);
    free(hero->name);
    free(hero);
}

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
        printf("[+] Na karti v koordynatakh (%d, %d) z'yavylosya serdechko [H]! Pidbyry yoho!\n",
               hearts[slot].x, hearts[slot].y);
    }
}

void drawGameMap(char mapBuffer[MAP_ROWS][MAP_COLS],
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
    fputs("[K] - Lytsar | [D] - Voroh (Cherepakha / Sobaka) | [H] - Serdechko | [O] - Kolona | [#] - Stina\n", stdout);
}

int getAdjacentEnemyIndex(const struct Adventurer *hero, const struct Enemy enemies[], int enemyCount) {
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

static void handleEnemyDefeatOrCounterattack(struct Adventurer *hero,
                                            struct Enemy *target,
                                            int *activeEnemies,
                                            int *levelHearts,
                                            int scoreReward,
                                            const char baseMap[MAP_ROWS][MAP_COLS],
                                            const struct Enemy enemies[],
                                            int enemyCount,
                                            struct HeartDrop hearts[],
                                            int heartCount) {
    if (target->hp == 0) {
        target->isAlive = 0;
        (*activeEnemies)--;
        hero->score += scoreReward;
        printf(">> Voroh '%s' peremozhenyy! +%d ochok (Zalyshylos' vorohiv: %d).\n",
               target->name, scoreReward, *activeEnemies);
        spawnRandomHeartOnMap(baseMap, hero, enemies, enemyCount, hearts, heartCount);
        return;
    }

    int enemyDmg = target->attack + (rand() % 5);
    hero->hpCurrent -= enemyDmg;

    if (hero->hpCurrent <= 0) {
        (*levelHearts)--;
        printf("<< Voroh '%s' zavdav %d shkody! Lytsar vtratyv 1 serdechko!\n",
               target->name, enemyDmg);
        if (*levelHearts > 0) {
            hero->hpCurrent = hero->hpMax;
            fputs("   Serdechko vidnovylo sily Lytsarya! ", stdout);
            printLevelHearts(*levelHearts, LEVEL_MAX_HEARTS);
            putchar('\n');
        } else {
            hero->hpCurrent = 0;
        }
    } else {
        printf("<< %-20s | Udar u vidpovid': -%2d HP | Lytsar HP: %3d/%3d ",
               target->name, enemyDmg, hero->hpCurrent, hero->hpMax);
        printHpBar(hero->hpCurrent, hero->hpMax);
        putchar('\n');
    }
}

void performAttackOnEnemy(struct Adventurer *hero,
                          struct Enemy *target,
                          int *activeEnemies,
                          int *levelHearts,
                          int scoreReward,
                          const char baseMap[MAP_ROWS][MAP_COLS],
                          const struct Enemy enemies[],
                          int enemyCount,
                          struct HeartDrop hearts[],
                          int heartCount) {
    int dmg = hero->strength + (rand() % 10);
    target->hp -= dmg;
    if (target->hp < 0) {
        target->hp = 0;
    }

    printf(">> %-20s | [UDAR] Shkoda: -%2d | Voroh HP: %3d/%3d ",
           hero->name, dmg, target->hp, target->maxHp);
    printHpBar(target->hp, target->maxHp);
    putchar('\n');

    handleEnemyDefeatOrCounterattack(hero, target, activeEnemies, levelHearts,
                                     scoreReward, baseMap, enemies, enemyCount,
                                     hearts, heartCount);
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
            if (*levelHearts < LEVEL_MAX_HEARTS) {
                (*levelHearts)++;
            }
            printf("\n[<3] Ty pidibrav serdechko! HP povnistyu vidnovleno (%d/%d), +50 ochok! Serdechka: ",
                   hero->hpCurrent, hero->hpMax);
            printLevelHearts(*levelHearts, LEVEL_MAX_HEARTS);
            putchar('\n');
        }
    }
}

int playRescueLevel(struct Adventurer *hero, int levelNumber) {
    char line[INPUT_BUF_SIZE];
    char mapBuffer[MAP_ROWS][MAP_COLS];
    char baseMap[MAP_ROWS][MAP_COLS];
    fillTemplateMap(baseMap);

    fputs("\n====================================================================\n", stdout);
    fputs(" SYUZHET: LikhodiYi-sobaky vykraly Pryntsesu Pushyn!\n", stdout);
    fputs(" Nash khorobryy Kotyachyy Lytsar pryyshov yiyi ryatuvaty,\n", stdout);
    fputs(" ale pershe yomu treba peremochty cherepakh ta vorohiv na shlyakhu!\n", stdout);
    fputs("====================================================================\n", stdout);
    waitForEnter("Natysni [Enter], shchob prodovzhyty ta vyty na kartu...");

    int hpScale = (levelNumber == 2) ? 15 : 0;
    int atkScale = (levelNumber == 2) ? 3 : 0;
    int scoreReward = (levelNumber == 2) ? 250 : 200;

    struct Enemy enemies[MAX_ENEMIES] = {
        {3, 5, 40 + hpScale, 40 + hpScale, 10 + atkScale, 1, "Cherepakha #1"},
        {5, 15, 50 + hpScale, 50 + hpScale, 12 + atkScale, 1, "Cherepakha #2"},
        {1, 12, 45 + hpScale, 45 + hpScale, 11 + atkScale, 1, "Zlyy Pes"},
        {7, 10, 65 + hpScale, 65 + hpScale, 15 + atkScale, 1, "Pansyrna Cherepakha"}
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

    printf("\n=== %d LVL: PUSHEEN'S RESCUE MISSION ===\n", levelNumber);
    fputs("Rukh: w / a / s / d + Enter | Knopka udaru (vprytul, 1 klytynka): f + Enter | q - menyu\n", stdout);

    while (levelHearts > 0 && activeEnemies > 0) {
        drawGameMap(mapBuffer, hero, enemies, MAX_ENEMIES, hearts, MAX_HEART_DROPS, levelNumber);

        printf("\nSerdechka: ");
        printLevelHearts(levelHearts, LEVEL_MAX_HEARTS);
        printf(" | HP: %3d/%3d ", hero->hpCurrent, hero->hpMax);
        printHpBar(hero->hpCurrent, hero->hpMax);
        printf(" | Ochky: %05d | Vorohiv: %d/%d\n",
               hero->score, activeEnemies, MAX_ENEMIES);

        int nearbyIdx = getAdjacentEnemyIndex(hero, enemies, MAX_ENEMIES);
        if (nearbyIdx != -1) {
            printf("\n[!] Voroh '%s' poruch (v 1 klytuntsi)! HP: %d/%d ",
                   enemies[nearbyIdx].name, enemies[nearbyIdx].hp, enemies[nearbyIdx].maxHp);
            printHpBar(enemies[nearbyIdx].hp, enemies[nearbyIdx].maxHp);
            fputs(" — natysny [f] + Enter dlya UDARU!\n", stdout);
        }

        fputs("Knopky: [w/a/s/d] rukh | [f] UDAR | [q] menyu + Enter: ", stdout);
        if (!readLine(line, sizeof(line))) {
            fprintf(stderr, "\nPomylka chytannya vvedennya z stdin.\n");
            return 0;
        }

        char cmd = '\0';
        if (sscanf(line, " %c", &cmd) != 1) {
            fputs("Karta onovlena. Vykorystovuy w, a, s, d, f abo q + Enter.\n", stdout);
            continue;
        }

        if (cmd == 'q' || cmd == 'Q') {
            fputs("Povernennya do menyu...\n", stdout);
            return 0;
        }

        if (cmd == 'f' || cmd == 'F') {
            if (nearbyIdx != -1) {
                performAttackOnEnemy(hero, &enemies[nearbyIdx], &activeEnemies, &levelHearts,
                                     scoreReward, baseMap, enemies, MAX_ENEMIES,
                                     hearts, MAX_HEART_DROPS);
            } else {
                fputs("Poryad (v 1 klytuntsi) nemaye voroha dlya udaru! Pidydi vprytul.\n", stdout);
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
            fprintf(stderr, "Nevidoma knopka '%c'. Vykorystovuy w, a, s, d, f abo q.\n", cmd);
            continue;
        }

        if (newX < 0 || newX >= MAP_ROWS || newY < 0 || newY >= MAP_COLS ||
            baseMap[newX][newY] == '#' || baseMap[newX][newY] == 'O') {
            fputs("Pereshkoda na shlyakhu (stina abo kolona)! Obery inshyy napryamok.\n", stdout);
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
            printf("Voroh '%s' na klytuntsi! Natysny [f] + Enter dlya UDARU (abo bud'-shcho inshe, shchob zalyshytys'): ",
                   enemies[hitEnemyIdx].name);
            if (!readLine(line, sizeof(line))) {
                return 0;
            }
            char strikeChoice = '\0';
            if (sscanf(line, " %c", &strikeChoice) == 1 && (strikeChoice == 'f' || strikeChoice == 'F')) {
                performAttackOnEnemy(hero, &enemies[hitEnemyIdx], &activeEnemies, &levelHearts,
                                     scoreReward, baseMap, enemies, MAX_ENEMIES,
                                     hearts, MAX_HEART_DROPS);
            } else {
                fputs("Lytsar zatrymavsya pered vorohom.\n", stdout);
            }
        } else {
            hero->x = newX;
            hero->y = newY;
            collectHeartIfPresent(hero, hearts, MAX_HEART_DROPS, &levelHearts);
        }
    }

    if (levelHearts <= 0) {
        fputs("\n====================================================================\n", stdout);
        fputs(" U Lytsarya zakinchylysya vsi 3 serdechka... Sprobuy shche raz!\n", stdout);
        fputs("====================================================================\n", stdout);
        return 0;
    }

    int bonus = (levelHearts * 100) + hero->hpCurrent;
    hero->score += bonus;

    fputs("\n====================================================================\n", stdout);
    printf(" VITAJEMO! TY USPISHNO PROYSHOV %d LVL!\n", levelNumber);
    fputs(" Cherepakhy ta vorohy peremozheni, ale Pushyn zabraly v yikhn'o lohovo,\n", stdout);
    fputs(" de yiyi trymaYe yikhniy bos! Povertayemosya do menyu.\n", stdout);
    printf(" Nabranыkh ochok: %05d (vklyuchayuchy +%d bonus)\n", hero->score, bonus);
    fputs("====================================================================\n", stdout);
    waitForEnter("Natysni [Enter], shchob povernutysya v menyu...");
    return 1;
}

static void openLvlSubmenu(struct Adventurer *hero) {
    char line[INPUT_BUF_SIZE];
    int lvlChoice = -1;

    do {
        fputs("\n--- VYBIR LVL ---\n", stdout);
        fputs("1. 1 lvl (Pochaty / Pereproyty)\n", stdout);
        fputs("2. 2 lvl\n", stdout);
        fputs("3. dlc (Skoro bude — poky v rozrobtsi)\n", stdout);
        fputs("0. Nazad do golovnoho menyu\n", stdout);
        fputs("Vash vybir + Enter: ", stdout);

        if (!readLine(line, sizeof(line))) {
            fprintf(stderr, "\nPomylka chytannya vvedennya z stdin.\n");
            return;
        }

        if (sscanf(line, "%d", &lvlChoice) != 1) {
            fprintf(stderr, "Nevirnyy vvid '%s'. Vvedy chyslo (0-3).\n", line);
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
                fputs("\n[DLC] Skoro bude — poky v rozrobtsi!\n", stdout);
                break;
            case 0:
                fputs("Povernennya do golovnoho menyu...\n", stdout);
                return;
            default:
                fprintf(stderr, "Neividomyi vybir lvl: %d\n", lvlChoice);
                break;
        }
    } while (lvlChoice != 0);
}

static void printMenu(const struct Adventurer *hero) {
    fputs("\n=== GOLOVNE MENYU: PUSHEEN'S RESCUE MISSION ===\n", stdout);
    printf("Heroj: %-20s | HP: %3d/%3d | Ochky: %05d\n",
           hero->name, hero->hpCurrent, hero->hpMax, hero->score);
    fputs("1. lvl\n", stdout);
    fputs("2. Pereglyanuty statystyku ta inventar\n", stdout);
    fputs("0. Vykhid z gry\n", stdout);
    fputs("Vash vybir + Enter: ", stdout);
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    char heroName[NAME_BUF_SIZE] = "Kotyachyy Lytsar";
    if (argc >= 2 && argv[1][0] != '\0') {
        snprintf(heroName, sizeof(heroName), "%s", argv[1]);
        trimNewlineOrSpaces(heroName);
    } else {
        fputs("Vvedy im'ya Kotyachoho Lytsarya (abo natysni Enter dlya standartnoho): ", stdout);
        char inputName[NAME_BUF_SIZE];
        if (readLine(inputName, sizeof(inputName))) {
            trimNewlineOrSpaces(inputName);
            if (inputName[0] != '\0') {
                snprintf(heroName, sizeof(heroName), "%s", inputName);
            }
        }
    }

    struct Adventurer *hero = createAdventurer(heroName);
    if (hero == NULL) {
        return EXIT_FAILURE;
    }

    addItemToInventory(hero, "Zolotyy Tapok", "Legendarna zbroya blyzhn'oho boyu.");
    addItemToInventory(hero, "Zirka Pryntsesy", "Symvol poryatunku Pushyn.");

    char line[INPUT_BUF_SIZE];
    int menuChoice = -1;

    do {
        printMenu(hero);
        if (!readLine(line, sizeof(line))) {
            fprintf(stderr, "\nKinec' potoku vvedennya (EOF).\n");
            break;
        }

        if (sscanf(line, "%d", &menuChoice) != 1) {
            fprintf(stderr, "Nevirnyy vvid '%s'. Vvedy chyslo (0, 1 abo 2).\n", line);
            continue;
        }

        switch (menuChoice) {
            case 1:
                openLvlSubmenu(hero);
                break;
            case 2:
                fputs("\n--- STATYSTYKA TA INVENTAR ---\n", stdout);
                printf("Im'ya:     %-25s\n", hero->name);
                printf("Zdorov'ya: %3d / %3d HP\n", hero->hpCurrent, hero->hpMax);
                printf("Syla:      %3d | Monety: %3d | Ochky: %05d\n",
                       hero->strength, hero->coins, hero->score);
                printf("Inventar (%d predmety/iv):\n", hero->invCount);
                for (int i = 0; i < hero->invCount; i++) {
                    printf("  [%d] %-18s — %s\n",
                           i + 1, hero->inventory[i].name, hero->inventory[i].description);
                }
                break;
            case 0:
                fputs("Dyakuyu za гру! Do novykh prygod z Pushyn!\n", stdout);
                break;
            default:
                fprintf(stderr, "Neividomyi punkt menyu: %d\n", menuChoice);
                break;
        }
    } while (menuChoice != 0);

    freeAdventurer(hero);
    return EXIT_SUCCESS;
}