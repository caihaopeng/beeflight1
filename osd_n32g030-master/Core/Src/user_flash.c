/**
 * @file user_flash.c
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief
 * @version 0.1
 * @date 2022-03-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <string.h>

#include "user_flash.h"

bool UserFlash_ModifyRomData(uint32_t address, uint8_t *dat, uint16_t length)
{
    uint32_t backup_buffer[FLASH_PAGE_SIZE / 4];
    uint32_t temp_val;

    uint16_t opt_page = (address - FLASH_START_ADDR) / FLASH_PAGE_SIZE;
    uint16_t end_page = ((address + length) - FLASH_START_ADDR) / FLASH_PAGE_SIZE;

    uint8_t *src_dat;
    uint8_t *dst_dat;

    if ((length == NULL) || (end_page > FLASH_END_PAGE)) {
        return false;
    }

    do {
        /* step 1: backup flash page */
        src_dat = (uint8_t *)(uint32_t)(opt_page * FLASH_PAGE_SIZE + FLASH_START_ADDR);
        dst_dat = (uint8_t *)backup_buffer;
        for (uint16_t backupLoop = FLASH_PAGE_SIZE; backupLoop != NULL; backupLoop--) {
            *(dst_dat++) = *(src_dat++);
        }

        /* step 2: overwrite page data */
        temp_val = address - ((opt_page * FLASH_PAGE_SIZE) + FLASH_START_ADDR); // over-write-data(on this page) location
        dst_dat = (uint8_t *)backup_buffer + temp_val;
        if ((temp_val + length) < FLASH_PAGE_SIZE) { // last page
            temp_val = length;
            for (uint16_t overWriteLoop = temp_val; overWriteLoop != NULL; overWriteLoop--) {
                *(dst_dat++) = *(dat++);
            }
        } else { // cross page
            temp_val = FLASH_PAGE_SIZE - temp_val; // calualate how many byte for this page
            for (uint16_t overWriteLoop = temp_val; overWriteLoop != NULL; overWriteLoop--) {
                *(dst_dat++) = *(dat++);
            }
            address += temp_val;
            length -= temp_val;
        }

        /* step 3: unlocks the flash */
        FLASH_Unlock();

        /* step 4: erase flash page */
        while (FLASH_COMPL != FLASH_EraseOnePage((opt_page * FLASH_PAGE_SIZE) + FLASH_START_ADDR));

        /* step 5: write back page data to flash */
        for (uint16_t writeBackLoop = 0; writeBackLoop < FLASH_PAGE_SIZE; writeBackLoop += 4) {
            while (FLASH_COMPL != FLASH_ProgramWord(((opt_page * FLASH_PAGE_SIZE) + FLASH_START_ADDR) + writeBackLoop, backup_buffer[writeBackLoop / 4]));
        }

        /* step 6: lock flash */
        FLASH_Lock();

        opt_page += 1U;

    } while (opt_page <= end_page);

    return true;
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
static struct
{
    bool isPageErased;
    uint16_t optPage;

    union {
        uint32_t u32Word;
        uint8_t u8Byte[4];
    } tempDataWord;
    uint8_t byteCounter;
    uint16_t wordCounter;

} flashContinuousPar_s;

#ifdef __EXT_FLASH_BACKUP
uint8_t backupSpcPage[FLASH_PAGE_SIZE - SPC_BK_OFFSET];
#endif

void UserFlash_StartContinuousWrite(uint16_t page)
{
    flashContinuousPar_s.optPage = page;
    FLASH_Unlock();

#ifdef __EXT_FLASH_BACKUP
    for (uint16_t loop = 0; loop < sizeof(backupSpcPage); loop++) {
        backupSpcPage[loop] = *(uint8_t *)((SPC_BK_PAGE * FLASH_PAGE_SIZE + FLASH_START_ADDR + SPC_BK_OFFSET) + loop);
    }
#endif
}

void UserFlash_StopContinuousWrite(void)
{
    if (flashContinuousPar_s.isPageErased) {
        if (flashContinuousPar_s.byteCounter != NULL) {
            do {
                flashContinuousPar_s.tempDataWord.u8Byte[flashContinuousPar_s.byteCounter++] = 0xFF;
            } while (flashContinuousPar_s.byteCounter < 4);
            FLASH_ProgramWord((flashContinuousPar_s.optPage * FLASH_PAGE_SIZE) + FLASH_START_PAGE + (flashContinuousPar_s.wordCounter * 4),
                              flashContinuousPar_s.tempDataWord.u32Word);
        }
        flashContinuousPar_s.byteCounter = 0;
        flashContinuousPar_s.wordCounter = 0;
        flashContinuousPar_s.isPageErased = false;
#ifdef __EXT_FLASH_BACKUP
        if (flashContinuousPar_s.optPage == SPC_BK_PAGE) {
            UserFlash_ModifyRomData(SPC_BK_PAGE * FLASH_PAGE_SIZE + FLASH_START_ADDR + SPC_BK_OFFSET, backupSpcPage, sizeof(backupSpcPage));
        }
#endif

        FLASH_Lock();
    }
}

bool UserFlash_WriteContinuousRomData(uint8_t *dat, uint16_t length)
{
    uint16_t optPageNumber = (length + flashContinuousPar_s.wordCounter * 4 + flashContinuousPar_s.byteCounter) / FLASH_PAGE_SIZE;

    if ((length == NULL) || ((flashContinuousPar_s.optPage + optPageNumber) > FLASH_END_PAGE)) {
        return false;
    }

    do {
        if (!flashContinuousPar_s.isPageErased) {
            FLASH_EraseOnePage(flashContinuousPar_s.optPage * FLASH_PAGE_SIZE + FLASH_START_ADDR);
            flashContinuousPar_s.isPageErased = true;
        }

        while (length--) {
            flashContinuousPar_s.tempDataWord.u8Byte[flashContinuousPar_s.byteCounter++] = *(dat++);

            if (flashContinuousPar_s.byteCounter >= 4) {
                FLASH_ProgramWord((flashContinuousPar_s.optPage * FLASH_PAGE_SIZE) + FLASH_START_ADDR + (flashContinuousPar_s.wordCounter * 4),
                                  flashContinuousPar_s.tempDataWord.u32Word);
                flashContinuousPar_s.wordCounter++;
                flashContinuousPar_s.byteCounter = 0;

                if (flashContinuousPar_s.wordCounter >= (FLASH_PAGE_SIZE / 4)) {
                    flashContinuousPar_s.wordCounter = 0;
                    flashContinuousPar_s.optPage++;
                    flashContinuousPar_s.isPageErased = false;
                    break;
                }
            }
        };
    } while (optPageNumber--);

    return true;
}
