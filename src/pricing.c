/* Owner C: pure price calculation, without I/O or balance changes. */
#include "pricing.h"
#include <stddef.h>

AppStatus pricing_quote(int crust_id, int size_id, int topping_id, Money *out_price)
{
    if (out_price == NULL) { return APP_INVALID_INPUT; }
    (void)crust_id;
    (void)size_id;
    (void)topping_id;
    return APP_NOT_IMPLEMENTED;
}
