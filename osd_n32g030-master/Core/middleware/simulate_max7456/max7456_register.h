/**
 * @file max7456_register.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2021-12-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __MAX7456_REGISTER_H
#define __MAX7456_REGISTER_H

#include <stdbool.h>
#include <stdint.h>

// #define DYNAMIC_VTX_SPI_SUPPORT

/******************************************************************
 * 
 *  MAX7456 Function Enumerate
 * 
 * ****************************************************************/
/**
 * @brief MAX7456 Operation Mask
 * 
 */
typedef enum {
    MAX7456_OP_WRITE = 0x00,
    MAX7456_OP_READ  = 0x80,
} MAX7456_Op_TypeDef;

/**
 * @brief Register Address Map
 */
typedef enum {
    MAX7456_REGADDR_VM0     = 0x00, // Video Mode 0
    MAX7456_REGADDR_VM1     = 0x01, // Video Mode 1
    MAX7456_REGADDR_HOS     = 0x02, // Horizontal Offset
    MAX7456_REGADDR_VOS     = 0x03, // Vertical Offset
    MAX7456_REGADDR_DMM     = 0x04, // Display Memory Mode
    MAX7456_REGADDR_DMAH    = 0x05, // Display Memory Address High
    MAX7456_REGADDR_DMAL    = 0x06, // Display Memory Address Low
    MAX7456_REGADDR_DMDI    = 0x07, // Display Memory Data In
    MAX7456_REGADDR_CMM     = 0x08, // Character Memory Mode
    MAX7456_REGADDR_CMAH    = 0x09, // Character Memory Address High
    MAX7456_REGADDR_CMAL    = 0x0A, // Character Memory Address Low
    MAX7456_REGADDR_CMDI    = 0x0B, // Character Memory Data In
    MAX7456_REGADDR_OSDM    = 0x0C, // OSD Insertion Mux
    MAX7456_REGADDR_RB0     = 0x10, // Row 0 Brightness
    MAX7456_REGADDR_RB1     = 0x11, // Row 1 Brightness
    MAX7456_REGADDR_RB2     = 0x12, // Row 2 Brightness
    MAX7456_REGADDR_RB3     = 0x13, // Row 3 Brightness
    MAX7456_REGADDR_RB4     = 0x14, // Row 4 Brightness
    MAX7456_REGADDR_RB5     = 0x15, // Row 5 Brightness
    MAX7456_REGADDR_RB6     = 0x16, // Row 6 Brightness
    MAX7456_REGADDR_RB7     = 0x17, // Row 7 Brightness
    MAX7456_REGADDR_RB8     = 0x18, // Row 8 Brightness
    MAX7456_REGADDR_RB9     = 0x19, // Row 9 Brightness
    MAX7456_REGADDR_RB10    = 0x1A, // Row 10 Brightness
    MAX7456_REGADDR_RB11    = 0x1B, // Row 11 Brightness
    MAX7456_REGADDR_RB12    = 0x1C, // Row 12 Brightness
    MAX7456_REGADDR_RB13    = 0x1D, // Row 13 Brightness
    MAX7456_REGADDR_RB14    = 0x1E, // Row 14 Brightness
    MAX7456_REGADDR_RB15    = 0x1F, // Row 15 Brightness
    MAX7456_REGADDR_OSDBL   = 0x6C, // OSD Black Level
    MAX7456_REGADDR_STAT    = 0xA0, // Status (read only) *address can be 0xA?
    MAX7456_REGADDR_DMDO    = 0xB0, // Display Memory Data Out (read only) *address can be 0xB?
    MAX7456_REGADDR_CMDO    = 0xC0, // Character Memory Data Out (read only) *address can be 0xC?
#ifdef DYNAMIC_VTX_SPI_SUPPORT
    MAX7456_REGADDR_DVP     = 0xD0, // Dynamic VTx power
#endif
    MAX7456_REGADDR_UNKNOW  = 0xFF, // Unknow Register Address
} MAX7456_RegAddr_TypeDef;

/**
 * @brief Video Standard Select
 */
typedef enum {
    MAX7456_VIDEOSTDSEL_NTSC            = 0,
    MAX7456_VIDEOSTDSEL_PAL             = 1,
} MAX7456_VideoStdSel_TypeDef;

/**
 * @brief Sync Select Mode
 *          - 0x = Autosync select (external sync when LOS = 0 and internal sync when LOS = 1)
 *          - 10 = External
 *          - 11 = Internal
 */
typedef enum {
    MAX7456_SYNCSELMODE_AUTOSYNC0       = 0,
    MAX7456_SYNCSELMODE_AUTOSYNC1       = 1,
    MAX7456_SYNCSELMODE_INTERNAL        = 2,
    MAX7456_SYNCSELMODE_EXTERNAL        = 3,
} MAX7456_SyncSelMode_TypeDef;

/**
 * @brief Vertical Synchronization of On-Screen Data
 *        Or Vertical Sync Clear
 *          - 0 = Enable on-screen(clear) display immediately
 *          - 1 = Enable on-screen(clear) display at the next VSYNC
 */
typedef enum {
    MAX7456_DISPLAYSYNC_IMMEDIATELY     = 0,
    MAX7456_DISPLAYSYNC_NEXTVSYNC       = 1,
} MAX7456_DisplaySync_TypeDef;

/**
 * @brief Background Mode
 *          - 0 = The Local Background Control bit (see DMM[5] and DMDI[7]) sets the state of each character background.
 *          - 1 = Sets all displayed background pixels to gray. The gray level is specified by bits VM1[6:4] below. 
 *                This bit overrides the local background control bit.
 */
typedef enum {
    MAX7456_BACKGROUNDMODE_SEPARATE     = 0,
    MAX7456_BACKGROUNDMODE_ALLGRAY      = 1,
} MAX7456_BackgroundMode_TypeDef;

/**
 * @brief Background Mode Brightness (% of OSD White Level)
 */
typedef enum {
    MAX7456_BACKGROUNDBRIGHTNESS_0      = 0,
    MAX7456_BACKGROUNDBRIGHTNESS_7      = 1,
    MAX7456_BACKGROUNDBRIGHTNESS_14     = 2,
    MAX7456_BACKGROUNDBRIGHTNESS_21     = 3,
    MAX7456_BACKGROUNDBRIGHTNESS_28     = 4,
    MAX7456_BACKGROUNDBRIGHTNESS_35     = 5,
    MAX7456_BACKGROUNDBRIGHTNESS_42     = 6,
    MAX7456_BACKGROUNDBRIGHTNESS_49     = 7,
} MAX7456_BackgroundBrightness_TypeDef;

/**
 * @brief Blinking Time (BT)
 *          - 00 = 2 fields (33ms in NTSC mode, 40ms in PAL mode)
 *          - 01 = 4 fields (67ms in NTSC mode, 80ms in PAL mode)
 *          - 10 = 6 fields (100ms in NTSC mode, 120ms in PAL mode)
 *          - 11 = 8 fields (133ms in NTSC mode, 160ms in PAL mode)
 */
typedef enum {
    MAX7456_BLINKINGTIME_2F             = 0,
    MAX7456_BLINKINGTIME_4F             = 1,
    MAX7456_BLINKINGTIME_6F             = 2,
    MAX7456_BLINKINGTIME_8F             = 3,
} MAX7456_BlinkingTime_TypeDef;

/**
 * @brief Blinking Duty Cycle (On : Off)
 *          - 00 = BT : BT
 *          - 01 = BT : (2 x BT)
 *          - 10 = BT : (3 x BT)
 *          - 11 = (3 x BT) : BT
 */
typedef enum {
    MAX7456_BLINKINGDUTY_11             = 0,
    MAX7456_BLINKINGDUTY_12             = 1,
    MAX7456_BLINKINGDUTY_13             = 2,
    MAX7456_BLINKINGDUTY_31             = 3,
} MAX7456_BlinkingDuty_TypeDef;

/**
 * @brief Operation Mode Selection
 */
typedef enum {
    MAX7456_OPERATIONMODE_16B           = 0,
    MAX7456_OPERATIONMODE_8B            = 1,
} MAX7456_OperationMode_TypeDef;

/**
 * @brief Written to or read from the nonvolatile character memory
 *          - 1010 XXXX = Write to NVM array from shadow RAM.
 *          - 0101 XXXX = Read from NVM array into shadow RAM.
 *          - 1010 1100 = (LOGO LOCK) Write to logo zone.
 *          - 1010 1100 = (LOGO LOCK) Read from logo zone.
 */
typedef enum {
    MAX7456_NVMMODE_NONE                = 0x00,
    MAX7456_NVMMODE_WRITE               = 0xA0,
    MAX7456_NVMMODE_WRITE_UNLIMIT       = 0xAC,
    MAX7456_NVMMODE_READ                = 0x50,
    MAX7456_NVMMODE_READ_UNLIMIT        = 0x5C,
} MAX7456_NVMMode_TypeDef;

/**
 * @brief Local Background Control Bit
 *          - 0 = Sets the background pixels of the character to the video input (VIN) when in external sync mode.
 *          - Sets the background pixels of the character to the background mode brightness level defined by VM1[6:4] 
 *            in external or internal sync mode.
 */
typedef enum {
    MAX7456_BACKGROUNDCTRL_EXT          = 0,
    MAX7456_BACKGROUNDCTRL_REG          = 1,
} MAX7456_BackgroundCtrl_TypeDef;

/**
 * @brief Byte Selection Bit
 *          - 0 = Character Address byte is written to or read
 *          - 1 = Character Attribute byte is written to or read 
 */
typedef enum {
    MAX7456_BYTESELECTION_ADDR          = 0,
    MAX7456_BYTESELECTION_ATTR          = 1,
} MAX7456_ByteSelection_TypeDef;

/**
 * @brief OSD Rise and Fall Time(ns)—typical transition times between adjacent OSD pixels
 */
typedef enum {
    MAX7456_EDGETIME_20                 = 0,
    MAX7456_EDGETIME_30                 = 1,
    MAX7456_EDGETIME_35                 = 2,
    MAX7456_EDGETIME_60                 = 3,
    MAX7456_EDGETIME_80                 = 4,
    MAX7456_EDGETIME_110                = 5,
} MAX7456_EdgeTime_TypeDef;

/**
 * @brief OSD Insertion Mux Switching Time(ns)–typical transition times between input video and OSD pixels
 */
typedef enum {
    MAX7456_INSERTTIME_30               = 0,
    MAX7456_INSERTTIME_35               = 1,
    MAX7456_INSERTTIME_50               = 2,
    MAX7456_INSERTTIME_75               = 3,
    MAX7456_INSERTTIME_100              = 4,
    MAX7456_INSERTTIME_120              = 5,
} MAX7456_InsertTime_TypeDef;

/**
 * @brief Character Black Level
 *        — All the characters in row N use these brightness levels for the black pixel, in % of OSD white level.
 */
typedef enum {
    MAX7456_BLACKLEVEL_120              = 0,
    MAX7456_BLACKLEVEL_100              = 1,
    MAX7456_BLACKLEVEL_90               = 2,
    MAX7456_BLACKLEVEL_80               = 3,
} MAX7456_BlackLevel_TypeDef;

/**
 * @brief Character White Level
 *        — All the characters in row N use these brightness levels for the white pixel, in % of OSD white level.
 */
typedef enum {
    MAX7456_WHITELEVEL_0                = 0,
    MAX7456_WHITELEVEL_10               = 1,
    MAX7456_WHITELEVEL_20               = 2,
    MAX7456_WHITELEVEL_30               = 3,
} MAX7456_WhiteLevel_TypeDef;

/**
 * @brief Single pixel color for define character
 */
typedef enum {
    MAX7456_PIXELCOLOR_BLACK            = 0,
    MAX7456_PIXELCOLOR_TRANSPARENT0     = 1,
    MAX7456_PIXELCOLOR_WHITE            = 2,
    MAX7456_PIXELCOLOR_TRANSPARENT1     = 3,
} MAX7456_PixelColor_TypeDef;

#ifdef DYNAMIC_VTX_SPI_SUPPORT
/**
 * @brief Dynamic VTx power level
 */
typedef enum {
    MAX7456_VTX_POWER_5MW               = 0,
    MAX7456_VTX_POWER_25MW              = 1,
    MAX7456_VTX_POWER_MAX               = 2,
} MAX7456_VTxPower_TypeDef;
#endif /* DYNAMIC_VTX_SPI_SUPPORT */

/******************************************************************
 * 
 *  MAX7456 Register Structure
 * 
 * ****************************************************************/

typedef struct max7456Register_s
{
    union {
        uint8_t regByte;
        struct {
            bool video_buffer_enable_n                                          : 1;
            bool software_reset                                                 : 1;
            MAX7456_DisplaySync_TypeDef vertical_sync_onscreen_data             : 1;
            bool enable_display_osd                                             : 1;
            MAX7456_SyncSelMode_TypeDef sync_select_mode                        : 2;
            MAX7456_VideoStdSel_TypeDef video_standard_select                   : 1;
            uint8_t REV                                                         : 1;
        };
    } VM0;

    union {
        uint8_t regByte;
        struct {
            MAX7456_BlinkingDuty_TypeDef blinking_duty_cycle                    : 2;
            MAX7456_BlinkingTime_TypeDef blinking_time                          : 2;
            MAX7456_BackgroundBrightness_TypeDef background_mode_brightness     : 3;
            MAX7456_BackgroundMode_TypeDef background_mode                      : 1;
        };
    } VM1;

    union {
        uint8_t regByte;
        struct {
            uint8_t horizontal_position_offset                                  : 6;
            uint8_t REV                                                         : 2;
        };
    } HOS;

    union {
        uint8_t regByte;
        struct {
            uint8_t vertical_position_offset                                    : 5;
            uint8_t REV                                                         : 3;
        };
    } VOS;

    union {
        uint8_t regByte;
        struct {
            bool auto_increment_mode                                            : 1;
            MAX7456_DisplaySync_TypeDef vertical_sync_clear                     : 1;
            bool clear_display_memory                                           : 1;
            bool invert_bit                                                     : 1;
            bool blink_bit                                                      : 1;
            MAX7456_BackgroundCtrl_TypeDef local_background_control_bit         : 1;
            MAX7456_OperationMode_TypeDef operation_mode_selection              : 1;
            uint8_t REV                                                         : 1;
        };
    } DMM;

    union {
        uint8_t regByte;
        struct {
            uint8_t display_memory_address_bit8                                 : 1;
            MAX7456_ByteSelection_TypeDef byte_selection_bit                    : 1;
            uint8_t REV                                                         : 6;
        };
    } DMAH;

    union {
        uint8_t regByte;
        struct {
            uint8_t display_memory_address_bit70                                : 8;
        };
    } DMAL;

    union {
        uint8_t regByte;
        struct {
            uint8_t display_memory_data                                         : 8;
        };
    } DMDI;

    union {
        uint8_t regByte;
        struct {
            MAX7456_NVMMode_TypeDef character_memory_mode                       : 8;
        };
    } CMM;

    union {
        uint8_t regByte;
        struct {
            uint8_t character_memory_address_bits                               : 8;
        };
    } CMAH;

    union {
        uint8_t regByte;
        struct {
            uint8_t character_memory_address_bits                               : 6;
            uint8_t REV                                                         : 2;
        };
    } CMAL;

    union {
        uint8_t regByte;
        struct {
            MAX7456_PixelColor_TypeDef rightmost_pixel                          : 2;
            MAX7456_PixelColor_TypeDef right_center_pixel                       : 2;
            MAX7456_PixelColor_TypeDef left_center_pixel                        : 2;
            MAX7456_PixelColor_TypeDef leftmost_pixel                           : 2;
        };
    } CMDI;

    union {
        uint8_t regByte;
        struct {
            MAX7456_InsertTime_TypeDef insertion_mux_switching_time             : 3;
            MAX7456_EdgeTime_TypeDef rise_and_fall_time                         : 3;
            uint8_t REV                                                         : 2;
        };
    } OSDM;

    union {
        uint8_t regByte;
        struct {
            MAX7456_WhiteLevel_TypeDef character_white_level                    : 2;
            MAX7456_BlackLevel_TypeDef character_black_level                    : 2;
            uint8_t REV                                                         : 4;
        };
    } RB[15];

    union {
        uint8_t regByte;
        struct {
            uint8_t factory_preset                                              : 4;
            bool image_black_level_control_n                                    : 1;
            uint8_t REV                                                         : 3;
        };
    } OSDBL;

    union {
        uint8_t regByte;
        struct {
            bool pal_detected                                                   : 1;
            bool ntsc_detected                                                  : 1;
            bool loss_of_sync_n                                                 : 1;
            bool hsync_output_level_n                                           : 1;
            bool vsync_output_level_n                                           : 1;
            bool character_memory_status_n                                      : 1;
            bool reset_mode                                                     : 1;
            uint8_t REV                                                         : 1;
        };
    } STAT;

    union {
        uint8_t regByte;
        struct {
            uint8_t display_memory_data                                         : 8;
        };
    } DMDO;

    union {
        uint8_t regByte;
        struct {
            MAX7456_PixelColor_TypeDef rightmost_pixel                          : 2;
            MAX7456_PixelColor_TypeDef right_center_pixel                       : 2;
            MAX7456_PixelColor_TypeDef left_center_pixel                        : 2;
            MAX7456_PixelColor_TypeDef leftmost_pixel                           : 2;
        };
    } CMDO;

#ifdef DYNAMIC_VTX_SPI_SUPPORT
    union {
        uint8_t regByte;
        struct {
            MAX7456_VTxPower_TypeDef power                                      : 2;
            uint8_t REV                                                         : 6;
        };
    } DVP;
#endif /* DYNAMIC_VTX_SPI_SUPPORT */
} max7456Register_t;

/**
 * @brief MAX7456 Register Default Value
 *  *? Unknow value OSDBL[0:3](factory_preset)
 */
#define REG_VM0_DEFAULT_VALUE           (0x00)
#define REG_VM1_DEFAULT_VALUE           (0x47)
#define REG_HOS_DEFAULT_VALUE           (0x20)
#define REG_VOS_DEFAULT_VALUE           (0x10)
#define REG_DMM_DEFAULT_VALUE           (0x00)
#define REG_DMAH_DEFAULT_VALUE          (0x00)
#define REG_DMAL_DEFAULT_VALUE          (0x00)
#define REG_DMDI_DEFAULT_VALUE          (0x00)
#define REG_CMM_DEFAULT_VALUE           (0x00)
#define REG_CMAH_DEFAULT_VALUE          (0x00)
#define REG_CMAL_DEFAULT_VALUE          (0x00)
#define REG_CMDI_DEFAULT_VALUE          (0x00)
#define REG_OSDM_DEFAULT_VALUE          (0x1B)
#define REG_RBN_DEFAULT_VALUE           (0x01)
#define REG_OSDBL_DEFAULT_VALUE         (0x10)
#ifdef DYNAMIC_VTX_SPI_SUPPORT
    #define REG_DVP_DEFAULT_VALUE       (0x00)
#endif

/**
 * @brief Auto-Increment Mode mode is disabled by writing the escape character 1111 1111
 */
#define AUTO_INC_MODE_ESCAPE_CHAR       (0xFF)

#endif /* __MAX7456_REGISTER_H */
