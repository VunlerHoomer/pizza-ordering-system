#ifndef PIZZA_MENU_H
#define PIZZA_MENU_H

#include "types.h"

/* Temporary integration entry point; business API is TODO. */
AppStatus menu_demo(void);

/* Member A integration contract; implementation owned by C. */
AppStatus menu_show(void);
AppStatus menu_manage(void);

#endif
