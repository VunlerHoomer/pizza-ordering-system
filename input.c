/* 成员 A：大家都可以使用这两个输入函数。 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "project.h"

/* 先读一整行，再检查长度，防止多余字符影响下一次输入。 */
int readText(const char prompt[], char text[], int size)
{
    char line[256];
    int length;
    int ch;

    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        text[0] = '\0';
        if (fgets(line, sizeof(line), stdin) == NULL) {
            return 0;
        }

        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            while ((ch = getchar()) != '\n' && ch != EOF) {
                /* 丢弃这一行剩余的字符。 */
            }
            printf("Input is too long. Please try again.\n");
            continue;
        }

        length = (int)strlen(line);
        while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            line[--length] = '\0';
        }
        if (length >= size) {
            printf("Please enter at most %d characters.\n", size - 1);
            continue;
        }
        strcpy(text, line);
        return 1;
    }
}

/* strtol 可以检查转换是否成功；1abc、1.5 都不能作为菜单选项。 */
int readInt(const char prompt[], int min, int max)
{
    char text[100];
    char *end;
    long number;

    while (1) {
        if (!readText(prompt, text, sizeof(text))) {
            return -1;
        }
        errno = 0;
        number = strtol(text, &end, 10);
        if (end != text && errno != ERANGE) {
            while (isspace((unsigned char)*end)) {
                end++;
            }
            if (*end == '\0' && number >= min && number <= max) {
                return (int)number;
            }
        }
        printf("Please enter a whole number from %d to %d.\n", min, max);
    }
}
