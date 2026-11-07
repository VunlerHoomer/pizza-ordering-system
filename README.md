# Pizza Ordering System

SOE103 2026–27 五人小组项目，C11 控制台程序。

**当前版本：成员 A 框架已实现，B–E 业务功能待接入。**

已实现顾客/店长菜单、登录结果校验、会话与退出登录、12 个操作的角色校验及模块分发、
整数/文本/金额/确认输入、启动加载与退出保存流程、错误提示、VS Code 构建和回归测试。
真实注册/身份验证、菜单数据、订单/余额和存储/统计仍由 B–E 实现，返回明确的“暂未提供”状态。

## 在 VS Code 中使用

打开本项目文件夹：

- `Cmd+Shift+B`（Windows：`Ctrl+Shift+B`）：构建正常程序。
- 命令面板 → `Tasks: Run Task` → `Pizza: Run`：运行正常程序。
- `Pizza: Demo`：演示 A 的两种角色菜单，使用独立的测试替身。
- `Pizza: Test`：运行 A 的自动化测试（需要 Python 3）。

macOS / Linux（需要 C 编译器与 Make）：

```sh
make run
make demo
make test
```

正常程序：`build/pizza-ordering-system`。独立演示：`build/pizza-demo`。
**演示账户只在演示程序中生效，不保存数据，不产生真实订单或扣款。**

| 演示角色 | 账号 | 密码 |
|---|---|---|
| 顾客 | 1001 | customer-demo |
| 店长 | 9001 | manager-demo |

Windows（需要 GCC/MinGW-w64；测试另需 Python 3）：

```powershell
./build.ps1
./build/pizza-ordering-system.exe
./build.ps1 -Mode demo
./build/pizza-demo.exe
./build.ps1 -Mode test
```

Windows 脚本和任务已配置，仍需 Windows 实机验证。Mac 原生构建不是 Windows `.exe`。
最终提交的 `.exe` 应包含全组完成的真实业务功能，不能拿演示程序代替。

## 五人代码归属

| 成员 | 主要文件 | 工作 |
|---|---|---|
| A | `main.c`、`ui.c`、`app.c`、`input.c`、构建脚本 | 框架、身份状态、权限、输入、整合 |
| B | `account.c` | 注册、真实身份验证、顾客资料管理 |
| C | `menu.c`、`pricing.c` | 菜单和价格 |
| D | `order.c`、`wallet.c` | 订单、充值、扣款、历史 |
| E | `storage.c`、`stats.c` | 文件存储和销售统计 |

接口在 `include/`。A 已给 B–E 增加可编译的接入函数占位；这些函数没有业务实现。
`tests/demo_modules.c` 只能用于演示/测试，不属于实际业务实现。

## 文档

- [成员 A 交付范围与接入步骤](docs/MEMBER_A.md)
- [操作说明](docs/MANUAL.md)
- [接口与数据约定](docs/INTERFACES.md)
- [Problem Specification 报告草稿](docs/PROBLEM_SPECIFICATION.md)
- [系统结构及程序流程](docs/ARCHITECTURE.md)
- [A 的测试记录](tests/MEMBER_A_RESULTS.md)
- [五人分工与时间表](docs/TEAM_PLAN.md)
- [GitHub 同步操作](docs/COLLABORATION.md)
- [全组报告模板](docs/REPORT_OUTLINE.md)

正式任务书截止时间：2026-12-13 23:59；公告截图年份有冲突，请向老师核实。
最终交付包括 PDF 报告、源码、Windows `.exe`、数据、操作说明、全员签名贡献表。
