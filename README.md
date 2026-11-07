# Pizza Ordering System

SOE103 2026–27 五人小组项目。语言：C11；界面：控制台。

**当前状态：可编译的协作骨架。** 已实现开发菜单、模块入口和基础输入校验。
注册、登录、披萨定价、下单、充值、文件保存、销售统计尚未实现；所有业务入口明确返回未实现。

## 在 VS Code 中开始

打开本文件所在的 `pizza-ordering-system` 文件夹。

- `Cmd+Shift+B`（Windows：`Ctrl+Shift+B`）：编译。
- 命令面板 → `Tasks: Run Task` → `Pizza: Run`：编译并运行。
- 推荐安装工作区提示的 Microsoft C/C++ 扩展；命令行编译不依赖它。

macOS / Linux 终端：

```sh
make
make run
```

需要 C 编译器和 Make。macOS 原生编译结果在 `build/pizza-ordering-system`，不是 Windows `.exe`。

Windows：安装 GCC（如 MinGW-w64）并加入 PATH，在 PowerShell 运行：

```powershell
./build.ps1
./build/pizza-ordering-system.exe
```

Windows 脚本和任务已配置，但仍需在 Windows 实机验证；最终提交必须包含经过验证的 `.exe`。

## 目录与分工

| 成员 | 主要文件 | 工作 |
|---|---|---|
| A | `src/main.c`、`ui.c`、`input.c` | 框架、输入、整合与构建 |
| B | `src/account.c` | 账户、登录、顾客管理 |
| C | `src/menu.c`、`pricing.c` | 菜单管理和价格计算 |
| D | `src/order.c`、`wallet.c` | 订单、充值、扣款、历史 |
| E | `src/storage.c`、`stats.c` | 文件存储和销售统计 |

各模块公开接口放在 `include/`，共同类型放在 `include/types.h`。
当前 `*_demo()` 仅用于验证模块可以一起编译；正式业务接口需共同商定。

## 协作与文档

- [五人分工与时间表](docs/TEAM_PLAN.md)
- [GitHub 同步操作](docs/COLLABORATION.md)
- [接口与数据约定](docs/INTERFACES.md)
- [报告模板](docs/REPORT_OUTLINE.md)
- [测试记录模板](tests/TEST_PLAN.md)

正式任务书截止时间：2026-12-13 23:59。公告截图年份有冲突，请向老师核实。
完整交付：PDF 报告、`.c/.h`、Windows `.exe`、所需数据、操作说明、全员签名贡献表。
