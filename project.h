#ifndef PROJECT_H
#define PROJECT_H

#define CUSTOMER_ROLE 0
#define MANAGER_ROLE 1
#define MAX_CUSTOMERS 100
#define MAX_OPTIONS 10
#define MAX_ORDERS 500

struct Customer {
    int id;
    char name[50];
    char phone[20];
    char password[32];
    int balance;                 /* 分 */
    int active;                  /* 1 可用，0 停用 */
};

struct MenuOption {
    int id;
    char name[32];
    int price;                   /* 分 */
    int active;
};

struct Order {
    int id;
    int customerId;
    int typeId;
    int sizeId;
    int toppingId;
    char typeName[32];            /* 成交时的名称 */
    char sizeName[32];
    char toppingName[32];
    int quantity;
    int unitPrice;
    int total;
};

/* main 只创建一份数据，其他函数通过参数访问它。 */
struct ShopData {
    struct Customer customers[MAX_CUSTOMERS];
    int customerCount;
    struct MenuOption types[MAX_OPTIONS];
    int typeCount;
    struct MenuOption sizes[MAX_OPTIONS];
    int sizeCount;
    struct MenuOption toppings[MAX_OPTIONS];
    int toppingCount;
    struct Order orders[MAX_ORDERS];
    int orderCount;
};

/* A：readText 返回 1 成功、0 输入结束/出错；readInt 返回 -1 表示结束。 */
int readText(const char prompt[], char text[], int size);
int readInt(const char prompt[], int min, int max);
/* 只接受非负十进制数字串；成功返回 1，失败返回 0 且不改变 result。 */
int parseNumber(const char text[], int min, int max, int *result);

/* 修改数据的操作：1 已保存，0 无改动/取消，-1 保存失败，必须退出。 */
/* B：登录返回 1 成功、0 失败；查找返回下标或 -1。 */
int checkLogin(const struct ShopData *data, int accountId,
               const char password[], int role);
int findCustomerIndex(const struct ShopData *data, int customerId);
int registerCustomer(struct ShopData *data);
void showCustomer(const struct ShopData *data, int customerId);
int editCustomer(struct ShopData *data, int customerId);
int manageCustomer(struct ShopData *data, int operation); /* 1 添加，2 停用，3 编辑 */

/* C：选项查找返回下标或 -1；报价返回分，无法报价返回 -1。 */
void showPizzaMenu(const struct ShopData *data);
int findOptionIndex(const struct MenuOption options[], int count, int id);
int editPizzaMenu(struct ShopData *data);
int editPrices(struct ShopData *data);
int calculatePrice(const struct ShopData *data, int type, int size, int topping);

/* D：使用当前顾客编号，不能重新指定另一名顾客。 */
int placeOrder(struct ShopData *data, int customerId);
void showOrderHistory(const struct ShopData *data, int customerId);
int depositMoney(struct ShopData *data, int customerId);

/* E：加载和保存都返回 1 成功、0 失败。失败后不得继续写入。 */
int loadData(struct ShopData *data);
int saveData(const struct ShopData *data);
void showStatistics(const struct ShopData *data);

#endif
