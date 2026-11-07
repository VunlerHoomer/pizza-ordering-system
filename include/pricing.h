#ifndef PIZZA_PRICING_H
#define PIZZA_PRICING_H

#include "types.h"

/* Proposed API: writes a price only on APP_OK; currently NOT_IMPLEMENTED. */
AppStatus pricing_quote(int crust_id, int size_id, int topping_id, Money *out_price);

#endif
