/**
 * @file max7456.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2021-12-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __MAX7456_H
#define __MAX7456_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief MAX7456 Register & Implementation List
 * 
 *      | Function                                      | Status                                                | R/W
 * @ VM0 --------------------------------------------------------------------------------------------------------------------
 *      | Video Standard Select                         | Support                                               | WO
 *      | Sync Select Mode                              | No, always be [external]                              | -
 *      | Enable Display of OSD Image                   | Support                                               | WO
 *      | Vertical Synchronization of On-Screen Data    | No, always be [immediately]                           | -
 *      | Software Reset Bit                            | Support                                               | WO
 *      | Video Buffer Enable                           | No                                                    | -
 * @ VM1 --------------------------------------------------------------------------------------------------------------------
 *      | Background Mode                               | No                                                    | -
 *      | Background Mode Brightness                    | No                                                    | -
 *      | Blinking Time                                 | No                                                    | -
 *      | Blinking Duty Cycle                           | No                                                    | -
 * @ HOS --------------------------------------------------------------------------------------------------------------------
 *      | Horizontal Position Offset                    | Support                                               | WO
 * @ VOS --------------------------------------------------------------------------------------------------------------------
 *      | Vertical Position Offset                      | Support                                               | WO
 * @ DMM --------------------------------------------------------------------------------------------------------------------
 *      | Operation Mode Selection                      | Support                                               | WO
 *      | Local Background Control Bit                  | No                                                    | -
 *      | Blink Bit                                     | No                                                    | -
 *      | Invert Bit                                    | No                                                    | -
 *      | Clear Display Memory                          | Support                                               | WO
 *      | Vertical Sync Clear                           | No, always be [immediately]                           | -
 *      | Auto-Increment Mode                           | Support                                               | WO
 * @ DMAH --------------------------------------------------------------------------------------------------------------------
 *      | Byte Selection Bit                            | No                                                    | -
 *      | Display Memory Address Bit 8                  | Support                                               | WO
 * @ DMAL --------------------------------------------------------------------------------------------------------------------
 *      | Display Memory Address Bits 7–0               | Support                                               | WO
 * @ DMDI --------------------------------------------------------------------------------------------------------------------
 *      | Display Memory Data In                        | Part, only support `Display Area`                     | WO
 * @ CMM --------------------------------------------------------------------------------------------------------------------
 *      | Writing to NVM                                | Support                                               | WO
 *      | Reading from NVM                              | Support                                               | WO
 * @ CMAH --------------------------------------------------------------------------------------------------------------------
 *      | Character Memory Address Bits                 | Support                                               | WO
 * @ CMAL --------------------------------------------------------------------------------------------------------------------
 *      | Character Memory Address Bits                 | Support                                               | WO
 * @ CMDI --------------------------------------------------------------------------------------------------------------------
 *      | Character Memory Data In                      | Support                                               | WO
 * @ OSDM --------------------------------------------------------------------------------------------------------------------
 *      | OSD Rise and Fall Time                        | No                                                    | RW
 *      | OSD Insertion Mux Switching Time              | No                                                    | RW
 * @ RB(x) --------------------------------------------------------------------------------------------------------------------
 *      | Character Black Level                         | No                                                    | -
 *      | Character White Level                         | No                                                    | -
 * @ OSDBL --------------------------------------------------------------------------------------------------------------------
 *      | OSD Image Black Level Control                 | No                                                    | -
 *      | Factory preset                                | No                                                    | -
 * @ STAT --------------------------------------------------------------------------------------------------------------------
 *      | Reset Mode                                    | No                                                    | -
 *      | Character Memory Status                       | No                                                    | -
 *      | VSYNC Output Level                            | No                                                    | -
 *      | HSYNC Output Level                            | No                                                    | -
 *      | Loss-of-Sync (LOS)                            | No                                                    | -
 *      | NTSC signal detected status                   | No                                                    | -
 *      | PAL signal detected status                    | No                                                    | -
 * @ DMDO --------------------------------------------------------------------------------------------------------------------
 *      | Display Memory Data Out                       | No                                                    | -
 * @ CMDO --------------------------------------------------------------------------------------------------------------------
 *      | Character Memory Data Out                     | No                                                    | -
 * @ DVP* --------------------------------------------------------------------------------------------------------------------
 *      | Dynamic VTx Power                             | Support                                               | WO
 * 
 * *DVP register can when define DYNAMIC_VTX_SPI_SUPPORT marco in max7456_register.h
 */

typedef enum {
    MAX7456_STATTYPE_RESET,
    MAX7456_STATTYPE_MEM,
    MAX7456_STATTYPE_VSYNC,
    MAX7456_STATTYPE_HSYNC,
    MAX7456_STATTYPE_LOS,
    MAX7456_STATTYPE_NTSC,
    MAX7456_STATTYPE_PAL,
} max7456StatusType_t;

void MAX7456_Init(void);
uint8_t MAX7456_SpiDataHandler(uint8_t dat);
void MAX7456_ResetDataHandler(void);

void MAX7456_Reset(void);
void MAX7456_ClearDisplayMemory(void);
void MAX7456_ClearNVMWriteFlag(void);
void MAX7456_SetStatusExternal(max7456StatusType_t type, bool state);

uint32_t MAX7456_GetOutputPixelAddress(void);
uint8_t MAX7456_GetOutputPixelNumber(void);
uint8_t MAX7456_GetVideoStandard(void);
bool MAX7456_GetRegisterChange(void);
bool MAX7456_GetResetStatus(void);
bool MAX7456_GetEnableStatus(void);
bool MAX7456_GetClearDisplay(void);
bool MAX7456_GetWriteLogoFlag(void);
bool MAX7456_GetWriteNVMFlag(void);
uint32_t MAX7456_GetNVMAddress(void);
uint8_t *MAX7456_GetNVMDataPoint(void);
uint8_t MAX7456_GetDynamicVTxPower(void); // Only using for dynamic vtx power (#defined DYNAMIC_VTX_SPI_SUPPORT - max7456_register.h)

void MAX7456_ReloadOutput(void);
void MAX7456_OutputSingleLine(uint16_t line);

#endif /* __MAX7456_H */
