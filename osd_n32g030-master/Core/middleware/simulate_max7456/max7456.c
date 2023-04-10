/**
 * @file max7456.c
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "n32g030.h"
#include <string.h>

#include "max7456.h"
#include "max7456_character.h"
#include "max7456_display.h"
#include "max7456_register.h"

#include "betaflight_2_bold_zip.h"

#define DISPLAY_HOS_OFFSET_BYTE 0
#define DISPLAY_HOS_OFFSET_ROW_PER_BYTE 16

enum { DISPLAY_LINE_BYTE = (uint8_t)(DISPLAY_AREA_COLUMNS * CHARACTER_BYTE_PER_LINES) };

struct
{
    volatile max7456Register_t registers;
    volatile bool isRegisterModify;
    const uint8_t (*pCharacters)[CHARACTER_BYTE_SIZE];
    uint8_t displayCharacterAddress[DISPLAY_AREA_MAX_ROWS][DISPLAY_AREA_COLUMNS];
    // uint8_t displayCharacterAttribute[DISPLAY_AREA_MAX_ROWS][DISPLAY_AREA_COLUMNS];
} max7456Runtime;

struct
{
    bool isRegFill;
    bool isWriteEnable;

    uint8_t regAddr;
    uint8_t lastData;
} max7456SpiData;

struct
{
    uint8_t nvmAddress;
    uint8_t pCharCache[CHARACTER_BYTE_SIZE];
} max7456NvmData;

uint8_t output_line_cache[DISPLAY_LINE_BYTE + DISPLAY_HOS_OFFSET_BYTE + 1]; // ! If not '+1', output will crash
uint8_t pre_output_line_cache[DISPLAY_LINE_BYTE + DISPLAY_HOS_OFFSET_BYTE + 1]; // ! If not '+1', output will crash

static void MAX7456_WriteReg(uint8_t reg, uint8_t dat);
static void MAX7456_ReadReg(uint8_t reg);
static uint8_t *MAX7456_PointDisplayMem(MAX7456_ByteSelection_TypeDef selection);

void MAX7456_Init(void)
{
    MAX7456_Reset();

    max7456Runtime.pCharacters = characters_betaflight_2_bold;
}

uint8_t MAX7456_SpiDataHandler(uint8_t dat)
{
    uint8_t ret = max7456SpiData.lastData;

    if (!max7456SpiData.isRegFill) {
        if (dat == MAX7456_REGADDR_UNKNOW) {
            return NULL;
        }

        if (dat & MAX7456_OP_READ) {
            max7456SpiData.isWriteEnable = false;
        } else {
            max7456SpiData.isWriteEnable = true;
        }
        max7456SpiData.regAddr = dat & (~MAX7456_OP_READ);
        max7456SpiData.isRegFill = true;

    } else {
        if (max7456SpiData.isWriteEnable) {
            MAX7456_WriteReg(max7456SpiData.regAddr, dat);
            max7456SpiData.isWriteEnable = false;
        } else {
            MAX7456_ReadReg(max7456SpiData.regAddr);
        }
        max7456SpiData.isRegFill = false;
    }

    max7456SpiData.lastData = dat;
    return ret;
}

void MAX7456_ResetDataHandler(void)
{
    max7456SpiData.isRegFill = false;
}

static void MAX7456_WriteReg(uint8_t reg, uint8_t dat)
{
    uint16_t temp_val = 0;

    switch (reg) {
    case MAX7456_REGADDR_VM0:
        max7456Runtime.registers.VM0.regByte = dat;
        break;

    case MAX7456_REGADDR_VM1:
        max7456Runtime.registers.VM1.regByte = dat;
        break;

    case MAX7456_REGADDR_HOS:
        max7456Runtime.registers.HOS.regByte = dat;
        break;

    case MAX7456_REGADDR_VOS:
        max7456Runtime.registers.VOS.regByte = dat;
        break;

    case MAX7456_REGADDR_DMM:
        max7456Runtime.registers.DMM.regByte = dat;
        break;

    case MAX7456_REGADDR_DMAH:
        max7456Runtime.registers.DMAH.regByte = dat;
        break;

    case MAX7456_REGADDR_DMAL:
        max7456Runtime.registers.DMAL.regByte = dat;
        break;

    case MAX7456_REGADDR_DMDI:
        max7456Runtime.registers.DMDI.regByte = dat;

        temp_val = (max7456Runtime.registers.DMAH.display_memory_address_bit8 << 8) | max7456Runtime.registers.DMAL.display_memory_address_bit70;

        if (temp_val > (DISPLAY_AREA_MAX_ROWS * DISPLAY_AREA_COLUMNS)) {
            return;
        }

        if (max7456Runtime.registers.DMM.auto_increment_mode) {
            if (dat == AUTO_INC_MODE_ESCAPE_CHAR) {
                return;
            }
            max7456Runtime.registers.DMAH.display_memory_address_bit8 = (uint8_t)((temp_val + 1) >> 8);
            max7456Runtime.registers.DMAL.display_memory_address_bit70 = (uint8_t)((temp_val + 1) & 0x00FF);
        }

        if (max7456Runtime.registers.DMM.operation_mode_selection == MAX7456_OPERATIONMODE_16B) {
            MAX7456_PointDisplayMem(MAX7456_BYTESELECTION_ADDR)[temp_val] = dat;
            // MAX7456_PointDisplayMem(MAX7456_BYTESELECTION_ATTR)[temp_val] = 0x00 |
            //                                                                 (max7456Runtime.registers.DMM.local_background_control_bit << 7) |
            //                                                                 (max7456Runtime.registers.DMM.blink_bit << 6) |
            //                                                                 (max7456Runtime.registers.DMM.invert_bit << 5);
        } else {
            if (max7456Runtime.registers.DMAH.byte_selection_bit == MAX7456_BYTESELECTION_ADDR) { // For limit MAX7456_BYTESELECTION_ATTR area, remove if enable ATTR area
                MAX7456_PointDisplayMem(max7456Runtime.registers.DMAH.byte_selection_bit)[temp_val] = dat;
            }
        }
        break;

    case MAX7456_REGADDR_CMM:
        max7456Runtime.registers.CMM.regByte = dat;
        break;

    case MAX7456_REGADDR_CMAH:
        max7456Runtime.registers.CMAH.regByte = dat;
        max7456NvmData.nvmAddress = max7456Runtime.registers.CMAH.character_memory_address_bits;
        memset(max7456NvmData.pCharCache, 0, sizeof(max7456NvmData.pCharCache));
        break;

    case MAX7456_REGADDR_CMAL:
        max7456Runtime.registers.CMAL.regByte = dat;
        break;

    case MAX7456_REGADDR_CMDI:
        max7456Runtime.registers.CMDI.regByte = dat;
        temp_val = ((max7456Runtime.registers.CMDI.leftmost_pixel == MAX7456_PIXELCOLOR_WHITE) ? (1U) : (0U)) << 3;
        temp_val |= ((max7456Runtime.registers.CMDI.left_center_pixel == MAX7456_PIXELCOLOR_WHITE) ? (1U) : (0U)) << 2;
        temp_val |= ((max7456Runtime.registers.CMDI.right_center_pixel == MAX7456_PIXELCOLOR_WHITE) ? (1U) : (0U)) << 1;
        temp_val |= ((max7456Runtime.registers.CMDI.rightmost_pixel == MAX7456_PIXELCOLOR_WHITE) ? (1U) : (0U)) << 0;
        if ((max7456Runtime.registers.CMAL.character_memory_address_bits % 2) == 0) {
            max7456NvmData.pCharCache[max7456Runtime.registers.CMAL.character_memory_address_bits / 2] |= (uint8_t)(temp_val << 4);
        } else {
            max7456NvmData.pCharCache[max7456Runtime.registers.CMAL.character_memory_address_bits / 2] |= (uint8_t)temp_val & 0x0F;
        }
        break;

    case MAX7456_REGADDR_OSDM:
        max7456Runtime.registers.OSDM.regByte = dat;
        break;

    case MAX7456_REGADDR_RB0:
    case MAX7456_REGADDR_RB1:
    case MAX7456_REGADDR_RB2:
    case MAX7456_REGADDR_RB3:
    case MAX7456_REGADDR_RB4:
    case MAX7456_REGADDR_RB5:
    case MAX7456_REGADDR_RB6:
    case MAX7456_REGADDR_RB7:
    case MAX7456_REGADDR_RB8:
    case MAX7456_REGADDR_RB9:
    case MAX7456_REGADDR_RB10:
    case MAX7456_REGADDR_RB11:
    case MAX7456_REGADDR_RB12:
    case MAX7456_REGADDR_RB13:
    case MAX7456_REGADDR_RB14:
    case MAX7456_REGADDR_RB15:
        max7456Runtime.registers.RB[reg & 0x0F].regByte = dat;
        break;

    case MAX7456_REGADDR_OSDBL:
        max7456Runtime.registers.OSDBL.regByte = dat;
        break;

#ifdef DYNAMIC_VTX_SPI_SUPPORT
    case MAX7456_REGADDR_DVP:
        max7456Runtime.registers.DVP.regByte = dat;
        break;
#endif /* DYNAMIC_VTX_SPI_SUPPORT */

    default:
        break;
    }

    max7456Runtime.isRegisterModify = true;
}

static void MAX7456_ReadReg(uint8_t reg)
{
    switch (reg) { // remove MSB

    case MAX7456_REGADDR_VM0:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.VM0.regByte);
        break;

    case MAX7456_REGADDR_VM1:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.VM1.regByte);
        break;

    case MAX7456_REGADDR_HOS:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.HOS.regByte);
        break;

    case MAX7456_REGADDR_VOS:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.VOS.regByte);
        break;

    case MAX7456_REGADDR_DMM:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.DMM.regByte);
        break;

    case MAX7456_REGADDR_DMAH:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.DMAH.regByte);
        break;

    case MAX7456_REGADDR_DMAL:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.DMAL.regByte);
        break;

    case MAX7456_REGADDR_DMDI:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.DMDI.regByte);
        break;

    case MAX7456_REGADDR_CMM:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.CMM.regByte);
        break;

    case MAX7456_REGADDR_CMAH:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.CMAH.regByte);
        break;

    case MAX7456_REGADDR_CMAL:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.CMAL.regByte);
        break;

    case MAX7456_REGADDR_CMDI:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.CMDI.regByte);
        break;

    case MAX7456_REGADDR_OSDM:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.OSDM.regByte);
        break;

    case MAX7456_REGADDR_RB0:
    case MAX7456_REGADDR_RB1:
    case MAX7456_REGADDR_RB2:
    case MAX7456_REGADDR_RB3:
    case MAX7456_REGADDR_RB4:
    case MAX7456_REGADDR_RB5:
    case MAX7456_REGADDR_RB6:
    case MAX7456_REGADDR_RB7:
    case MAX7456_REGADDR_RB8:
    case MAX7456_REGADDR_RB9:
    case MAX7456_REGADDR_RB10:
    case MAX7456_REGADDR_RB11:
    case MAX7456_REGADDR_RB12:
    case MAX7456_REGADDR_RB13:
    case MAX7456_REGADDR_RB14:
    case MAX7456_REGADDR_RB15:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.RB[reg & 0x0F].regByte);
        break;

    case MAX7456_REGADDR_OSDBL:
        SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.OSDBL.regByte);
        break;

    default:
        switch (reg & 0xF0) {
        case MAX7456_REGADDR_STAT:
            SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.STAT.regByte);
            break;

        case MAX7456_REGADDR_DMDO:
            SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.DMDO.regByte);
            break;

        case MAX7456_REGADDR_CMDO:
            SPI_I2S_TransmitData(SPI2, max7456Runtime.registers.CMDO.regByte);
            break;

        default:
            break;
        }
        break;
    }
}

static uint8_t *MAX7456_PointDisplayMem(MAX7456_ByteSelection_TypeDef selection)
{
    switch (selection) {
    case MAX7456_BYTESELECTION_ADDR:
        return (uint8_t *)max7456Runtime.displayCharacterAddress;

    // case MAX7456_BYTESELECTION_ATTR:
    //     return (uint8_t *)max7456Runtime.displayCharacterAttribute;

    default:
        return NULL;
    }
}

void MAX7456_Reset(void)
{
    max7456Runtime.isRegisterModify = true;

    max7456Runtime.registers.VM0.regByte = REG_VM0_DEFAULT_VALUE;
    max7456Runtime.registers.VM1.regByte = REG_VM1_DEFAULT_VALUE;
    max7456Runtime.registers.HOS.regByte = REG_HOS_DEFAULT_VALUE;
    max7456Runtime.registers.VOS.regByte = REG_VOS_DEFAULT_VALUE;
    max7456Runtime.registers.DMM.regByte = REG_DMM_DEFAULT_VALUE;
    max7456Runtime.registers.DMAH.regByte = REG_DMAH_DEFAULT_VALUE;
    max7456Runtime.registers.DMAL.regByte = REG_DMAL_DEFAULT_VALUE;
    max7456Runtime.registers.DMDI.regByte = REG_DMDI_DEFAULT_VALUE;
    max7456Runtime.registers.CMM.regByte = REG_CMM_DEFAULT_VALUE;
    max7456Runtime.registers.CMAH.regByte = REG_CMAH_DEFAULT_VALUE;
    max7456Runtime.registers.CMAL.regByte = REG_CMAL_DEFAULT_VALUE;
    max7456Runtime.registers.CMDI.regByte = REG_CMDI_DEFAULT_VALUE;
    max7456Runtime.registers.OSDM.regByte = REG_OSDM_DEFAULT_VALUE;
    for (uint8_t i = 0; i < 15; i++) {
        max7456Runtime.registers.RB[i].regByte = REG_RBN_DEFAULT_VALUE;
    }
    max7456Runtime.registers.OSDBL.regByte = REG_OSDBL_DEFAULT_VALUE;
#ifdef DYNAMIC_VTX_SPI_SUPPORT
    max7456Runtime.registers.DVP.regByte = REG_DVP_DEFAULT_VALUE;
#endif

    MAX7456_ClearDisplayMemory();
    max7456Runtime.isRegisterModify = true;

    max7456SpiData.isRegFill = false;
}

void MAX7456_ClearDisplayMemory(void)
{
    memset((void *)max7456Runtime.displayCharacterAddress, 0, sizeof(max7456Runtime.displayCharacterAddress));
    // memset((void *)max7456Runtime.displayCharacterAttribute, 0, sizeof(max7456Runtime.displayCharacterAttribute));
}

void MAX7456_ClearNVMWriteFlag(void)
{
    max7456Runtime.registers.CMM.character_memory_mode = MAX7456_NVMMODE_NONE;
}

void MAX7456_SetStatusExternal(max7456StatusType_t type, bool state)
{
    switch (type) {
    case MAX7456_STATTYPE_RESET:
        max7456Runtime.registers.STAT.reset_mode = state;
        break;

    case MAX7456_STATTYPE_MEM:
        max7456Runtime.registers.STAT.character_memory_status_n = state;
        break;

    case MAX7456_STATTYPE_VSYNC:
        max7456Runtime.registers.STAT.vsync_output_level_n = state;
        break;

    case MAX7456_STATTYPE_HSYNC:
        max7456Runtime.registers.STAT.hsync_output_level_n = state;
        break;

    case MAX7456_STATTYPE_LOS:
        max7456Runtime.registers.STAT.loss_of_sync_n = state;
        break;

    case MAX7456_STATTYPE_NTSC:
        max7456Runtime.registers.STAT.ntsc_detected = state;
        break;

    case MAX7456_STATTYPE_PAL:
        max7456Runtime.registers.STAT.pal_detected = state;
        break;

    default:
        break;
    }
}

uint32_t MAX7456_GetOutputPixelAddress(void)
{
    return (uint32_t)&output_line_cache;
}

uint8_t MAX7456_GetOutputPixelNumber(void)
{
    return sizeof(output_line_cache);
}

/**
 * @brief Get MAX7456 video standard setting
 *
 * @return 1 = NTSC
 *         2 = PAL
 */
uint8_t MAX7456_GetVideoStandard(void)
{
    return ((max7456Runtime.registers.VM0.video_standard_select == MAX7456_VIDEOSTDSEL_NTSC) ? (1) : (2));
}

bool MAX7456_GetRegisterChange(void)
{
    bool tmp = max7456Runtime.isRegisterModify;
    max7456Runtime.isRegisterModify = false;
    return tmp;
}

bool MAX7456_GetResetStatus(void)
{
    bool tmp = max7456Runtime.registers.VM0.software_reset;
    max7456Runtime.registers.VM0.software_reset = false;
    return tmp;
}

bool MAX7456_GetEnableStatus(void)
{
    return max7456Runtime.registers.VM0.enable_display_osd;
}

bool MAX7456_GetClearDisplay(void)
{
    bool tmp = max7456Runtime.registers.DMM.clear_display_memory;
    max7456Runtime.registers.DMM.clear_display_memory = false;
    return tmp;
}

uint8_t MAX7456_GetDynamicVTxPower(void)
{
#ifdef DYNAMIC_VTX_SPI_SUPPORT
    return (uint8_t)max7456Runtime.registers.DVP.power;
#else
    return (uint8_t)NULL;
#endif
}

bool MAX7456_GetWriteLogoFlag(void)
{
    return (max7456Runtime.registers.CMM.character_memory_mode == MAX7456_NVMMODE_WRITE_UNLIMIT) ? (true) : (false);
}

bool MAX7456_GetWriteNVMFlag(void)
{
    return ((max7456Runtime.registers.CMM.character_memory_mode & 0xF0) == MAX7456_NVMMODE_WRITE) ? (true) : (false);
}

uint32_t MAX7456_GetNVMAddress(void)
{
    return max7456NvmData.nvmAddress;
}

uint8_t *MAX7456_GetNVMDataPoint(void)
{
    return max7456NvmData.pCharCache;
}

void MAX7456_ReloadOutput(void)
{
    memcpy(output_line_cache, pre_output_line_cache, sizeof(output_line_cache));
}

void MAX7456_OutputSingleLine(uint16_t line)
{
    uint8_t display_area_row, display_area_col = 0;
    uint8_t character_index, character_row;
    uint8_t *pBuffer;
    const uint8_t FIX_HOS_OFFSET = DISPLAY_HOS_OFFSET_BYTE;

    display_area_row = line / CHARACTER_PIXEL_PER_ROW;
    character_row = (line % CHARACTER_PIXEL_PER_ROW) * CHARACTER_BYTE_PER_LINES;
    pBuffer = pre_output_line_cache + FIX_HOS_OFFSET;

#define _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(COL1, COL2, DEST1, DEST2, DEST3)                                  \
    character_index = max7456Runtime.displayCharacterAddress[display_area_row][COL1]; /* first character */  \
    pBuffer[DEST1] = max7456Runtime.pCharacters[character_index][character_row];                             \
    pBuffer[DEST2] = max7456Runtime.pCharacters[character_index][character_row + 1] & 0xF0;                  \
    character_index = max7456Runtime.displayCharacterAddress[display_area_row][COL2]; /* second character */ \
    pBuffer[DEST2] |= max7456Runtime.pCharacters[character_index][character_row] >> 4;                       \
    pBuffer[DEST3] = max7456Runtime.pCharacters[character_index][character_row] << 4;                        \
    pBuffer[DEST3] |= max7456Runtime.pCharacters[character_index][character_row + 1] >> 4

#define _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(COL1, COL2, DEST1, DEST2, DEST3)                                   \
    character_index = max7456Runtime.displayCharacterAddress[display_area_row][COL1]; /* first character */  \
    pBuffer[DEST1] = max7456Runtime.pCharacters[character_index][character_row] << 4;                        \
    pBuffer[DEST1] |= max7456Runtime.pCharacters[character_index][character_row + 1] >> 4;                   \
    pBuffer[DEST2] = max7456Runtime.pCharacters[character_index][character_row + 1] << 4;                    \
    character_index = max7456Runtime.displayCharacterAddress[display_area_row][COL2]; /* second character */ \
    pBuffer[DEST2] |= max7456Runtime.pCharacters[character_index][character_row] & 0x0F;                     \
    pBuffer[DEST3] = max7456Runtime.pCharacters[character_index][character_row + 1]

    if ((line % 2) == 0) {
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER( 0,  1,    0,  1,  2);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER( 2,  3,    3,  4,  5);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER( 4,  5,    6,  7,  8);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER( 6,  7,    9, 10, 11);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER( 8,  9,   12, 13, 14);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(10, 11,   15, 16, 17);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(12, 13,   18, 19, 20);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(14, 15,   21, 22, 23);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(16, 17,   24, 25, 26);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(18, 19,   27, 28, 29);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(20, 21,   30, 31, 32);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(22, 23,   33, 34, 35);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(24, 25,   36, 37, 38);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(26, 27,   39, 40, 41);
        _DISPLAY_EVEN_LINE_2B_TO_3B_HELPER(28, 29,   42, 43, 44);
    } else {
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER( 0,  1,     0,  1,  2);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER( 2,  3,     3,  4,  5);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER( 4,  5,     6,  7,  8);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER( 6,  7,     9, 10, 11);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER( 8,  9,    12, 13, 14);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(10, 11,    15, 16, 17);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(12, 13,    18, 19, 20);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(14, 15,    21, 22, 23);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(16, 17,    24, 25, 26);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(18, 19,    27, 28, 29);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(20, 21,    30, 31, 32);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(22, 23,    33, 34, 35);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(24, 25,    36, 37, 38);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(26, 27,    39, 40, 41);
        _DISPLAY_ODD_LINE_2B_TO_3B_HELPER(28, 29,    42, 43, 44);
    }
}