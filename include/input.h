#ifndef PIZZA_INPUT_H
#define PIZZA_INPUT_H

/* Reads one integer in [min, max]. Returns 1 on success, 0 on EOF/error. */
int read_menu_choice(int min, int max, int *choice);

#endif
