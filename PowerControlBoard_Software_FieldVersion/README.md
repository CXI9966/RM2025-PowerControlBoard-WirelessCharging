# JointSuperCap v1.0-debug

## Makefile + VS Code

### 1. 工具和硬件

工程默认使用 Makefile、ARM GNU Toolchain、OpenOCD 和 VS Code。本机工具路径：

- GNU Make：`D:/msys64/usr/bin/make.exe`
- POSIX shell：`D:/msys64/usr/bin/sh.exe`
- ARM GNU：`C:/Toolchain/arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi/bin`
- GDB：`C:/ST/STM32CubeCLT_1.21.0/GNU-tools-for-STM32/bin/arm-none-eabi-gdb.exe`
- OpenOCD：`D:/msys64/mingw64/bin/openocd.exe`

DAPLink 与 STM32G474 的 SWD 连接必须正确：`SWDIO`、`SWCLK`、`GND`，并按调试器/目标板要求接 `VTref` 和 `NRST`。目标板需要单独供电；只连接 USB 不能证明 MCU 已上电。

在 VS Code 扩展中安装：

- `Makefile Tools`（扩展 ID：`ms-vscode.makefile-tools`）
- `Cortex-Debug`（扩展 ID：`marus25.cortex-debug`）

重新加载 VS Code 窗口后，Makefile Tools 的 `Default` 配置包含本工程需要的 MSYS shell 和 ARM 工具链参数。CMake 自动配置已关闭。

`.vscode/launch.json` 使用当前保留的 `DAPlink` 配置，OpenOCD 加载根目录 `openocd_dap.cfg`，后者加载同目录 `stm32g4x.cfg`，调试 Makefile 生成的 ELF。安装目录的 scripts 搜索路径仍用于 `interface/cmsis-dap.cfg`、`target/swj-dp.tcl` 和 `mem_helper.tcl`。当前按未接 NRST 的四线 SWD 配置，连接和复位时均使用 100 kHz。

### 2. 命令行构建

在工程根目录执行：

```powershell
& 'D:/msys64/usr/bin/make.exe' -j2 `
  'SHELL=D:/msys64/usr/bin/sh.exe' 'ECHO=@echo' `
  'GCC_PATH=C:/Toolchain/arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi/bin' `
  'BUILD_DIR=build'
```

产物位于 `build/`：

- `JointSuperCap-v1.0-debug.elf`：GDB 调试符号和下载输入
- `JointSuperCap-v1.0-debug.hex`：HEX 烧录文件
- `JointSuperCap-v1.0-debug.bin`：BIN 烧录文件

### 3. VS Code 中 F5 调试

主调试和 Live Watch 使用 STM32CubeCLT 自带的 GDB。ARM GNU 15.2 Windows 包中的 GDB 仅提供有限的字符集转换，读取 `bool` 类型时可出现 `could not convert 'bool' from the host encoding (CP1252) to UTF-32` 警告；直接设置 `UTF-8` 在该版本中也会被拒绝。当前 ELF 已用 CubeCLT GDB 离线验证，类型查询和布尔表达式正常且无此警告。修改调试配置后，停止当前调试会话，再按 `F5` 启动，新路径才会生效。

1. 用 VS Code 打开本目录，执行 `Developer: Reload Window`。
2. 按 `Ctrl+Shift+B` 运行默认的 `build task`，确认编译成功。
3. 打开“运行和调试”，选择 `DAPlink`。
4. 按 `F5`。VS Code 会先执行同一个 Makefile 构建任务，然后启动 OpenOCD、下载 ELF，并在 `main` 停住。
5. 用左侧“变量”“监视”“调用堆栈”和“断点”进行调试；`F5` 继续，`F10` 单步越过，`F11` 单步进入，`Shift+F5` 停止。

本次排查确认：旧任务调用本机不存在的 `mingw32-make`；ST-Link 的 `preLaunchTask` 引用未定义的任务；部分模板还引用 `basic_framework.elf` 和 STM32F407。配置已统一到实际 Makefile 产物。另有 CMake 的 Windows/MSYS 缓存路径冲突；Makefile 流程不使用该缓存。

完整 Makefile 编译通过，仍有 HAL 空翻译单元和 ELF RWX LOAD 段两条警告。本次未执行烧录或硬件调试。

### 4. OpenOCD 连接自检

先关闭占用调试器的其他程序，在 PowerShell 执行：

```powershell
& 'D:/msys64/mingw64/bin/openocd.exe' `
  -s 'D:/msys64/mingw64/share/openocd/scripts' `
  -f './openocd_dap.cfg' `
  -c "gdb_port disabled; tcl_port disabled; telnet_port disabled; init; targets; shutdown"
```

连接成功应看到 `SWD DPIDR 0x2ba01477` 和 Cortex-M4 已完成检查，只有 target 配置名称不代表已连接。`CMSIS-DAP command CMD_INFO failed` 属于电脑与调试器的 CMSIS-DAP 通信阶段，需检查 USB、驱动、调试器固件和接口占用；COM 串口不是 SWD 调试接口。

若调试器已经初始化，随后出现 `Error connecting DP: cannot read IDR`，则仍未读到芯片的 SWD Debug Port ID。2026-10-09 实测 2000 kHz 和 100 kHz 均复现该错误，降速尚未解决连接。先量 MCU 的 VDD 对 GND 是否约 3.3 V，再检查 DAP 的 SWDIO → PA13、SWCLK → PA14、GND 共地及接插件/焊点。四线连接未接 NRST 时，调试器日志的 `nRESET = 0` 不代表已经量到芯片 NRST；应在芯片侧测量。接出 NRST 后，才可另行测试复位下连接。

当前为关节超电应用层调试版：电池经本板总电流采样后，输出同时供四个关节电机和 DJI 官方电源管理模块。已完成当前实板烧录、回读和启动核对；电池功率上限、电流零点及实际CAN通信仍需验证，尚非正式带载版本。DCDC启动许可、控制环和保护沿用原工程。

产品名和版本、三个 CAN 报文 ID 集中在 `Core/Inc/Calibration.hpp`。构建目标为 `JointSuperCap-v1.0-debug`；用户选择的调试地址为命令 `0x161`、浮点反馈 `0x151`、紧凑反馈 `0x152`。主板需同步；后两种反馈只选一种发送。501配置已匹配当前板的UID，采样系数尚未实测确认。上板记录位于工作区 `dev-docs/board-bringup.md`。

## 通讯格式

### 主控板>电容

~~~
struct RxData {
    uint8_t enableDCDC: 1;
    uint8_t systemRestart: 1;
    uint8_t resv0: 3;
    uint8_t clearError: 1;
    uint8_t enableActiveChargingLimit: 1;
    uint8_t useNewFeedbackMessage: 1;

    uint16_t refereePowerLimit;
    uint16_t refereeEnergyBuffer;
    uint8_t activeChargingLimitRatio; 
    int16_t resv2;
} __attribute__((packed));
~~~

| 变量名 | 功能 | 详细描述 |
| -- | -- | -- |
| enableDCDC | 允许启动DCDC | 如果为0：立即关闭DCDC，并且不主动重启 |
| systemRestart | 系统重启 | 触发 `NVIC_SystemReset();` |
| clearError | 清除故障 | 可清除 `ERROR_RECOVER_MANUAL` (短路保护或电容组故障)、`ERROR_RECOVER_AUTO` (过流或过压，电容组本身也会自动尝试恢复) 级别的错误；但是不可清除 `ERROR_UNRECOVERABLE` (功率级故障) 级别的错误。 <br> 建议主控板只对 `ERROR_RECOVER_MANUAL` 级别的错误进行处理，且触发方式为操作手手动|
| enableActiveChargingLimit | 启用主动充电限制 | 开启后当电容组能量达到设定值，将不会再对电容组进行主动充电（即不会从裁判系统获取电量，但可以通过能量回收等方式充电直到最大电压），此时裁判系统功率的闭环为一个略小于底盘供电网络静态功耗的值 <br> 开启关闭有0.2V的施密特触发防止震荡 <br> **使用方式：开始比赛设为1，未开始比赛或开始比赛进入虚弱模式后设为0** |
| useNewFeedbackMessage | 反馈格式选择 | 0：`0x151` 浮点格式；1：`0x152` 两个紧凑功率字段。上电默认0；每次仅发送选中的一种格式。字节布局保留。 |
| refereePowerLimit | 裁判系统功率限制 | 单位W <br> 默认主控板发来的功率限制可信，建议在主控板上做好数据保护 |
| refereeEnergyBuffer | 原裁判系统缓冲能量 | 单位J，当前遗留能量环目标57 J。DJI模块位于分支上，后续必须与电池总功率控制分开；本基础稿尚未替换该能量环。 |
| activeChargingLimitRatio | 允许启动DCDC | 主动充电目标比例（能量），范围为0-255，计算方式为 `TargetVCap = sqrtf (activeChargingLimitRatio / 255) * CAPARR_MAX_VOLTAGE`，同时对计算结果进行限制，最低 `CAPARR_LOW_VOLTAGE` (10.0V)|


### 电容>主控板(旧)
~~~
struct TxData {                     // CAN_FEEDBACK_ID (useNewFeedbackMessage = 0)
    uint8_t statusCode;
    float chassisPower;
    uint16_t chassisPowerLimit;
    uint8_t capEnergy;              // 
} __attribute__((packed));
~~~

| 变量名 | 功能 | 详细描述 |
| -- | -- | -- |
| statusCode | 状态信息 | 详情见下一节 “电容>主控板(新)” |
| chassisPower | 两路整车负载总功率 | 低通后的 `vA × (iR − iA)`，单位W；包含电容对母线的贡献。沿用历史字段名，物理电流方向不变。 |
| chassisPowerLimit | 遗留可用功率估算 | `CAPARR::getMaxPowerFeedback() + refereePowerLimit`，单位W；仍受旧命令含义影响，不能当作BMS已确认的整车最大可用功率。电容正常区估算采用15 A，未补偿效率。 |
| capEnergy | 电容现有能量 | 以能量比例为准，计算方式为`(VCap/CAPARR_MAX_VOLTAGE)^2 * 250U`，满电值为250，超过250说明代表电容实际充电超过100% |

### 电容>主控板(新)

~~~
struct TxDataNew {                  // CAN_FEEDBACK_NEW_ID (useNewFeedbackMessage = 1)
    uint8_t statusCode;
    uint16_t chassisPower;
    uint16_t refereePower;
    uint16_t chassisPowerLimit;
    uint8_t capEnergy;
} __attribute__((packed));
~~~

由于旧通讯格式下主控板对`errorCode`几乎没有使用，新通讯格式将其改为`statusCode`进行错误与系统状态反馈。

| Bit | 功能 | 详细描述 |
| -- | -- | -- |
|  7 | 功率级状态 | 1为启动，0为未启动（触发保护或主控板禁用）|
| 6 | 反馈信息格式 | 1为新通讯格式，0为旧通讯格式（RM2024） |
| 5:4 | 遗留无线充电状态 | 本产品关闭无线充电，固定沿用OFF=1；主板忽略该字段。 |
| 3:2 | 控制外环限制因素 | 0：输入功率；1：最高电容电压；2：主动充电目标；3：电容充/放电限流。 |
| 1:0 | 错误等级 | `NO_ERROR = 00` `ERROR_RECOVER_AUTO = 01` `ERROR_RECOVER_MANUAL = 10` `ERROR_UNRECOVERABLE = 11` |


其余变量的定义更改如下，其余变量定义与旧通讯格式相同
| 变量名 | 功能 | 详细描述 |
| -- | -- | -- |
| chassisPower | 两路整车负载总功率 | 低通后的 `vA × (iR − iA)` 编码为 `P × 64 + 16384`。解码 `P = raw/64 − 256` W。 |
| refereePower | 电池总输出功率 | 低通后的 `vA × iR`，历史字段名沿用；现在是四个关节电机和DJI支路共用的电池输入测量，不能直接视作DJI模块测得的功率。编码同上。 |

紧凑功率字段的可表示范围是 −256～767.984375 W，越界显式饱和；饱和值不表示已测得真实峰值。整车功率可能超过此范围时，使用浮点反馈，或与主板共同确定新编码后再更改紧凑格式。

## 关节适配与验证

已有总电流和电压采样继续使用，不假设电压固定为24 V。`Core/Inc/PowerTelemetry.hpp` 定义电池/整车负载功率关系和紧凑编码。普通/标定配置都关闭 `WPT_HARDWARE`，外部编译参数尝试启用时会报错；ASK/E桥代码保留且不运行，ADC通道与硬件接口保留。

目前 `PowerControl::updateRefereePower()` 的5～135 W钳位、37 W默认值、裁判缓冲能量修正及保护阈值仍为原实现。电池/BMS规格和主板报文确定后，按工作区 `dev-docs/joint-supercap-adaptation.md` 替换为电池总功率预算；不能仅提高电容/DCDC电流来解决整车电池功率限制。

在工作区根目录执行 `python tools/verify_joint_firmware.py`：运行真实C++功率计算/编码边界测试、ARM报文布局和位域检查，以及无线编译配置检查。完整构建命令（GNU Make/MSYS shell）：

```powershell
$env:PATH = 'D:\msys64\usr\bin;' + $env:PATH
make -j2 SHELL=D:/msys64/usr/bin/sh.exe 'ECHO=@echo' BUILD_DIR=build_joint_review
```

本次完整编译成功；有HAL空翻译单元和链接RWX段两项警告。未烧录、未测量板卡波形；软件测试不代表电气验证。

## 峰值电流模式BuckBoost

频率250k，counter 21760

## 内环控制

HRTIM频率250k，counter 21760

### HRTIM

Master Timer
触发TimerA复位 (RST)
触发TimerB复位 (CMP2)
触发ADC采样 (CMP1)

Timer A B
1st RESET: 周期开始 (CMP1)
1st SET: 占空比保护 (CMP3)
2nd: SET: 外部事件触发 (External Event)
触发DAC三角波RST (RST)
触发DAC三角波STEP (CMP4)

### 峰值电流触发链

IA(+) & DAC1_OUT2(-) > COMP3 > HRTIM External Event 8 (low active) > External Event 8 Filtering (TimerB CMP4) > SET
IB(+) & DAC1_OUT1(-) > COMP2 > HRTIM External Event 6 (low active) > External Event 6 Filtering (TimerA CMP4) > SET


## 外环控制

### 控制逻辑

每次计算外环得出一个目标 deltaiLTarget

首先设为裁判系统功率PID输出

然后进行其他限制(优先级从高到低)
//已经过时
1. 电容最高电压 | IL上限
2. 反向IR为0 (动能回收) | IL下限
3. VA电压上限 (动能回收) | IL下限
4. 电容80%电压 | IL上限
5. 电容输入电流IB | IL上限
6. 电容输出电流IB | IL下限

## ADC与保护

使用ADC Watchdog硬件触发Timer输出关断和软件中断

注意：AWDG1和AWDG2/3不同，AWDG1精度为12位，AWGD2/3只能监测高8位，所以对于vA和vB使用AWDG1，对于电流使用AWDG2/3

iA > ADC1_Channel9 > ADC1_AWDG2 > HRTIM1_ExternalEvent2 > HRTIM1_FaultLine2 
iR > ADC1_Channel8 > ADC1_AWDG3 > HRTIM1_ExternalEvent3 > HRTIM1_FaultLine3
vA > ADC1_Channel12 > ADC1_AWDG1 > HRTIM1_ExternalEvent1 > HRTIM1_FaultLine1 
iB > ADC2_Channel5 > ADC2_AWDG2 > HRTIM1_ExternalEvent5 > HRTIM1_FaultLine5
vB > ADC2_Channel3 > ADC2_AWDG1 > HRTIM1_ExternalEvent4 > HRTIM1_FaultLine4

中断使用HRTIM的IER寄存器启用

中断后通过ISR寄存器判断中断来源

中断是否直接通过FaultLine禁用TimerA/TimerB输出可以通过HRTIM_FLTxR寄存器实现


## ASK数据格式（历史代码，本产品停用）

ASK通讯等效2000波特率

周期20 发送频率100Hz

单个数据包长12bit，开始停止各1bit，校验1bit，数据9bit

数据位：

需求功率5W或80W：1bit
功率反馈：8bit，量程100W
