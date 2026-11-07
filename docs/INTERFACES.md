# 接口约定 v0.2：A 的模块接入点

这些接口是 A 为多人联调提供的起点。B–E 替换自身文件中的 TODO 实现；修改签名时先通知调用方。
`*_demo()` 是上一版遗留占位，不再被主程序使用，可在各自接入时删除。

## 共同类型与状态

- `Money`：`int64_t`，单位为分。C、D 均不得用浮点金额替换。
- `Session`：`role` 与 `account_id`；`ROLE_NONE/0` 为退出状态。
- 当前控制台账号范围是 `1..INT_MAX`；0 用于取消。B 应按此范围发放编号。
- `APP_OK` 仅在操作确实完成时返回；未实现返回 `APP_NOT_IMPLEMENTED`。
- 登录失败返回 `APP_AUTH_FAILED`；越权返回 `APP_FORBIDDEN`；取消返回 `APP_CANCELLED`。
- I/O 失败返回 `APP_IO_ERROR`；数据损坏返回 `APP_DATA_ERROR`。
- 数据记录 `Customer`、`Order` 和文件格式仍由 B/D/E 商定，本次没有虚构业务结构。

## B：账户

```c
AppStatus account_authenticate(UserRole role, unsigned int account_id,
                               const char *password, Session *out_session);
AppStatus account_register(void);
AppStatus account_view_profile(CustomerId customer_id);
AppStatus account_edit_profile(CustomerId customer_id);
AppStatus account_manage_customers(AccountManagementAction action);
```

`role` 是用户选择的登录入口，不是已验证权限。B 必须核对账号、密码、账户状态和实际角色。
只有全部通过才填入真实的身份并返回 `APP_OK`；失败应清空 `out_session`。
A 额外验证返回身份与所请求的账号/角色一致，然后才保存会话。
密码当前最大 64 字节；普通控制台输入会回显，实际凭据存储由 B 设计。

后三个资料操作仅操作指定顾客，管理操作为 `ACCOUNT_ADD/REMOVE/EDIT`。
资料编辑/注册/店长管理的具体字段输入由 B 的处理函数负责，完成后返回 A 菜单。

## C：菜单与价格

```c
AppStatus menu_show(void);
AppStatus menu_manage(void);
AppStatus pricing_manage(void);
AppStatus pricing_quote(int crust_id, int size_id, int topping_id, Money *out_price);
```

管理函数负责自身编辑子流程。报价函数供 D 调用，成功才写入 `out_price`，不读取终端、不扣款。

## D：订单与充值

```c
AppStatus order_place(CustomerId customer_id);
AppStatus order_history(CustomerId customer_id);
AppStatus wallet_deposit(CustomerId customer_id);
```

A 将当前登录顾客的编号直接传入，D 不应再次询问“要操作谁的账号”。
选披萨、数量、确认、充值金额等子流程由 D 负责，可复用 `input.h`。
余额与订单必须在操作成功时一致地保存；A 的退出保存不能替代每次交易的保存保证。

## E：存储与统计

```c
AppStatus storage_load(void);
AppStatus storage_save(void);
AppStatus stats_show(void);
```

正常启动加载一次。加载 I/O 错误或数据损坏时，A 中止启动且不调用保存，防止覆盖旧数据。
在当前未实现存储的开发阶段，`APP_NOT_IMPLEMENTED` 会显示明确提示后开放菜单。
其他模块接入前，E 应先完成可用存储；该开发回退不代表可持久化运行。
退出时（包括 EOF）仅在加载成功后调用保存；失败时允许重试，选择放弃或遇到 EOF 则以退出码 1 结束。
保存应可重试，不应在失败时破坏最后一份有效数据。内部记录组织与事务保存由 E 和 B/D 协调。

## A：输入与权限分发

- `input_read_text`：完整读取一行，最大 `capacity - 1` 字节；过长或包含控制字符时整行丢弃并重试。
- `input_read_int`：完整匹配整数与上下界；拒绝 `1abc`、小数、溢出等。
- `input_read_money`：接受 `12`、`12.3`、`12.30`，分别为 1200、1230、1230 分；拒绝负号、指数和超过两位小数。
- `input_confirm`：接受 y/n（大小写均可）；其他输入重试。
- `INPUT_OK`：成功；`INPUT_END`：EOF；`INPUT_ERROR`：参数/读取错误。调用方必须检查返回值，避免 EOF 无限循环。
- 子模块遇到 EOF 可返回 `APP_CANCELLED`，主菜单随后读取到 EOF 并退出。
- `app_dispatch`：先验证会话和操作所属角色，再调用模块；拒绝时不会进入业务函数。

顾客可调用：浏览、下单、历史、充值、查看/编辑自己资料。
店长可调用：菜单编辑、改价、添加/删除/编辑顾客、销售统计。
业务函数也应校验目标账户有效性、余额及业务约束；A 的分发不是这些检查的替代。
