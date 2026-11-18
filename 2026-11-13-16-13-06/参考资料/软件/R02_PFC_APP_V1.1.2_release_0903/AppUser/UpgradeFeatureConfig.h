#ifndef UPGRADE_FEATURE_CONFIG_H
#define UPGRADE_FEATURE_CONFIG_H

/*
 * OTA/IAP 升级功能总开关（PFC 从机 APP_A / APP_B）
 * 1 = 编译并启用 INTO/IAP、双协议 UART、Flash 元数据、设备信息查询等升级逻辑
 * 0 = 裁剪为桩函数，USART1 仅保留 LLC Modbus 从机通信
 *
 * 也可在 Keil 预定义宏中覆盖：OTA_UPGRADE_ENABLE=0 或 OTA_UPGRADE_ENABLE=1
 */
#ifndef NO_BOOT
#ifndef OTA_UPGRADE_ENABLE
#define OTA_UPGRADE_ENABLE  1
#endif
#else
#define OTA_UPGRADE_ENABLE  0
#endif
#endif /* UPGRADE_FEATURE_CONFIG_H */
