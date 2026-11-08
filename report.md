# Pizza Ordering System — Report Draft

Group members: 待填写五位成员的姓名和学号。

这是全组报告的草稿。已写的功能目标不代表这些功能已经全部实现。
完成各模块后再补充实际设计、截图和测试结果，最后导出 PDF。

实施依据见 [PRD.md](PRD.md)。A 的接口、输入和失败退出已按 PRD 修正；真实账户、订单及文件业务仍待 B–E 完成。

## 1. Problem Specification（A）

A pizza shop needs a program for an in-store ordering terminal. Customers should be able to
register an account, log in, choose a pizza and place an order. A pizza can have different
types, sizes and toppings, and its price depends on these choices.

Each customer has a unique account number, a name, a telephone number, a balance and an
order history. Customers can deposit money and review their previous orders. The shop manager
can edit the pizza menu, change prices, manage customer information and view sales statistics.

We plan to use a console interface. The program has a main menu, a customer menu and a manager
menu. Users must pass the account check before entering the menu for their role. Invalid
menu input should produce a message and allow the user to try again.

The completed project should save the necessary information in files so that it is still
available after restarting the program. The final submission includes the C source files,
a Windows executable, required data files, this report, an operating manual and the signed
contribution form.

## 2. Analysis（B 主笔，其他人补充）

Customer、MenuOption、Order、ShopData 及容量已在 project.h 定义。B–E 需结合实际实现补充各字段含义、定价、余额处理和文件格式。
金额单位暂定为分。账号范围暂定为 1–999999。
这些是小组的设计选择，不是老师规定的唯一做法。

## 3. Algorithm Design（C 主笔，A/D 补充）

A 的程序流程：

1. main 创建一份 ShopData，调用 `loadData(&data)`；只有返回 1 才进入菜单，否则退出且不保存。
2. 显示首页，读取用户选择。
3. 用户选择登录时，输入账号和密码，调用 B 的 `checkLogin()`。
4. 登录成功后进入对应菜单，用 `switch` 调用各成员的函数。
5. 修改函数返回 -1 时，菜单逐层退出，main 报错并结束，不调用额外保存。
6. 退出登录时返回首页。正常退出或 EOF 时只调用一次 saveData；失败后报错退出，不重试覆盖备份。

例如一次下单的调用关系为：

`main → login → customerMenu → placeOrder`

`customerMenu` 把 data 指针和当前顾客编号传给 `placeOrder`。各模块访问同一份数据，订单模块不重新询问要使用谁的账户。
菜单使用循环，所以操作结束后可以继续选择，不需要反复调用 `main()`。

`readText()` 用 getchar 读完整行，检查容量和控制字符，兼容 LF/CRLF，并正确处理 EOF。
`readInt()` 调用 parseNumber，逐个字符检查 '0'–'9'，先检查上界再乘 10 累加，避免溢出。
空串、+1、-0、空格、1abc 和小数都不能作为数字输入。parseNumber 失败时不会修改输出参数。

待 C、D、E 补充：价格计算、下单和扣款步骤、文件读写方法、统计方法。

## 4. Testing（E 汇总，每人测试自己部分）

2026-11-07 修正后，在 Mac 上以 -std=c11 -Wall -Wextra -Wpedantic -Werror 编译通过。
完成数字边界断言、共享数据指针一致性检查，以及 38 次程序运行检查。
为单独验证 A，临时替换了 B–E 的主要业务函数；正常源代码依旧不提供假登录或假保存成功。
保存失败检查在临时目录实际生成了截断文件，确认 A 不重复保存、原有效备份得到保留；这不代表 E 的真实存储已完成。
临时 C 文件和数据已清理，没有给仓库增加测试框架。

| 测试内容 | 预期结果 | 实际结果 |
|---|---|---|
| 编译全部六个 C 文件 | 编译成功 | 通过，无编译警告 |
| 空串、符号、空格、字母、小数、越界整数 | 拒绝；失败时不修改转换结果 | 通过 |
| 输入太长后再输入正常选项 | 不影响下一次输入 | 通过 |
| 登录验证失败 | 不进入角色菜单 | 通过 |
| 顾客、店长各自的菜单 | 调用正确函数 | 临时模块检查通过 |
| 退出登录后再次进入 | 必须重新验证 | 临时模块检查通过 |
| 文件读取失败、旧 -1 占位值 | 不进入菜单、不保存 | 临时模块检查通过；当前正常程序因加载未实现而停止 |
| 注册、下单、充值、改资料及 5 项店长修改的保存失败 | 共 9 条路径立即退出，无第二次保存 | 临时模块检查通过，原备份保留 |
| 正常退出时保存失败 | 仅调用一次，报错结束 | 临时模块检查通过 |
| 各模块的数据指针和顾客编号 | 与 main 的数据一致，顾客编号来自登录 | 临时模块检查通过 |
| parseNumber 上界与输出参数 | INT_MAX 合法；超限拒绝且不修改输出 | 通过 |
| 注册、真实下单、充值和统计 | 业务结果正确 | 尚未实现 |
| Windows 可执行文件 | 能在 Windows 运行 | 待实机检查 |

### C：菜单与价格（member-c-menu，2026-11-08）

实现了 `pizza.c` 的菜单显示、菜单编辑（添加/改名/停用）、改价和报价。E 的 `loadData()` 尚未实现，因此本次用**临时测试程序**（提供可切换成功/失败的 `saveData()` 桩函数，未提交到仓库）单独检查 C 的逻辑，属于“仅模块检查，待真实联调”。

| 测试内容 | 预期结果 | 实际结果 |
|---|---|---|
| 报价：饼底 2000 + 大号 1200 + 配料 500 | 返回 3700 分 | 通过 |
| 饼底 2000 + 小号 0 + 火腿 700 | 返回 2700 分 | 通过 |
| 不存在的饼底/尺寸/配料编号、编号 0、data 为 NULL | 一律返回 -1 | 通过 |
| 停用的配料参与报价 | 返回 -1，不出价 | 通过 |
| 价格被写成负数、菜单计数被写成 11 | 返回 -1，不返回错误金额 | 通过 |
| 添加选项 | 编号取“现有最大编号 + 1”，保存成功返回 1，计数 +1 | 通过 |
| 停用后再添加 | 新编号跳过已停用编号（3 停用后新选项为 4），历史订单不指向别的选项 | 通过 |
| 列表已满（10 个）时添加 | 拒绝并提示，计数保持 10，不覆盖旧数据 | 通过 |
| 存储失败时添加/改名/停用/改价 | 分别返回 -1，并恢复计数、名称、启用状态和价格 | 通过 |
| 改名成同名、空名称取消、编号不存在 | 返回 0，名称不变，不产生多余保存 | 通过 |
| 重复停用同一选项 | 返回 0，提示已停用 | 通过 |
| 改价成相同价格 | 返回 0，不写文件 | 通过 |
| 价格边界 999999 分、超出上限 1000000 分 | 接受上限；超限被拒绝后重新输入 | 通过 |
| 名称超过 31 字符 | 拒绝该行并重新提示，下一行正常输入 | 通过 |
| 顾客菜单显示 | 只显示启用选项并标注停用项 | 通过 |
| 六个 C 文件一起编译（-Wall -Wextra -Wpedantic -Werror） | 无警告、无错误 | 通过 |
| 菜单中的改价、菜单编辑被店长菜单调用 | 参数和返回值与 `project.h` 一致 | 临时模块检查通过 |

`AI 提出的问题 / 本人复核结论`：AI 自查确认了编号复用、保存失败回退、价格上限和数组越界四类问题，均已按上述实现修复；临时桩函数只用于模块检查，真实登录与文件保存仍需与 B、E 联调后复测。

最终需增加真实数据的测试，列出用于老师验收的账号和密码。

## 提交前检查

报告封面包含全员姓名、学号。确认源码、Windows `.exe`、数据文件、操作说明和签名贡献表齐全。
README 中的运行说明可以整理为最终 manual。由组长压缩为一个文件后提交。
