/**
 * @file user_encryption.c
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief
 * @version 0.1
 * @date 2022-04-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "user_encryption.h"

#include "main.h"

void User_EncryptionVerify(void)
{
    volatile uint32_t key = 0;
    uint8_t uid[UID_LENGTH];

    key = *(volatile uint32_t *)ENCRYPTION_KEY_MEMORY_ADDR;
    GetUID(uid);

    if (key != 0x00000000) {
        while (key != Encryption_GenerateKeyFromUID(uid, UID_LENGTH));
    } else {
        key = Encryption_GenerateKeyFromUID(uid, UID_LENGTH);

        FLASH_Unlock();
        FLASH_EraseOnePage(ENCRYPTION_KEY_MEMORY_ADDR);
        FLASH_ProgramWord(ENCRYPTION_KEY_MEMORY_ADDR, key);
        FLASH_Lock();
    }
}