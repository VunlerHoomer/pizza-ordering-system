# 操作说明：成员 A 版本

## 1. 构建与运行

在 VS Code 中打开仓库根目录。Mac 按 Cmd+Shift+B 编译。
命令面板运行 `Tasks: Run Task` → `Pizza: Run`。终端也可执行：

```sh
make run
```

没有 C 编译器时需先安装相应工具链。Windows 安装 GCC/MinGW-w64 并加入 PATH，使用：

```powershell
./build.ps1
./build/pizza-ordering-system.exe
```

程序路径：Mac/Linux 为 `build/pizza-ordering-system`，Windows 为 `build/pizza-ordering-system.exe`。
可使用 `--help` 查看帮助、`--version` 查看版本。未知参数以退出码 2 结束。

## 2. 正常程序当前状态

首页：1 顾客登录，2 顾客注册，3 店长登录，0 退出。
选择登录后输入账号和密码；账号 0 或空密码取消并返回首页。
B 尚未实现身份验证，所以正常程序会提示功能暂不可用，不会进入已登录菜单。
存储尚未实现时启动会提示数据不能保存；不要把这个版本当成可记录订单的正式系统。

## 3. 演示 A 的菜单

```sh
make demo
```

或者 VS Code 运行 `Pizza: Demo`。Windows 先 `./build.ps1 -Mode demo`，再运行 `./build/pizza-demo.exe`。

| 入口 | 演示账号 | 演示密码 |
|---|---|---|
| 1 顾客 | 1001 | customer-demo |
| 3 店长 | 9001 | manager-demo |

这些凭据仅在演示程序中有效，密码输入当前会在终端显示。
登录成功后可选择 1–6 查看模块调用，0 退出登录。业务动作会显示 `[DEMO]` 调用记录并提示未实现，
不会创建实际账户、订单或收款。不要将演示账户放进实际账户实现。
退出登录后回到首页，可再登录另一角色；首页选择 0 退出整个程序。

## 4. 菜单与异常处理

顾客：浏览菜单、下单、订单历史、充值、查看资料、编辑资料。
店长：编辑菜单、修改价格、添加顾客、删除顾客、编辑顾客、销售统计。
菜单要求范围内的整数；字母、超范围数字、尾随字符、超长输入会提示重新输入。

Mac/Linux 的 Ctrl+D（空行）或 Windows 控制台 EOF 会结束输入，程序尝试正常关闭。
真实存储接入后，加载失败时不会进入菜单或覆盖数据。退出保存失败会提示：
1 重试保存；0 放弃保存并退出（退出码 1）。无输入可用时同样返回失败，不能视为已保存。
强行结束进程不能保证退出保存运行，因此 D/E 仍须在每笔交易成功时保存。

## 5. 测试与数据

```sh
make test
```

需要 Python 3，只使用标准库。Windows：`./build.ps1 -Mode test`。
测试和演示替身不读写业务文件；未来个人运行数据放 `data/runtime/`，该目录不上传 GitHub。
自动化记录见 `tests/MEMBER_A_RESULTS.md`。最终提交必须另行测试全组真实业务和 Windows 环境。
