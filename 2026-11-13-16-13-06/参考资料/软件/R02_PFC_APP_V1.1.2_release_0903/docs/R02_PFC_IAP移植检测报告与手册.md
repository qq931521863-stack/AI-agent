# R02 PFC AB 双槽 IAP 移植检测报告与手册

> **检测日期**：2026-06-11  
> **目标工程**：`R02_PFC_APP_V1.0.1`  
> **参考工程**：`xpPFC1600_v1.0.3_release_6.6`  
> **Boot 工程**：`IAP Bootloader`（与 xp 共用布局）  
> **主机侧**：`R02_LLC_APP_V1.0.3`  
> **本文性质**：只读检测报告 + 移植手册，**不含代码改动**

---

## 一、结论摘要

| 维度 | 结论 |
|------|------|
| **功率控制是否被 IAP 改动破坏** | **否（设计上隔离）**。快环/慢环/ADC/PWM 核心路径未被 IAP 补丁直接修改；APP 槽仅在 `HAL_IncTick` 中追加 1ms 升级任务 |
| **IAP 栈是否移植完成** | **软件栈基本完成（约 85%）**；编译可通过；**端到端 CAN→RS485 OTA 尚未在本仓库内验收通过** |
| **为何总出问题** | 见 [第八章](#八为什么移植这么难根因分析)；本质是 **三套协议 + 四套约束 + 两套产品基线** 叠加，而非单一模块 bug |

---

## 二、改动边界（什么动了、什么没动）

### 2.1 设计原则（移植约定）

移植说明原文约定：

> 只移植 OTA/IAP 升级能力，**不改动 R02 量产 Target 的 PFC/Modbus 业务逻辑**

Keil 通过 **Target 分离** 实现：

| Target | 宏 | 链接 IAP 源文件 | 用途 |
|--------|-----|----------------|------|
| `xpPFC431_48` | 无 `APP_SLOT_*` | 否 | 量产 `@0x08000000` |
| `APP_A` | `APP_SLOT_A_BUILD` | 是 | OTA 槽 A `@0x08008000` |
| `APP_B` | `APP_SLOT_B_BUILD` | 是 | OTA 槽 B `@0x08013000` |

### 2.2 IAP 移植实际改动的文件（仅 APP_A/B）

| 文件 | 改动性质 | 是否触及功率算法 |
|------|----------|------------------|
| `Core/Src/main.c` | 增加 `UpgradeAppEarlyInit/Init`、`AppIntoUpgrade_MainLoopTask`；USART1 波特率 **115200** | 否（启动序列/串口参数） |
| `AppUser/ConsoleSlow.c` | `#if APP_SLOT_*` 下 `HAL_IncTick` 调用 `Timer1msProFunction()` | **否**（仅追加 1ms 任务；`interrupt_ADC1→HandleFast` 未改） |
| `Core/Src/stm32g4xx_it.c` | APP 槽 USART1 IDLE 走 `UartPort_ProcessIdleIrq` | 否（通信入口） |
| `AppUser/MODBUS_SLAVE.c/h` | APP 槽 RX 由 UartPort 接管；`com_0x03` 增加 OTA 安全关断 `0x80` 位 | **否**（通信/关断钩子，不改 PID） |
| `User/*`（UartPort、INTO、Flash 元数据等） | 自 xp 拷贝 | 否 |
| `Service/pfc_safe/*` | 自 xp 拷贝；`PfcApp_PrepareForOta` 调 `PwmClose/RelayOff` | **间接**（OTA 前关 PWM，正常运行不触发） |
| `User/FlashMemoryMap.h` | 槽地址表；`DF_APP_MIN_BIN_LENGTH_BYTES=34000` | 否 |
| `MDK-ARM/xpPFC431_48/APP_*.sct` | IROM 偏移 | 否 |
| `MDK-ARM/xpPFC431_48.uvprojx` | 新增 APP_A/B Target；`useUlib=0` | 否 |

### 2.3 功率相关文件——**未被 IAP 补丁修改**

以下文件在 R02 与 xp 的差异 **来自 R02 产品基线**，不是本次 IAP 移植引入：

| 模块 | 路径 | 与 xp 关系 | 对功率的影响 |
|------|------|-----------|--------------|
| 快环（电流环） | `Math_Lib/lib/math_pfc.lib` → `HandleFast()` | 同机制，预编译库 | **无 IAP 改动** |
| 慢环状态机 | `AppUser/ConsoleSlow.c` → `StateMan()` | R02 基线较 xp 简化 | R02 原有差异 |
| 电压环 | `AppUser/ConsoleMid.c` → `HandleSlow()` | R02 无 `vBusContrl()` | R02 原有差异 |
| ADC 采样 | `AppUser/user_sample.c` | R02 无 xp 温度/辅源写入 `DataFlowFace` | R02 原有差异 |
| PWM 驱动 | `ConsoleSlow.c` → `SHRTIMERdrive()`（实际 TIM1） | 与 xp 相同 | 无 IAP 改动 |
| 算法头文件 | `pfc/controlAlgorithm.h` | 与 xp 宏/结构一致 | 无 IAP 改动 |
| 硬件阈值 | `AppUser/HwConfig.h` | R02 `DataFlow_t` 字段较少 | R02 原有差异 |
| ADC/DMA 初始化 | `Core/Src/main.c` MX_ADC/TIM | 与 xp 相同 | 无 IAP 改动 |

**功率处理调用链（APP_A/B 与量产相同）：**

```
TIM2 触发 ADC → DMA 完成 → interrupt_ADC1()
  → ADC0_Sample() + HandleFast()（math_pfc.lib）
  → SHRTIMERdrive()（TIM1 PWM）

5ms 分片 → StateMan() / VIacRmsCal() / HandleSlow()（ConsoleMid）
```

IAP 仅在并行路径上增加：

```
HAL_IncTick (1ms) → Timer1msProFunction() → UartRxMsgProTask（INTO 排队）
main while(1)     → AppIntoUpgrade_MainLoopTask()（Flash 擦写，仅 INTO 后）
```

---

## 三、功率部分专项检测

### 3.1 OTA 安全关断对功率的影响

`Service/pfc_safe/PfcSafeAppBridge.c` 与 xp **一致**：

- `PfcApp_RequestSafeShutdown()`：`PwmClose()` + `RelayOff()` + `PfcHw_SafeShutdown()` → `RunState=Wait`
- `PfcApp_PrepareForOta()`：等待安全态（最多 3s）后同样关断

**触发条件**（非正常运行）：

1. LLC 经 Modbus `Stuts & 0x80` 请求（`MODBUS_SLAVE.c` → `com_0x03_Callback`）
2. Boot/INTO 流程进入 `AppIntoUpgrade_MainLoopTask`

**结论**：正常运行时功率环不受影响；OTA 前关断是 **预期行为**，与 xp 一致。

### 3.2 通信层差异是否影响功率控制

| 项 | xpPFC1600 | R02 | 是否影响闭环 |
|----|-----------|-----|--------------|
| LLC↔PFC 协议 | 12B 对等帧（`llc_id=0xA1`） | 8B Modbus 主站帧（`01 03 ...`） | **不改变** PID，只改变 LLC 如何读状态 |
| 母线跟随 | `ConsoleMid` 有 `vBusContrl()` | R02 固定 `HW_VBUS_SET`，`vBusContrl` 已注释 | **R02 基线行为**，非 IAP 引入 |
| LLC 通信看门狗 | `overTime_llc()` → `SCI_Err` | R02 无 | R02 基线缺失 |
| Modbus 遥测映射 | xp 打包完整 `DataFlowFace` | R02 `McuData_To_RegData()` **仅 6 字段** | **不影响 PFC 自身控制**，但 LLC 读不到完整遥测 |

### 3.3 功率相关风险项（与 IAP 的间接关系）

| 风险 | 说明 | 严重度 |
|------|------|--------|
| OTA 关断超时 | LLC 等 `PfcState==0` 超时 → CAN 升级失败 | 中（升级链） |
| Modbus 寄存器不全 | LLC 无法准确判断 PFC 温度/辅源 | 低（功率环本身） |
| bin 被截断至 36800 | 34000 门槛可过 Boot，但代码可能不完整 → HardFault | **高**（若用截断 bin） |
| `math_pfc.lib` 未入库 | 换机无法重建 APP | 中（工程维护） |

---

## 四、IAP 移植完成度检查表

### 4.1 从机 PFC（R02）

| # | 检查项 | 状态 | 证据 |
|---|--------|------|------|
| 1 | Boot 30KB + AB 槽 scatter | ✅ | `IAP Bootloader.sct` `0x7800`；`APP_A/B.sct` |
| 2 | `FlashMemoryMap.h` 双端同步 | ✅ | APP + Boot 均为 `34000` |
| 3 | INTO 0x8500 延迟 ACK 模式 | ✅ | `Uart_Rec_Analysis.c` 与 xp 一致 |
| 4 | main 循环 Flash 任务 | ✅ | `main.c` `AppIntoUpgrade_MainLoopTask()` |
| 5 | UartPort IAP/Modbus 分流 | ✅ | `UartPort.c` 8B Modbus vs 长帧 IAP |
| 6 | PfcSafe OTA 关断 | ✅ | `PfcSafeAppBridge.c` 与 xp 一致 |
| 7 | USART1 115200 | ✅ | `main.c` / `MODBUS_SLAVE.h` |
| 8 | Keil APP_A/B 0 Error 编译 | ✅ | `APP_A.build_log.htm`：Code=34142，0 Error |
| 9 | `useUlib=0`（标准库） | ✅ | `uvprojx` APP_A/B 均为 0 |
| 10 | 端到端 CAN OTA 联调 | ❓ 未验收 | 日志曾出现 Boot `bin too small` / B5 `0x02` |
| 11 | 合包工具 ≥34000 预检 | ⚠️ 未做 | `MergedPackBuilder` 仍仅 ≥10KB |
| 12 | `UPGRADE_DBG` 联调 | ⚠️ 默认关 | `UPGRADE_DBG_ENABLE=0` |

### 4.2 主机 LLC（R02）

| # | 检查项 | 状态 |
|---|--------|------|
| 1 | Modbus 主站 8B + Rev 字节 | ✅ |
| 2 | USART3 IDLE + InitModbusData | ✅ |
| 3 | OTA 网关 CAN→Modbus 安全关断→INTO | ✅ 代码存在 |
| 4 | 与 PFC 115200 对齐 | ✅ |
| 5 | 端到端联调 | ❓ 待验证 |

### 4.3 总体判定

```
[IAP 软件栈]  ████████████████░░  ~85%  （编译/架构 OK，联调未闭环）
[功率控制]    ██████████████████  ~95%  （IAP 未改环；R02 基线差异保留）
[通信对齐]    ████████████░░░░░░  ~70%  （8B Modbus 已对齐；遥测/看门狗未补齐）
[工具链一致]  ██████████████░░░░  ~75%  （34000 已改；合包/文档未完全同步）
```

**移植完成的定义**：不仅代码合入，还需 **Boot+APP_A/B 烧录 + 完整 bin 合包 + CAN OTA 成功 + 重启运行**。当前停在「代码与编译 OK，联调曾失败」阶段。

---

## 五、升级链路架构（R02 实际路径）

```
PC / 上位机
    │ CAN FD（合包：LLC段 + PFC段）
    ▼
R02 LLC（APP_A2 / CAN 网关）
    │ ① Modbus 8B：Stuts|0x80 请求 PFC 安全关断
    │ ② 轮询 PFC Modbus 响应 PfcState==0
    │ ③ RS485 UART4 → PFC USART1：IAP 长帧（0x8500 INTO / 0x10 下载）
    ▼
R02 PFC（Boot @0x08000000 或 APP_A/B 槽）
    │ UartPort 分流：8B→Modbus；长帧→IAP
    │ INTO → PfcApp_PrepareForOta → ACK → 复位进 Boot → RS485 收 bin
    ▼
Flash 非活动槽写入 → CRC → 切换活动槽 → 跳转新 APP
```

**与 xp 的关键差异**：xp 全程 12B LLC 协议 + 相同 RS485 IAP；R02 日常通信是 **R02 2800W Modbus**，IAP 长帧与之 **共用 USART1**，因此必须做 UartPort 分流。

---

## 六、烧录与编译手册

### 6.1 编译

1. 打开 `MDK-ARM/xpPFC431_48.uvprojx`
2. 选择 **APP_A** 或 **APP_B** → **Rebuild**（建议 Clean 后全量编译）
3. 产物：`MDK-ARM/xpPFC431_48/APP_A.hex` + `APP_A.bin`
4. 量产固件用 **xpPFC431_48** Target（无 IAP，**勿用于 OTA 槽**）

**当前 APP_A 规模**（2026-06-13 编译日志）：

- Code + RO ≈ **36752 B**（map 总 ROM ≈ 36912 B）
- 下限 `DF_APP_MIN_BIN_LENGTH_BYTES = **34000**`（Boot 同步）
- **禁止**使用手动截断的 36800 B bin（曾触发 Boot `0xD107`）

### 6.2 烧录顺序

| 顺序 | 组件 | 地址 | 说明 |
|------|------|------|------|
| 1 | IAP Bootloader | `0x08000000` | 必须先烧 |
| 2 | APP_A 或 APP_B | `0x08008000` / `0x08013000` | 与 scatter 一致 |
| 3 | 元数据区 | Boot 管理 | 首次 OTA 后由 Boot 写入 |

### 6.3 合包与升级

- 合包输入：**完整** `APP_A.bin` + `APP_B.bin`（各 ≥34000，推荐用 Keil 产出全文）
- 主机 CAN 升级工具：`CANFD_IAP_GC` / `CAN1FD升级`
- 失败码：`B5 d[1]=0x02` = 主机侧 CAN 升级失败（含 PFC RS485 阶段 Boot 拒收）

### 6.4 联调建议顺序

1. 串口助手直连 PFC USART1：8B Modbus 通断 + 长帧 IAP 不互相干扰
2. LLC↔PFC 115200 互通：`PfcState` 能随关断变 0
3. 仅 RS485 INTO（不进 CAN）验证 Boot 收 bin
4. 完整 CAN 合包 OTA
5. 可选：`UPGRADE_DBG_ENABLE=1`，USART3 PB9 115200 看 Boot/INTO 日志

---

## 七、已知问题与排查索引

| 现象 | 根因 | 处理 |
|------|------|------|
| Boot `[DUAL_SLOT] bin too small len=36800 min=38000` | bin 短于 Boot 下限 | 用完整 bin；Boot/FlashMemoryMap 已改 34000，需 **重编译烧 Boot** |
| Keil `__initial_sp` / `__use_two_region_memory` | APP_A 曾开 MicroLib 与 scatter 冲突 | `useUlib=0` + Clean Rebuild |
| `math_pfc.lib` duplicate | 工程重复添加库 | 只保留 AppUser 组一处 |
| B5 `0x02` | RS485 阶段失败（非 INTO ACK） | 查 Boot 日志 err/upState |
| OTA 后 HardFault | 截断 bin 或向量不在槽内 | 用完整 APP_A/B.bin |
| LLC 等不到 PfcState=0 | Modbus 映射仅 6 字段或关断未执行 | 查 `com_0x03` 0x80 与 `McuData_To_RegData` |

---

## 八、为什么移植这么难？（根因分析）

### 8.1 不是「拷文件」，而是「三套系统对齐」

本次移植同时要满足：

1. **R02 产品基线**（2800W Modbus 从机、固定母线、简化 ConsoleSlow）
2. **xpPFC1600 IAP 参考**（双槽 Boot、INTO 时序、PfcSafe、UartPort）
3. **R02 LLC 主机**（8B Modbus 主站、CAN OTA 网关）

三者 **协议不同、历史不同、bin 大小不同**。任何只按 xp 文档逐步拷贝的步骤，都会撞上 R02 特有分支。

### 8.2 约束叠加（每层只查一部分）

| 层级 | 检查什么 | 不检查什么 | 典型坑 |
|------|----------|------------|--------|
| 合包工具 | ≥10KB、向量在槽内 | Boot 34000/38000 | 36800 合包合法但 Boot 拒收 |
| CAN/RS485 传输 | 帧数、CRC | bin 完整性 | 传输成功但 Boot 失败 |
| Keil 链接 | 0 Error | 与 xp ROM 大小 | 截断/缺符号的 bin |
| Boot | min length、CRC、槽切换 | 上位机合包规则 | 最后一关才报错 |
| 运行时 | Modbus + IAP 共存 | 功率环 | IDLE 分流错误吞帧 |

**失败往往出现在最后一层**，前面全绿，容易误判为「RS485 坏了」或「功率被改坏了」。

### 8.3 工具链与工程配置的隐性依赖

- **Scatter + MicroLib + startup** 三者必须一致（曾出现 `useUlib=1` 链错）
- **`math_pfc.lib` 预编译** 不在 git 中 → 换环境编译结果不同
- **多 Target 共用 uvprojx** → Cube 重生成易覆盖 Target
- **DF_APP_MIN_BIN** 在 APP 与 Boot 两处定义 → 只改一边会 silent fail

### 8.4 通信与功率耦合在 OTA 路径上交汇

正常运行：Modbus 与功率环 **解耦**。  
OTA 路径：LLC 必须先 Modbus 关断 PFC → 再发 IAP → Boot 写 Flash → 跳转 APP。

任一环节（`PfcState` 映射、115200、UartPort 分流、INTO ACK 时序）失败，表现都是 **升级失败**，排查面跨 **主机+从机+Boot+工具**，体感「怎么又错了」。

### 8.5 文档与代码多版本并行

仓库内同时存在：

- `AB双槽IAP移植说明书.md`（38000）
- `R02_AB双槽IAP移植说明.md`（操作向）
- xp 参考工程（38000、12B 协议）
- R02 实际（34000、8B Modbus）

**阈值、协议、Target 名称** 不一致时，按错文档操作必出问题。

### 8.6 小结：难在哪里

| 类型 | 说明 |
|------|------|
| **架构难** | 双槽 IAP + 双协议 UART + CAN 网关 |
| **对齐难** | R02 / xp / LLC 三套协议与数据结构 |
| **验证难** | 失败点分散在 5 层，日志需对照 Boot err |
| **维护难** | 多 Target、预编译库、Boot 与 APP 双工程 |
| **不是功率算法难** | 功率环基本未动；问题集中在 **工程配置、长度门槛、通信对齐** |

---

## 九、后续建议（不改代码，仅流程）

1. **Boot 重烧**：确认 `DF_APP_MIN_BIN=34000` 已生效  
2. **禁止截断 bin**：合包只用 Keil 完整 `APP_A.bin` / `APP_B.bin`  
3. **联调顺序**：Modbus 关断 → INTO → 全 CAN OTA  
4. **文档统一**：合包手册中的 38000 改为 34000，并注明 R02 8B 协议  
5. **验收标准**：写清「移植完成 = OTA 成功 + 新 APP 功率正常运行 30min」  

---

## 十、关联文档

| 文档 | 路径 |
|------|------|
| R02 操作向移植说明 | `docs/R02_AB双槽IAP移植说明.md` |
| 通用 AB 双槽说明书 | `../docs/AB双槽IAP移植说明书.md` |
| INTO 联调经验 | `../docs/INTO升级联调经验总结.md` |
| xp 移植方案 | `../docs/xpPFC431移植到xpPFC1600方案.md` |
| 上位机手册 | `../../docs/上位机升级工具使用手册.md` |

---

*本报告基于仓库静态分析与既有编译日志，未执行现场 OTA 复测。*
