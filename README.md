# 披萨点餐系统

SOE103 五人小组作业，用 C 语言实现控制台点餐。
A 的菜单、C 的披萨功能和 E 的文件统计已完成；B 的账户与 D 的订单仍在开发。
项目内容看 [PRD.md](PRD.md)，提交前看 [CODE_REVIEW.md](CODE_REVIEW.md)。

## 文件分工

| 文件 | 负责人 | 内容 |
|---|---|---|
| main.c、input.c | A | 主菜单、登录入口、输入检查 |
| account.c | B | 注册、登录、顾客资料 |
| pizza.c | C | 披萨菜单、价格和编辑 |
| order.c | D | 下单、充值和历史 |
| file.c | E | 读写数据、备份和统计 |
| project.h | 全员共用 | 结构体、容量和函数声明 |
| sample.txt | E | 初次运行的数据样例 |
| test_file.c | E | 单独运行的简单文件测试 |
| report.md | 全员 | 报告草稿和测试记录 |

代码都放在同一层。保持已有公共函数名，避免组员正在写的代码接不上。
`.vscode/tasks.json` 用于编译运行，`.github/CODEOWNERS` 用于组长审查，不需要日常修改。

## 第一次运行

只在没有旧数据的新目录中，把 `sample.txt` 复制成 `shop.txt`。
原来的初始模板 `shop.example.txt` 已改名为 `sample.txt`；已有 shop.txt 的格式不用改。

Mac/Linux：

```sh
cp sample.txt shop.txt
gcc -std=c11 -Wall main.c input.c account.c pizza.c order.c file.c -o pizza
./pizza
```

Windows PowerShell：

```powershell
Copy-Item sample.txt shop.txt
gcc -std=c11 -Wall main.c input.c account.c pizza.c order.c file.c -o pizza.exe
./pizza.exe
```

已有 shop.txt 时跳过复制步骤，不能用样例覆盖自己的数据。
VS Code 也可以用 Build、Run 任务。需要预先安装 GCC；Windows 仍需组员实机验证。
目前可以进入首页，但 B 的真实登录和 D 的交易尚未完成，不能把占位返回值改成成功来绕过。

## 数据文件

- `shop.txt`：当前数据；`shop_backup.txt`：上一份备份。
- `shop.tmp`、`shop_backup.tmp`：保存时的临时文件，正常成功后会消失。
- 上述运行文件不上传 GitHub。最终交作业时，需要附上准备好的 shop.txt。

文本格式保持不变：文件头 PIZZA_SHOP_V1，然后依次为 CUSTOMERS、TYPES、SIZES、TOPPINGS、ORDERS，最后一行 END。
每个部分先写记录数，再按 project.h 中的字段顺序写各记录，每个字段一行。订单字段顺序为编号、顾客编号、三类选项编号、三个成交名称、数量、单价、总额。

保存顺序：先写好并检查新文件，再准备备份，最后替换正式文件。没有改动时保留原备份。
保存失败后程序结束，不重试；业务模块恢复本次内存改动。程序按单终端、单进程使用。

发现文件错误时：先关闭程序，另存现有数据和临时文件；核对有效备份后恢复为 shop.txt，再移走工作目录中的临时文件并重启。不能直接删除旧数据来消除报错。

E 主要用课内的数组、结构体、循环和文件读写。为保持原有数据兼容和保存安全，保留少量补充：
`INT_MAX` 表示整数上限，`errno/ENOENT` 区分文件不存在和没有权限，`rename/remove` 处理文件；Windows 的替换由一个小函数集中使用 `MoveFileExA`。
这些补充已在代码中注释，应由 E 本人理解后解释，不把它们说成课件全部讲过。代码不保证突然断电后的物理持久性。

## 简单测试

单独编译测试文件，不要与 main.c 一起编译：

```sh
gcc -std=c11 -Wall test_file.c input.c account.c pizza.c file.c -o test_file
```

Windows 把输出名改为 test_file.exe。将编译出的测试程序放到一个新建的空文件夹运行；它会建立虚构数据并检查保存和重读。
不要在项目目录或已有业务数据的目录运行。统计预期为 1 单、2 个披萨、74.00 元，结束时应显示 0 failed checks。
测试是开发辅助，不属于应用入口。更多异常检查的实际结果记录在 report.md。

## 组员协作

先接受组长的 GitHub 邀请。每个任务从干净、最新的 main 创建个人分支：

```sh
git switch main
git pull --ff-only
git switch -c member-b-account
# 改成自己的分支名；修改自己负责的文件
git add account.c
git commit -m "Complete customer registration"
git push -u origin HEAD
```

上传前先让 AI 按 CODE_REVIEW.md 自查，再由本人复核和实际测试。修改 project.h 前先在群里沟通。
上传后在 GitHub 发起 PR：base 选 main，说明完成内容、实际测试结果和剩余问题，由 VunlerHoomer 审查。
收到意见后继续改同一分支。main 已受保护，组员必须经组长批准；追加提交后需要重新批准。所有者保留管理员权限。

## 提交作业

正式任务书截止时间为 2026-12-13 23:59，公告截图年份有冲突，组长需核实。
提交源码、Windows exe、shop.txt、PDF 报告、操作说明和全员签名贡献表。报告封面写全员姓名、学号。
