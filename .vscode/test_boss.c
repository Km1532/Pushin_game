#include "hero.h"
#include "io_utils.h"
#include "map_combat.h"
#include "types.h"
#include "world_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned int)time(NULL));

    struct Adventurer *hero = createAdventurer("Boss Tester");
    if (hero == NULL) {
        fprintf(stderr, "Failed to create test hero.\n");
        return EXIT_FAILURE;
    }

    /* Set conditions that unlock the Boss fight immediately */
    hero->score = BOSS_UNLOCK_SCORE;
    hero->pusheenLevel = BOSS_UNLOCK_PUSHEEN_LVL;
    hero->coins = 300;
    hero->strength = 35;
    hero->defense = 10;
    hero->hpMax = 120;
    hero->hpCurrent = hero->hpMax;

    printf("Starting instant Boss fight test (Score: %d, Pusheen LVL: %d)...\n",
           hero->score, hero->pusheenLevel);

    int result = playRescueLevel(hero, 2);
    printf("Boss battle finished with status: %d | Final score: %d | Coins: %d\n",
           result, hero->score, hero->coins);

    freeAdventurer(hero);
    return EXIT_SUCCESS;
}