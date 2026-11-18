#ifndef _AppSlotValidate_H_
#define _AppSlotValidate_H_
#include "stdint.h"

uint8_t AppSlot_GetBaseFromBlock(uint8_t u8Block, uint32_t *pBase);
uint8_t AppSlot_ImageValidateAtAddr(uint32_t u32SlotBase, uint32_t u32ImageAddr);

#endif
