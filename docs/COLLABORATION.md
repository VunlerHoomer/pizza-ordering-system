# GitHub 协作操作

仓库：https://github.com/VunlerHoomer/pizza-ordering-system

仓库默认为私有。组长需在 Settings → Collaborators 中邀请组员；组员接受邀请后才可克隆及推送。
每个人在自己的电脑上登录自己的 GitHub 账号，不共享账号或令牌。

## 第一次获取代码

```sh
git clone https://github.com/VunlerHoomer/pizza-ordering-system.git
cd pizza-ordering-system
code .
```

## 开始一个新任务

确认工作区没有未提交修改，然后从最新 main 新建个人任务分支：

```sh
git switch main
git pull --ff-only
git switch -c codex/accounts-register
```

分支名示例：`codex/app-shell`、`codex/accounts-register`、`codex/menu-pricing`、
`codex/orders-wallet`、`codex/storage-stats`。每个任务用不同名称，不要多人推同一分支。

## 提交和上传

在 VS Code 左侧“源代码管理”检查差异，暂存本次修改，填写提交说明并提交。
首次使用“发布分支”，后续使用“推送”；仓库已连接 GitHub。

也可在终端执行（路径按实际任务修改）：

```sh
git status
git add src/account.c include/account.h
git commit -m "Implement customer registration"
git push -u origin HEAD
```

到 GitHub 发起 Pull Request，目标分支选 `main`。相关同学检查、编译验证后再合并。

## 在已有任务分支同步别人的更新

先提交当前工作，再执行：

```sh
git fetch origin
git merge origin/main
```

如有冲突，在 VS Code 中逐个确认正确内容，重新编译，然后提交合并结果。
不要强制推送公共分支，不要通过丢弃别人修改来解决冲突。

## 团队约定

- 主分支保持可编译。各模块负责人维护自己的 `.c/.h`。
- 修改 `types.h` 或公共接口时，先记录参数、返回值和受影响的调用方。
- 组员提交代码时附带对应报告说明和真实测试记录。
- 每周三短会；每周日演示验收。同步完成项、下一步和阻塞问题。
- Git 提交只是贡献记录的一部分；贡献表结合设计、测试、报告、整合等实际工作确定。
