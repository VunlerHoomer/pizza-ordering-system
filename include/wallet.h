#ifndef PIZZA_WALLET_H
#define PIZZA_WALLET_H

#include "types.h"

/* Temporary integration entry point; business API is TODO. */
AppStatus wallet_demo(void);

/* Member A integration contract; implementation owned by D. */
/* Handler may use input_read_money/input_confirm; D owns balance validation. */
AppStatus wallet_deposit(CustomerId customer_id);

#endif
