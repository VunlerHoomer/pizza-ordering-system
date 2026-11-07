#ifndef PROJECT_H
#define PROJECT_H

#define CUSTOMER_ROLE 0
#define MANAGER_ROLE 1

/* A：输入函数。readInt 返回 -1 表示输入结束，范围只用于非负整数。 */
int readText(const char prompt[], char text[], int size);
int readInt(const char prompt[], int min, int max);

/* B：账号。checkLogin 核对账号、密码和角色，成功返回 1，失败返回 0。 */
int checkLogin(int accountId, const char password[], int role);
void registerCustomer(void);
void showCustomer(int customerId);
void editCustomer(int customerId);
void manageCustomer(int operation);  /* 1 添加，2 删除，3 编辑 */

/* C：菜单和价格。金额用整数分；calculatePrice 返回 -1 表示无法报价。 */
void showPizzaMenu(void);
void editPizzaMenu(void);
void editPrices(void);
int calculatePrice(int type, int size, int topping);

/* D：下单、历史和充值。传入的编号是当前登录顾客。 */
void placeOrder(int customerId);
void showOrderHistory(int customerId);
void depositMoney(int customerId);

/* E：文件和统计。loadData：1 成功，0 出错，-1 尚未实现。 */
int loadData(void);
int saveData(void);                 /* 1 保存成功，0 保存失败 */
void showStatistics(void);

#endif
