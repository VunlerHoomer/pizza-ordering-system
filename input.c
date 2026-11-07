/* 成员 A：使用字符、数组和循环处理输入。 */
#include <stdio.h>
#include "project.h"

/* 读完一整行，即使超长也不把剩余字符留给下一次输入。 */
int readText(const char prompt[], char text[], int size)
{
    int ch, length, tooLong, invalid, hasInput, hasCR;

    if (prompt == NULL || text == NULL || size < 1) {
        return 0;
    }
    while (1) {
        length = 0;
        tooLong = 0;
        invalid = 0;
        hasInput = 0;
        hasCR = 0;
        printf("%s", prompt);
        fflush(stdout);
        text[0] = '\0';

        /* getchar 返回 int，才能同时表示字符和 EOF。 */
        while ((ch = getchar()) != '\n' && ch != EOF) {
            hasInput = 1;
            if (hasCR) {
                invalid = 1;       /* 回车只能出现在行末。 */
            }
            if (ch == '\r') {
                hasCR = 1;        /* 兼容 Windows 的 CRLF 行尾。 */
                continue;
            }
            if (ch < 32 || ch == 127) {
                invalid = 1;       /* 拒绝字符串中的控制字符。 */
            }
            if (length < size - 1) {
                text[length++] = (char)ch;
            } else {
                tooLong = 1;
            }
        }
        text[length] = '\0';
        if (ferror(stdin)) {
            text[0] = '\0';
            return 0;
        }
        if (ch == EOF && !hasInput) {
            return 0;
        }
        if (tooLong || invalid) {
            text[0] = '\0';
            printf("Enter at most %d characters without control characters.\n", size - 1);
            continue;
        }
        return 1;
    }
}

/* 成功才写回结果；在乘 10 之前检查上界，避免整数溢出。 */
int parseNumber(const char text[], int min, int max, int *result)
{
    int i, digit, value = 0;

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
        if (value > max / 10 || (value == max / 10 && digit > max % 10)) {
            return 0;
        }
        value = value * 10 + digit;
    }
    if (value < min) {
        return 0;
    }
    *result = value;
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
