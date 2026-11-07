#ifndef PIZZA_ORDER_H
#define PIZZA_ORDER_H

#include "types.h"

/* Temporary integration entry point; business API is TODO. */
AppStatus order_demo(void);

/* Member A integration contract; implementation owned by D. */
AppStatus order_place(CustomerId customer_id);
AppStatus order_history(CustomerId customer_id);

#endif
