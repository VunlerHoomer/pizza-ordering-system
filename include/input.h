#ifndef PIZZA_INPUT_H
#define PIZZA_INPUT_H

#include <stddef.h>
#include "types.h"

typedef enum {
    INPUT_ERROR = -1,
    INPUT_END = 0,
    INPUT_OK = 1
} InputResult;

/* Pure parsers: return 1 on success; leave output unchanged on failure. */
int input_parse_int(const char *text, int min, int max, int *out);
/* Decimal currency only: digits, optionally '.' and one or two digits. */
int input_parse_money(const char *text, Money min, Money max, Money *out);

/* Retry invalid input. EOF and stream errors return to the caller. */
InputResult input_read_text(const char *prompt, char *out, size_t capacity,
                           int allow_empty);
InputResult input_read_int(const char *prompt, int min, int max, int *out);
InputResult input_read_money(const char *prompt, Money min, Money max, Money *out);
InputResult input_confirm(const char *prompt, int *confirmed);

/* Compatibility wrapper: returns 1 on success, 0 on EOF/error. */
int read_menu_choice(int min, int max, int *choice);

#endif
