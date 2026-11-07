/* Member A: bounded line input and strict, reusable value validation. */
#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *skip_space(const char *text)
{
    while (isspace((unsigned char)*text)) { ++text; }
    return text;
}

int input_parse_int(const char *text, int min, int max, int *out)
{
    char *end;
    long value;
    if (text == NULL || out == NULL || min > max) { return 0; }
    errno = 0;
    value = strtol(text, &end, 10);
    if (end == text || errno == ERANGE || *skip_space(end) != '\0' ||
        value < min || value > max) { return 0; }
    *out = (int)value;
    return 1;
}

int input_parse_money(const char *text, Money min, Money max, Money *out)
{
    Money whole = 0;
    Money fraction = 0;
    int digits = 0;
    if (text == NULL || out == NULL || min < 0 || min > max) { return 0; }
    text = skip_space(text);
    if (*text < '0' || *text > '9') { return 0; }
    while (*text >= '0' && *text <= '9') {
        int digit = *text++ - '0';
        if (whole > (INT64_MAX - digit) / 10) { return 0; }
        whole = whole * 10 + digit;
    }
    if (*text == '.') {
        ++text;
        while (*text >= '0' && *text <= '9') {
            if (++digits > 2) { return 0; }
            fraction = fraction * 10 + (*text++ - '0');
        }
        if (digits == 0) { return 0; }
        if (digits == 1) { fraction *= 10; }
    }
    if (*skip_space(text) != '\0' || whole > (INT64_MAX - fraction) / 100) {
        return 0;
    }
    whole = whole * 100 + fraction;
    if (whole < min || whole > max) { return 0; }
    *out = whole;
    return 1;
}

InputResult input_read_text(const char *prompt, char *out, size_t capacity,
                           int allow_empty)
{
    if (prompt == NULL || out == NULL || capacity < 2) { return INPUT_ERROR; }
    for (;;) {
        size_t length = 0;
        int invalid = 0;
        int saw_cr = 0;
        int ch;
        out[0] = '\0';
        fputs(prompt, stdout);
        fflush(stdout);
        /* Always consume the whole line, even after overflow or a NUL byte. */
        while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
            if (saw_cr) { invalid = 1; }
            if (ch == '\r') { saw_cr = 1; continue; }
            if (ch == '\0' || iscntrl((unsigned char)ch)) {
                invalid = 1;
            }
            if (length < capacity - 1) { out[length++] = (char)ch; }
            else { invalid = 1; }
        }
        if (ferror(stdin)) { out[0] = '\0'; return INPUT_ERROR; }
        if (ch == EOF && length == 0 && !invalid) { return INPUT_END; }
        out[length] = '\0';
        if (invalid) {
            out[0] = '\0';
            puts("Input is too long or contains unsupported control characters.");
            continue;
        }
        if (!allow_empty && *skip_space(out) == '\0') {
            out[0] = '\0';
            puts("This field cannot be empty.");
            continue;
        }
        return INPUT_OK;
    }
}

InputResult input_read_int(const char *prompt, int min, int max, int *out)
{
    char line[128];
    InputResult result;
    if (out == NULL || min > max) { return INPUT_ERROR; }
    for (;;) {
        result = input_read_text(prompt, line, sizeof line, 1);
        if (result != INPUT_OK) { return result; }
        if (input_parse_int(line, min, max, out)) { return INPUT_OK; }
        printf("Enter a whole number between %d and %d.\n", min, max);
    }
}

InputResult input_read_money(const char *prompt, Money min, Money max, Money *out)
{
    char line[128];
    InputResult result;
    if (out == NULL || min < 0 || min > max) { return INPUT_ERROR; }
    for (;;) {
        result = input_read_text(prompt, line, sizeof line, 1);
        if (result != INPUT_OK) { return result; }
        if (input_parse_money(line, min, max, out)) { return INPUT_OK; }
        puts("Enter an amount within the allowed range, with at most two decimal places.");
    }
}

InputResult input_confirm(const char *prompt, int *confirmed)
{
    char line[16];
    InputResult result;
    if (confirmed == NULL) { return INPUT_ERROR; }
    for (;;) {
        const char *answer;
        result = input_read_text(prompt, line, sizeof line, 1);
        if (result != INPUT_OK) { return result; }
        answer = skip_space(line);
        if ((*answer == 'y' || *answer == 'Y' || *answer == 'n' || *answer == 'N') &&
            *skip_space(answer + 1) == '\0') {
            *confirmed = (*answer == 'y' || *answer == 'Y');
            return INPUT_OK;
        }
        puts("Enter y or n.");
    }
}

int read_menu_choice(int min, int max, int *choice)
{
    return input_read_int("Choice: ", min, max, choice) == INPUT_OK;
}
