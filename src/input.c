/* Owner A: shared input validation for the console UI. */
#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_menu_choice(int min, int max, int *choice)
{
    char line[128];
    if (choice == NULL || min > max) {
        return 0;
    }
    for (;;) {
        char *end;
        long value;
        printf("Choice [%d-%d]: ", min, max);
        fflush(stdout);
        if (fgets(line, sizeof line, stdin) == NULL) {
            return 0;
        }
        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            puts("Input too long. Please try again.");
            continue;
        }
        errno = 0;
        value = strtol(line, &end, 10);
        if (end == line || errno == ERANGE) {
            puts("Please enter a valid whole number.");
            continue;
        }
        while (isspace((unsigned char)*end)) { ++end; }
        if (*end != '\0' || value < min || value > max) {
            puts("Invalid menu choice. Please try again.");
            continue;
        }
        *choice = (int)value;
        return 1;
    }
}
