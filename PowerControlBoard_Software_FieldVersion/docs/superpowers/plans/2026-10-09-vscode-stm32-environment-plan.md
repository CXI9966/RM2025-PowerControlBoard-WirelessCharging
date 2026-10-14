# VS Code STM32 Makefile 环境整理实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (\`- [ ]\`) syntax for tracking.

**Goal:** 将当前 STM32G474 项目整理为 VS Code + Makefile + ARM GNU + OpenOCD/ST-Link 工作流，并去除失效及重复 PATH 项。

**Architecture:** VS Code 的构建入口直接执行项目根目录 Makefile，Cortex-Debug 使用 Makefile 生成的 \`build/JointSuperCap-v1.0-debug.elf\`。环境变量先备份，再去除失效项和跨作用域重复项。

**Tech Stack:** VS Code 1.141.0, Makefile Tools, Cortex-Debug, ARM GNU 15.2.1, GNU Make 4.4.1, OpenOCD 0.12.0, PowerShell.

---

### Task 1: 记录并检查基线

**Files:**
- Read: \`docs/superpowers/specs/2026-10-09-vscode-stm32-environment-design.md\`
- Create: \`docs/superpowers/plans/2026-10-09-vscode-stm32-environment-plan.md\`

- [ ] **Step 1: 保存当前配置与环境快照**

导出 \`.vscode\`、用户/系统 PATH 和 STM32 变量到 \`%USERPROFILE%\\.codex\\backups\\stm32-vscode-20261009\\before.json\`。

- [ ] **Step 2: 检查快照可读**

用 \`ConvertFrom-Json\` 解析快照并确认包含 PATH、变量和 VS Code 配置字段。

### Task 2: 修正 VS Code 设置与推荐扩展

**Files:**
- Modify: \`.vscode/settings.json\`
- Modify: \`.vscode/extensions.json\`

- [ ] **Step 1: 写入 Makefile 工具路径**

设置 \`makefile.makePath=D:\\msys64\\usr\\bin\\make.exe\`、\`makefile.configureOnOpen=true\`、\`stm32-for-vscode.makePath=D:\\msys64\\usr\\bin\\make.exe\`、\`stm32-for-vscode.openOCDPath=D:\\msys64\\mingw64\\bin\\openocd.exe\`、\`stm32-for-vscode.armToolchainPath=C:\\Toolchain\\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\\bin\`，并设置 Cortex-Debug 的 OpenOCD/ARM 工具链路径。

- [ ] **Step 2: 删除失效 CMake/Cube 覆盖**

移除 \`cube-cmake\`、\`cube\`、\`CUBE_BUNDLE_PATH\` 和与 Makefile 主流程冲突的 CMake 自动配置项。

- [ ] **Step 3: 更新扩展推荐**

推荐 \`ms-vscode.makefile-tools\`、\`marus25.cortex-debug\`、\`stmicroelectronics.stm32-vscode-extension\`。

### Task 3: 让 VS Code 构建和调试走 Makefile

**Files:**
- Modify: \`.vscode/tasks.json\`
- Modify: \`.vscode/launch.json\`

- [ ] **Step 1: 写入 Makefile 构建任务**

保留标签 \`firmware: build make\`，使用 process 任务执行 \`D:\\msys64\\usr\\bin\\make.exe\`，参数为 \`-j2\`、\`SHELL=D:/msys64/usr/bin/sh.exe\`、\`ECHO=@echo\`、\`BUILD_DIR=build\`，并设为默认 build 任务。

- [ ] **Step 2: 写入 STM32G474 ST-Link 配置**

保留 OpenOCD flash/debug 和 attach；使用 \`build/JointSuperCap-v1.0-debug.elf\`、\`STM32G474.svd\`、\`interface/stlink.cfg\`、\`target/stm32g4x.cfg\`，并将 flash 配置的 \`preLaunchTask\` 指向 \`firmware: build make\`。

- [ ] **Step 3: 写入 CubeCLT 备用配置**

保留一个显式指向 \`C:\\ST\\STM32CubeCLT_1.21.0\\STLink-gdb-server\\bin\\ST-LINK_gdbserver.exe\` 的备用 ST-Link 配置；不宣称硬件验证已完成。

- [ ] **Step 4: 解析并检查引用**

确认 launch 的任务存在，ELF/SVD/OpenOCD/GDB/服务器路径存在，旧 F407/DAPLink 引用消失。

### Task 4: 备份并清理 Windows 环境变量

**Files:**
- External backup: \`%USERPROFILE%\\.codex\\backups\\stm32-vscode-20261009\\before.json\`

- [ ] **Step 1: 计算清理结果**

从用户 PATH 删除两个不存在目录；从系统 PATH 删除四个与用户 PATH 完全重复的工具目录；不动工具文件夹和有效 STM32 变量。

- [ ] **Step 2: 写入用户 PATH**

用 \`[Environment]::SetEnvironmentVariable('Path', $newUserPath, 'User')\` 写入并读回验证。

- [ ] **Step 3: 写入系统 PATH**

尝试写入清理后的系统 PATH；权限拒绝时记录结果并保留用户级清理。

### Task 5: Makefile 构建和交付前验证

**Files:**
- Read: \`build/JointSuperCap-v1.0-debug.elf\`
- Read: \`build/JointSuperCap-v1.0-debug.hex\`
- Read: \`build/JointSuperCap-v1.0-debug.bin\`

- [ ] **Step 1: 验证工具版本**

运行 \`arm-none-eabi-gcc -v\`、\`make --version\`、\`openocd --version\`。

- [ ] **Step 2: 执行 Makefile 构建**

运行 \`make -j2 SHELL=D:/msys64/usr/bin/sh.exe ECHO=@echo BUILD_DIR=build\`，预期退出码为 0。

- [ ] **Step 3: 验证产物与配置**

确认三个产物存在，ELF 为 ARM 32-bit；解析 JSON 并确认无旧配置残留。

- [ ] **Step 4: 检查差异范围**

运行 \`git diff -- .vscode docs/superpowers\` 与 \`git status --short\`，确认没有修改源码和 Makefile。

