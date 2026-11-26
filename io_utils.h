#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stddef.h>

int readLine(char *buffer, size_t size);
void waitForEnter(const char *prompt);
void trimNewlineOrSpaces(char *str);
void sanitizeToken(char *str);
void printLevelHearts(int currentHearts, int maxHearts);
void printHpBar(int currentHp, int maxHp);

#endif