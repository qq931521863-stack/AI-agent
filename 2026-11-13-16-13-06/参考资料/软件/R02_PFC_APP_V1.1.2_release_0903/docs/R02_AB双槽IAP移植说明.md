# R02_PFC_APP_V1.0.1 AB 双槽 IAP 移植说明

> **源工程（旧）**：`xpPFC1600_v1.0.3_release_6.6`  
> **目标工程（新）**：`R02_PFC_APP_V1.0.1`  
> **原则**：只移植 OTA/IAP 升级能力，**不改动 R02 量产 Target 的 PFC/Modbus 业务逻辑**

---

## 1. Keil Target 与 IROM / Scatter

| Target | 用途 | IROM 起始 | 大小 | Scatter | 预定义宏 | 输出 |
|--------|------|-----------|------|---------|----------|------|
| `xpPFC431_48` | **量产（保持不变）** | `0x08000000` | 128KB | 默认 | `STM32G431xx` | `math_pfc.hex` |
| `APP_A` | OTA 槽位 A | `0x08008000` | 44KB | `APP_A.sct` | `APP_SLOT_A_BUILD` | `APP_A.hex` |
| `APP_B` | OTA 槽位 B | `0x08013000` | 44KB | `APP_B.sct` | `APP_SLOT_B_BUILD` | `APP_B.hex` |

Flash 分区与 `User/FlashMemoryMap.h` 一致（Boot 30KB + SysStatus 2KB + A/B 各 44KB + 元数据）。

---

## 2. 新增/拷贝的源码目录

从 `xpPFC1600_v1.0.3_release_6.6` 拷贝：

```
User/                  # upgrade_app, UartPort, Timer_pro, INTO 延迟处理等
User/UartUpgrade/
Service/pfc_safe/      # PfcSafeHw / PfcSafeAppBridge
Service/Flash_Internal/
Service/CRC/
Driver/USART/          # usart3_debug（UPGRADE_DBG 可选）
MDK-ARM/xpPFC431_48/APP_A.sct
MDK-ARM/xpPFC431_48/APP_B.sct
```

**未加入工程**：`mx25L1606E.c`（外部 SPI Flash，本机 IAP 不需要）。

---

## 3. R02 侧最小集成（条件编译）

所有升级钩子仅在 `APP_SLOT_A_BUILD` / `APP_SLOT_B_BUILD` 下生效，**量产 Target 行为不变**。

| 文件 | 改动 |
|------|------|
| `Core/Src/main.c` | `UpgradeAppEarlyInit` / `UpgradeAppInit` / `AppIntoUpgrade_MainLoopTask` |
| `AppUser/ConsoleSlow.c` | `HAL_IncTick` 内 `Timer1msProFunction()` |
| `Core/Src/stm32g4xx_it.c` | APP Target：`UartPort_ProcessIdleIrq`；量产 Target：原 Modbus IDLE 逻辑 |
| `AppUser/MODBUS_SLAVE.c` | APP Target 下 `InitModbusData` 不启 DMA（由 `UartPort_Init` 接管） |
| `User/UartPort.c` | **R02 协议分流**：`0x01+0x03` → 量产 Modbus；`0x10/0x15` → IAP |

---

## 4. USART1 双协议（R02 特化）

R02 量产帧：`01 03 ...`（`SLAVE_Addr` + `com_0x03`）  
IAP 帧：`01 10 ...`（INTO/下载）、`01 15 ...`（读设备信息）

`UartPort_HandleIdleRx` 按功能码分流，避免 IAP 拼帧吞掉量产 Modbus。

---

## 5. INTO 处理（与旧工程一致）

- 1ms 任务：仅 `AppIntoUpgrade_TryQueue`（校验 + pending）
- `while(1)`：`AppIntoUpgrade_MainLoopTask`（PrepareForOta → **ACK** → Flash → reset）
- **禁止**在 SysTick 内 `HAL_Delay` / Flash 擦写 / 软复位

详见：`.cursor/skills/pfc-iap-into-ota/SKILL.md`

---

## 6. 编译与烧录

1. 打开 `MDK-ARM/xpPFC431_48.uvprojx`
2. 选择 **APP_A** 或 **APP_B** Target → Rebuild
3. 烧录对应 `APP_A.hex` / `APP_B.hex` 到 Boot 管理的槽位
4. 量产固件仍用 **xpPFC431_48** Target 编译（无 IAP 宏、无升级源文件链接）

可选调试：Target 预定义 `UPGRADE_DBG_ENABLE=1`，USART3 PB9 115200。

---

## 7. 重新合并 Keil Target（可选）

若 `uvprojx` 被 Cube 覆盖，可运行：

```bash
python MDK-ARM/merge_upgrade_targets.py
```

---

## 8. 关联文档

- `xpPFC431移植到xpPFC1600方案.md`
- `INTO升级联调经验总结.md`
- `AB双槽IAP移植说明书.md`
