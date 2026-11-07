# Problem Specification — report draft contributed by Member A

Status: specification of the intended group product, not a claim that all features are implemented.
Source: SOE103 2026–27 Group Project brief, especially the customer specification on page 4.

## Problem and context

A new pizza shop requires a C program for an in-store ordering terminal. Customers need to select
pizzas, determine their prices, use prepaid accounts and review previous purchases. The shop
manager needs to maintain the menu, change prices, manage customer information and inspect sales.
The project must combine these functions in one usable program developed collaboratively by five members.

## Users and required functions

The customer service area must support account registration and login, pizza ordering, order-history
review and deposits. Each customer must have a unique account number, a name, a telephone number,
an available balance and an order history. The manager must be able to add, remove and edit customer
information. The program must offer different pizza types, sizes and toppings and calculate a price
from the selected combination. The examples of types and sizes in the brief guide the menu design;
the group must specify the actual menu and pricing rules.

Manager operations must include menu editing, setting or changing charges and providing statistics
related to pizza sales. The group must define which statistics are presented and what counts as a
completed sale. The proposed initial measures are order count, pizzas sold, total sales and a
breakdown by category. These measures are a group design proposal rather than an exhaustive list
specified by the assessment brief.

## Proposed scope and rules

The planned interface is a local console with a public entry menu and separate authenticated customer
and manager menus. This interface choice is made by the group; the brief does not demand a graphical
interface. Selecting a role must not by itself grant access: the account module must validate the
credentials and the account's actual role. A logged-in customer's actions use the identity stored
in the current session. Logging out clears that identity before returning to the public menu.

The proposed payment flow checks the prepaid balance before completing an order. Failed or cancelled
orders must not reduce the balance or count as completed sales. Each successful order must retain
its price at the time of purchase. Changing the current menu must not change historical order values.
Money will be represented as integer cents to avoid floating-point rounding in transaction calculations.

Persistent local storage is proposed so that accounts, balances, menus and orders survive a restart.
The team must define the file format and implement consistent saving of balance and order changes.
The application must report loading and saving failures; it must not overwrite invalid source data
by treating a failed load as an empty database. Online payment processing, delivery logistics and
networked multi-terminal operation are outside the planned scope.

## Quality and completion criteria

The program should recover from invalid menu choices, malformed numbers, overlong input and normal
end-of-input without an infinite loop or unexpected crash. Each module should be independently
testable, and integration tests must demonstrate complete customer and manager workflows. Tests
using simulated modules establish framework behaviour only; real account, pricing, transaction and
storage tests are needed before claiming completion of the group product.

The group will deliver appropriately commented C source and header files, a working Windows executable,
required data files, a PDF development-process report, an operating manual and a signed contribution
form. Final acceptance includes running the complete product in its intended Windows environment
and checking that the documentation describes the delivered behaviour accurately.
