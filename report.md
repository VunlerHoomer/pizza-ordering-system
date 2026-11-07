# Pizza Ordering System — Report Draft

Group members: 待填写五位成员的姓名和学号。

这是全组报告的草稿。已写的功能目标不代表这些功能已经全部实现。
完成各模块后再补充实际设计、截图和测试结果，最后导出 PDF。

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

待补充：顾客、披萨、订单的结构体和数组；各字段含义；定价方法；余额不足如何处理；文件格式。
金额单位暂定为分。账号范围暂定为 1–999999。
这些是小组的设计选择，不是老师规定的唯一做法。

## 3. Algorithm Design（C 主笔，A/D 补充）

A 的程序流程：

1. 调用 `loadData()`；文件读取出错则退出，防止覆盖原文件。
2. 显示首页，读取用户选择。
3. 用户选择登录时，输入账号和密码，调用 B 的 `checkLogin()`。
4. 登录成功后进入对应菜单，用 `switch` 调用各成员的函数。
5. 退出登录时，菜单函数返回首页。退出程序时，加载成功的数据会调用 `saveData()` 保存。
6. 保存失败时提示重试或放弃；放弃时明确提示没有保存成功。

例如一次下单的调用关系为：

`main → login → customerMenu → placeOrder`

`customerMenu` 把当前顾客的编号传给 `placeOrder`，订单模块不再重新询问要使用谁的账户。
菜单使用循环，所以操作结束后可以继续选择，不需要反复调用 `main()`。

`readText()` 用 `fgets()` 读取一整行，检查是否太长。`readInt()` 再用 `strtol()` 转换成整数，
检查有没有多余字符、数字是否超出范围。这样可以拒绝 `abc`、`1abc` 和 `1.5` 等菜单输入。

待 C、D、E 补充：价格计算、下单和扣款步骤、文件读写方法、统计方法。

## 4. Testing（E 汇总，每人测试自己部分）

2026-11-07 在 Mac 上完成编译和 26 次运行检查。为单独检查 A 的菜单，开发时临时替换了尚未完成的 B–E 函数，测试后已删除这些临时文件。
这只说明 A 的调用流程通过检查，不代表真实登录、交易或文件读写已经实现。其他成员完成后继续增加测试行。

| 测试内容 | 预期结果 | 实际结果 |
|---|---|---|
| 编译全部六个 C 文件 | 编译成功 | 通过，无编译警告 |
| 首页输入字母、小数、越界整数 | 提示重新输入 | 通过 |
| 输入太长后再输入正常选项 | 不影响下一次输入 | 通过 |
| 登录验证失败 | 不进入角色菜单 | 通过 |
| 顾客、店长各自的菜单 | 调用正确函数 | 临时模块检查通过 |
| 退出登录后再次进入 | 必须重新验证 | 临时模块检查通过 |
| 文件读取失败 | 退出且不覆盖保存 | 临时模块检查通过 |
| 保存失败 | 可以重试或明确放弃 | 临时模块检查通过 |
| 注册、真实下单、充值和统计 | 业务结果正确 | 尚未实现 |
| Windows 可执行文件 | 能在 Windows 运行 | 待实机检查 |

最终需增加真实数据的测试，列出用于老师验收的账号和密码。

## 提交前检查

报告封面包含全员姓名、学号。确认源码、Windows `.exe`、数据文件、操作说明和签名贡献表齐全。
README 中的运行说明可以整理为最终 manual。由组长压缩为一个文件后提交。
