#include "io_utils.h"
#include <stdio.h>
#include <string.h>

int readLine(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return 1;
}

void waitForEnter(const char *prompt) {
    char dummy[128];
    fputs(prompt, stdout);
    (void)readLine(dummy, sizeof(dummy));
}

void trimNewlineOrSpaces(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ')) {
        str[len - 1] = '\0';
        len--;
    }
}

void sanitizeToken(char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ' || str[i] == '\n' || str[i] == '\r') {
            str[i] = '_';
        }
    }
}

void printLevelHearts(int currentHearts, int maxHearts) {
    if (currentHearts < 0) currentHearts = 0;
    if (currentHearts > maxHearts) currentHearts = maxHearts;

    putchar('[');
    for (int i = 0; i < maxHearts; i++) {
        fputs(i < currentHearts ? "<3" : "--", stdout);
        if (i < maxHearts - 1) putchar(' ');
    }
    printf("] (%d/%d)", currentHearts, maxHearts);
}

void printHpBar(int currentHp, int maxHp) {
    if (currentHp < 0) currentHp = 0;
    if (maxHp <= 0) maxHp = 100;

    int segments = 5;
    int filled = (currentHp * segments + maxHp - 1) / maxHp;
    if (currentHp > 0 && filled == 0) filled = 1;
    if (filled > segments) filled = segments;

    putchar('[');
    for (int i = 0; i < segments; i++) {
        fputs(i < filled ? "##" : "..", stdout);
        if (i < segments - 1) putchar(' ');
    }
    putchar(']');
}