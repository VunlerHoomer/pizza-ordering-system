# Pizza Ordering System

SOE103 小组作业，使用 C 语言制作一个披萨店点餐程序。

项目框架、五人分工和课内知识的使用方法见 [PRD](PRD.md)。提交功能前按 [代码自查规范](CODE_REVIEW.md) 检查。A 的框架和公共接口已完成，B–E 的主要业务仍待实现。

目前完成了成员 A 的主菜单、登录入口、顾客/店长菜单、输入检查、共享数据传递和失败退出。
其他成员的接口及两个循环查找辅助函数已准备好，带 TODO 的业务还需要各自完成。

**当前 E 的文件加载尚未实现，运行后会明确提示并退出，不会进入菜单。**
这是按 PRD 在没有有效数据时停止启动；E 完成加载并返回 1 后，才开放正常流程。

## 文件与分工

所有代码放在同一层目录，方便找到和一起编译。

| 成员 | 文件 | 负责内容 |
|---|---|---|
| A | `main.c`、`input.c` | 主菜单、角色菜单、输入检查、整合程序 |
| B | `account.c` | 注册、核对登录信息、查看/修改顾客资料 |
| C | `pizza.c` | 显示/编辑菜单、设置价格、计算价格 |
| D | `order.c` | 下单、充值、订单历史 |
| E | `file.c` | 读写文件、销售统计 |
| 共同 | `project.h` | 共享结构体、容量和函数声明 |
| 共同 | `report.md` | 报告草稿和测试记录 |
| 共同 | `PRD.md` | 项目框架、成员分工和课内知识的使用方法 |
| 共同 | `CODE_REVIEW.md` | 提交前自查、模块检查重点和审查记录 |

`.vscode/tasks.json` 只用来配置编译和运行，平时不用改。

## 编译运行

在 VS Code 中打开这个文件夹。Mac 按 Cmd+Shift+B，Windows 按 Ctrl+Shift+B 编译。
命令面板选择 `Tasks: Run Task` → `Run` 可以编译并运行。
需要电脑中已经安装 GCC（Mac 的 gcc 命令也可能实际使用 Clang）。

Mac/Linux 终端：

```sh
gcc -std=c11 -Wall main.c input.c account.c pizza.c order.c file.c -o pizza
./pizza
```

Windows 终端（需要安装 MinGW-w64/GCC 并加入 PATH）：

```powershell
gcc -std=c11 -Wall main.c input.c account.c pizza.c order.c file.c -o pizza.exe
./pizza.exe
```

本次已在 Mac 验证，Windows 仍需组员实机检查。提交作业时需要经过验证的 Windows `.exe`。

## 程序怎样连接

`main()` 创建一份 `struct ShopData data = {0};`，调用 `loadData(&data)`，加载成功后显示首页。登录调用 B 的 `checkLogin(&data, ...)`。
验证通过才进入顾客或店长菜单。菜单使用 `switch` 直接调用对应函数。
例如顾客选择下单时调用 `placeOrder(data, customerId)`，D 通过 data 访问同一份数据，用编号查找当前顾客。
选择退出登录后，菜单函数直接返回首页；下一次进入菜单必须重新登录。

当前 `checkLogin()` 也还没有真实验证功能，不能把占位返回值改成 1 来假装登录成功。
项目没有内置测试账户或独立演示版本，不会把未实现的功能当成成功。

## 组内先约定这些规则

- 账号范围为 1–999999，输入 0 取消登录；密码暂定最多 31 字节。
- `checkLogin()` 必须同时检查账号、密码和真实角色；通过才返回 1。
- `readInt()` 只接受范围内的十进制数字串；拒绝符号、空格、小数和尾随字符。返回 -1 表示输入结束或调用范围错误，应结束当前输入流程。
- `readText()` 使用 getchar 逐字符读完整行，兼容 LF/CRLF；1 表示成功，0 表示输入结束/出错。超长行全部丢弃再重试。
- `parseNumber()` 用字符检查和整数运算解析数字；1 成功、0 失败，失败不改变输出变量，供 E 读取文件时复用。
- Customer、MenuOption、Order、ShopData 已在 project.h 定义，使用固定大小的结构体数组；不要各自另建一份共享数据。
- 金额建议统一用整数分，例如 1050 表示 10.50 元。价格、余额和订单使用同一种单位。
- 订单要保留成交时的价格；改菜单价格不能改变旧订单。
- 充值和下单应及时保存，不能只依赖最后退出时保存。
- `loadData(data)` 返回 1 才能启动；0 表示失败。旧的 -1 占位状态已移除，当前未实现时返回 0。
- `saveData(data)` 只有真正保存成功才返回 1，失败返回 0，不重试。
- 会修改数据的操作统一返回：1 已保存、0 无改动/取消、-1 保存失败。返回 -1 前由所属模块恢复本次内存改动；A 立即退出，不做额外保存，以保留有效备份。

每个人主要改自己的 `.c` 文件。需要改 `project.h` 的函数参数时，先在组里说清楚。
不需要为自己的模块另外写一个正式 `main()`，所有 `.c` 一起编译。

## GitHub 协作与组长审查

**设置状态：仓库已公开，main 已启用强制 PR 和代码所有者审查；组员必须经 VunlerHoomer 批准才能合入。**

组员可以上传个人分支，但修改进入 main 前必须先交组长审查。全部文件的审查负责人配置为 `@VunlerHoomer`。

### 邀请组员

组长进入仓库 `Settings → Collaborators → Add people`，输入组员的 GitHub 用户名发送邀请。
组员接受后，用自己的 GitHub 账号克隆仓库。先启用下述保护，再授予组员写权限。

### 组员提交功能

开始新任务时，确保工作区没有未提交修改，再从最新 main 创建个人分支：

```sh
git switch main
git pull --ff-only
git switch -c member-b-account
# 修改代码，先按 CODE_REVIEW.md 用 AI 自查，再本人复核并实际测试
git add account.c
# 如有其他相关修改，也逐个加入暂存区
git commit -m "Implement customer registration"
git push -u origin HEAD
```

不同成员/任务使用不同分支名，例如 `member-c-menu`、`member-d-order`、`member-e-files`。
提交前先让 AI 按 CODE_REVIEW.md 检查，再由本人逐项复核、编译和实际测试。
到 GitHub 点击 `Compare & pull request`，目标分支选 main，说明修改内容和实际测试结果，选择组长为 Reviewer。
收到修改意见后，继续在同一分支修改、提交、推送；原 PR 会更新，不用重新建一个 PR。

### 组长如何审查

打开 PR 的 `Files changed` 查看改动，按 CODE_REVIEW.md 检查并运行关键功能。
需要修改时选择 `Review changes → Request changes`；通过时选择 `Approve`，再合并到 main。
组员之后同步 main，下一项任务重新创建分支，不在 main 上直接改代码上传。

### main 要启用的保护

- 必须通过 Pull Request 合并，至少需要 1 个批准。
- 必须由 CODEOWNERS 指定的 `VunlerHoomer` 批准，其他组员的批准不能代替。
- PR 增加新提交后，旧批准失效，需要重新审查。
- 合并前解决审查讨论，禁止组员强制推送或删除 main。

保护针对普通协作者；仓库所有者保留管理员管理和绕过权限，方便处理自己的提交。
仅有 CODEOWNERS 文件不能强制审查，还必须有受支持的仓库套餐/可见性以及已启用的分支保护。

## 进度安排

- 11 月 15 日前：各人完成自己的基础功能。
- 11 月 22 日前：串通注册、充值、下单和历史查询。
- 11 月 29 日前：补齐店长管理和统计。
- 12 月 6 日前：完成测试和报告初稿。
- 12 月 10 日前：在目标电脑运行，准备完整提交包。
- 12 月 11–12 日：签贡献表并提前提交。

正式任务书截止时间为 2026 年 12 月 13 日 23:59；公告截图年份有冲突，需向老师核实。
