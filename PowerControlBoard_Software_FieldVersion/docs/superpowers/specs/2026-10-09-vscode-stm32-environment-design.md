# VS Code STM32 Makefile 环境整理设计

## 目标

依据博客中的 STM32 for VS Code 工作流，把当前 STM32G474RBT6 项目整理为 VS Code + Makefile + ARM GNU + OpenOCD/ST-Link 开发环境，并清理失效或跨作用域重复的 Windows PATH 项。

## 当前约束

- 工程目标芯片为 STM32G474RBT6，当前调试器为已枚举的 ST-Link。
- 工具链已安装并可用：ARM GNU 15.2.1、GNU Make 4.4.1、OpenOCD 0.12.0。
- 工程根目录的 \`Makefile\`/ \`SuperCap.mk\` 生成 \`build/JointSuperCap-v1.0-debug.elf/.hex/.bin\`。
- 不迁移、不卸载工具目录；不修改现有源码和 Makefile。
- 当前仓库存在其他未提交改动，本次只提交 \`.vscode\` 与设计/计划文档。

## 设计

### VS Code 项目配置

- \`.vscode/settings.json\` 使用实际的 \`D:\\msys64\\usr\\bin\\make.exe\`、\`D:\\msys64\\mingw64\\bin\\openocd.exe\` 和 ARM GCC bin 路径。
- 启用 Makefile Tools 的 \`makefile.configureOnOpen\`，移除无法解析的 \`cube-cmake\`、\`cube\` 和未定义的 \`CUBE_BUNDLE_PATH\` 覆盖。
- \`.vscode/extensions.json\` 推荐 Makefile Tools、Cortex-Debug 和 STM32 for VS Code。
- \`.vscode/tasks.json\` 直接执行 \`make -j2 SHELL=D:/msys64/usr/bin/sh.exe ECHO=@echo BUILD_DIR=build\`，保证构建产物与调试配置一致。
- \`.vscode/launch.json\` 只保留 STM32G474RB + ST-Link/OpenOCD 的 flash/debug 与 attach 配置，并把 ELF/SVD/任务引用改为当前项目实际路径；CubeCLT ST-LINK GDB Server 作为备用入口。

### Windows 环境变量

修改前导出用户级、系统级 PATH 和本次涉及的 STM32 变量到外部 JSON 备份。

- 用户 PATH 删除不存在的 \`C:\\Users\\ASUS\\.dotnet\\tools\`、\`C:\\Users\\ASUS\\AppData\\Local\\Python\\bin\`。
- 系统 PATH 删除与用户 PATH 完全重复的 VS Code、ARM GCC、MSYS2 usr/bin、MSYS2 mingw64/bin 四项，保留用户级项。
- 保留有效的 \`STM32CubeMX_PATH\`、\`STM32_PRG_PATH\`、\`GCC_TOOLCHAIN_ROOT\`、\`CLANG_GCC_CMSIS_COMPILER\`；不添加不存在的 \`CUBE_BUNDLE_PATH\`。
- 不删除任何工具目录；系统 PATH 若权限不足则明确报告未完成。

## 验证

- 新 PowerShell 中确认 \`arm-none-eabi-gcc -v\`、\`make --version\`、\`openocd --version\` 成功。
- 运行 \`make -j2 SHELL=D:/msys64/usr/bin/sh.exe ECHO=@echo BUILD_DIR=build\`，确认生成 ELF/HEX/BIN。
- 解析 VS Code JSON，确认所有任务、工具、ELF、SVD 和 OpenOCD 配置路径存在，且不残留 \`cube-cmake\`、\`basic_framework.elf\`、\`STM32F407\`、\`openocd_dap.cfg\`。
- 不在没有硬件会话结果时宣称 ST-Link 已完成实际烧录或运行验证。

