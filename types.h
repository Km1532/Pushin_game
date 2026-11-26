#ifndef TYPES_H
#define TYPES_H

#define MAP_ROWS 10
#define MAP_COLS 20
#define MAX_ENEMIES 5
#define MAX_HEART_DROPS 8
#define LEVEL_MAX_HEARTS 3
#define MAX_HERO_LEVEL 5
#define SHOP_ITEMS_COUNT 15
#define BOSS_UNLOCK_SCORE 5000
#define BOSS_UNLOCK_PUSHEEN_LVL 2
#define INPUT_BUF_SIZE 128
#define NAME_BUF_SIZE 50
#define SAVE_FILE_NAME "savegame.txt"

struct Item {
    char *name;
    char *type;
    char *description;
    int price;
};

struct ShopCatalogEntry {
    const char *name;
    const char *type;
    const char *description;
    int price;
};

struct Adventurer {
    char *name;
    int hpCurrent;
    int hpMax;
    int strength;
    int defense;
    int coins;
    int score;
    int xp;
    int heroLevel;
    int pusheenLevel;
    int x;
    int y;

    struct Item *inventory;
    int invCount;
    int invCapacity;

    int defeatedEnemiesCount;
    int heartsCollectedCount;
    int quest1Done;
    int quest2Done;
    int quest3Done;
};

struct Enemy {
    int x;
    int y;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int isAlive;
    char name[32];
};

struct HeartDrop {
    int x;
    int y;
    int isActive;
};

#endif