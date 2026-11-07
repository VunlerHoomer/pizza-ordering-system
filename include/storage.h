#ifndef PIZZA_STORAGE_H
#define PIZZA_STORAGE_H

#include "types.h"

/* Temporary integration entry point; business API is TODO. */
AppStatus storage_demo(void);

/* Member A integration contract; implementation owned by E. */
/* Failed load must never be followed by save. Save must support retry. */
AppStatus storage_load(void);
AppStatus storage_save(void);

#endif
