/* A：把输入先检查好，其他部分就不用重复处理了。 */
#include <stdio.h>
#include "project.h"

/* 一次读一行。太长的部分也要读掉，不然会影响下一次输入。 */
int readText(const char prompt[], char text[], int size)
{
    int ch, len, tooLong, badChar, readSomething, hasCR;

    if (prompt == NULL || text == NULL || size < 1) {
        return 0;
    }
    while (1) {
        len = 0;
        tooLong = 0;
        badChar = 0;
        readSomething = 0;
        hasCR = 0;
        printf("%s", prompt);
        fflush(stdout);
        text[0] = '\0';

        /* ch 用 int，除了普通字符，还要接收 EOF。 */
        while ((ch = getchar()) != '\n' && ch != EOF) {
            readSomething = 1;
            if (hasCR) {
                badChar = 1;       /* 回车后又有别的字符，这一行不接受。 */
            }
            if (ch == '\r') {
                hasCR = 1;        /* Windows 行尾可能先有回车，再有换行。 */
                continue;
            }
            if (ch < 32 || ch == 127) {
                badChar = 1;       /* 这些不是要输入的普通文字，不接受。 */
            }
            if (len < size - 1) {
                text[len++] = (char)ch;
            } else {
                tooLong = 1;
            }
        }
        text[len] = '\0';
        if (ferror(stdin)) {
            text[0] = '\0';
            return 0;
        }
        if (ch == EOF && !readSomething) {
            return 0;
        }
        if (tooLong || badChar) {
            text[0] = '\0';
            printf("Enter at most %d characters without control characters.\n", size - 1);
            continue;
        }
        return 1;
    }
}

/* 把数字字符串转成整数。全部检查成功后才改 result。 */
int parseNumber(const char text[], int min, int max, int *result)
{
    int i, digit, number = 0;

    if (text == NULL || result == NULL || min < 0 || min > max) {
        return 0;
    }
    if (text[0] == '\0') {
        return 0;
    }
    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] < '0' || text[i] > '9') {
            return 0;
        }
        digit = text[i] - '0';
        /* 先判断下一步会不会超过 max，不能乘完 10 再检查。 */
        if (number > max / 10 || (number == max / 10 && digit > max % 10)) {
            return 0;
        }
        number = number * 10 + digit;
    }
    if (number < min) {
        return 0;
    }
    *result = number;             /* 用地址把结果带回去。 */
    return 1;
}

int readInt(const char prompt[], int min, int max)
{
    char text[100];
    int number;

    if (min < 0 || min > max) {
        printf("Invalid input range.\n");
        return -1;
    }
    while (1) {
        if (!readText(prompt, text, sizeof(text))) {
            return -1;
        }
        if (parseNumber(text, min, max, &number)) {
            return number;
        }
        printf("Enter digits only, from %d to %d (no signs or spaces).\n", min, max);
    }
}
