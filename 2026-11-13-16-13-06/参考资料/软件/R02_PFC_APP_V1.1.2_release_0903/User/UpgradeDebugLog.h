/******************************************************************************
    UpgradeDebugLog.h — APP 侧升级调试打印
    输出：USART3 PB9（printf 重定向），与 USART1 IAP/LLC 无关
    用法：
      - UPGRADE_DBG_ENABLE=1 时编译打印并初始化 USART3
      - Keil 预定义宏 UPGRADE_DBG_ENABLE=1 亦可
      - 调试口接 PB9，115200 8N1
******************************************************************************/
#ifndef UPGRADE_DEBUG_LOG_H
#define UPGRADE_DEBUG_LOG_H

#include <stdio.h>

#ifndef UPGRADE_DBG_ENABLE
#define UPGRADE_DBG_ENABLE  0
#endif

#if UPGRADE_DBG_ENABLE
#define UPGRADE_DBG(fmt, ...)   printf((fmt), ##__VA_ARGS__)
#else
#define UPGRADE_DBG(fmt, ...)   ((void)0)
#endif

void UpgradeDebugLog_Init(void);

#endif /* UPGRADE_DEBUG_LOG_H */
