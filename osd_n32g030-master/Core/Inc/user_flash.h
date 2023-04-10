/**
 * @file user_flash.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2022-03-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __USER_FLASH_H
#define __USER_FLASH_H

#include "n32g030.h"

#define __EXT_FLASH_BACKUP // *use those macro to separate character/logo storage area
#ifdef __EXT_FLASH_BACKUP
    #define SPC_BK_PAGE     ((uint32_t)(0x08007000 - FLASH_START_ADDR) / FLASH_PAGE_SIZE)
    #define SPC_BK_OFFSET   ((uint32_t)0x000000E0)
#endif

#define FLASH_START_ADDR    ((uint32_t)0x08000000)
#define FLASH_END_ADDR      ((uint32_t)0x08008000)

#define FLASH_PAGE_SIZE     ((uint16_t)0x200)

#define FLASH_START_PAGE    ((uint16_t)0)
#define FLASH_END_PAGE      ((uint16_t)((FLASH_END_ADDR - FLASH_START_ADDR) / FLASH_PAGE_SIZE))

bool UserFlash_ModifyRomData(uint32_t address, uint8_t *dat, uint16_t length);

void UserFlash_StartContinuousWrite(uint16_t page);
void UserFlash_StopContinuousWrite(void);
bool UserFlash_WriteContinuousRomData(uint8_t *dat, uint16_t length);

#endif /* __USER_FLASH_H */
