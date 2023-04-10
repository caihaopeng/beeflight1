/**
 * @file cvbs.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2021-12-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __CVBS_H
#define __CVBS_H

#include <stdint.h>
#include <stdbool.h>

#define CVBS_CAP_TIM_FREQ   (60000000)
#define CVBS_CAP_TIM_PSC    (3)
#define CVBS_CAP_US         (1000000 / (float)(CVBS_CAP_TIM_FREQ / CVBS_CAP_TIM_PSC))

typedef enum {
    ITU_STANDARD_UNKNOW = 0,
    ITU_STANDARD_M,
    ITU_STANDARD_N,
    ITU_STANDARD_B_B1_G_H_I_D_D1_K_K1_L,
} ITU_Standards_TypeDef;

typedef enum {
    CVBS_ENCODE_UNKNOW = 0,
    CVBS_ENCODE_NTSC,
    CVBS_ENCODE_PAL,
    CVBS_ENCODE_SECAM,
} CVBS_Encode_TypeDef;

typedef enum {
    CVBS_FIELD_ODD,
    CVBS_FIELD_EVEN
} CVBS_Field_TypeDef;

typedef enum {
    CVBS_SYNC_UNKNOW = 0,
    CVBS_SYNC_DETECT_STANDARD,
    CVBS_SYNC_SUCCEED_NTSC,
    CVBS_SYNC_SUCCEED_PAL,
    CVBS_SYNC_FAILED
} CVBS_Sync_TypeDef;

typedef enum {
    CVBS_CAPTURE_STATE_UNKNOW = 0,
    CVBS_CAPTURE_STATE_FIRST_EPULSE,
    CVBS_CAPTURE_STATE_FSPULSE,
    CVBS_CAPTURE_STATE_SECOND_EPULSE,
    CVBS_CAPTURE_STATE_SPULSE
} CVBS_Capture_State_TypeDef;

typedef struct _tickErrInfo_s {
    uint16_t ref; // reference time
    uint16_t err; // bias time
} _tickErrInfo_t;

#define _INSERT_TIMENERR_HELPER(time, error)                                                   \
    {                                                                                          \
        .ref = (uint16_t)((time) / CVBS_CAP_US), .err = (uint8_t)(1 + ((error) / CVBS_CAP_US)) \
    }

#define _COMP_TIMENERR_HELPER(val, stru) \
    (((val) > (stru.ref - stru.err)) && ((val) < (stru.ref + stru.err)))

/**
 * @brief CVBS info struct
 * 
 */
typedef struct cvbsStaticInfo_s {
    CVBS_Encode_TypeDef cvbsEncode;
    ITU_Standards_TypeDef ituStandard;

    uint16_t lines;
    uint8_t fields;
    uint8_t equalizingPulses;
    uint8_t synchronizingPulses;
    uint8_t fieldBlankingLines;

    _tickErrInfo_t sTickLinePeriod;
    _tickErrInfo_t sTickHsyncPulses;
    _tickErrInfo_t sTickEqualizingPulses;
    _tickErrInfo_t sTickSynchronizingPulses;
} cvbsStaticInfo_t;

/**
 * @brief CVBS format N/PAL defined
 * 
 */
static const cvbsStaticInfo_t PAL_N = {
    // format PAL
    .cvbsEncode = CVBS_ENCODE_PAL,
    // standard N
    .ituStandard = ITU_STANDARD_N,

    // total line number 625
    .lines = 625,
    // odd & even field
    .fields = 2,
    // field-synchronizing equalizing pulse(low) number, check ITU-R BT.470-6 FIGURE 2-1a
    .equalizingPulses = 10,
    // field-synchronizing pulse(low) number, check ITU-R BT.470-6 FIGURE 2-1a
    .synchronizingPulses = 5,
    // field-blanking line number
    .fieldBlankingLines = 24,

    // Nominal line period: Nominally 64 µs
    .sTickLinePeriod = _INSERT_TIMENERR_HELPER(64, 3.0),
    // Duration of synchronizing pulse: 4.7 ± 0.2 µs
    .sTickHsyncPulses = _INSERT_TIMENERR_HELPER(4.7, 0.8),
    // Duration of equalizing pulse: 2.35 ± 0.1 µs
    .sTickEqualizingPulses = _INSERT_TIMENERR_HELPER(2.35, 0.8),
    // Duration of field-synchronizing pulse: 27.3 ± 0.6 µs
    .sTickSynchronizingPulses = _INSERT_TIMENERR_HELPER(27.3, 3.0),
};

/**
 * @brief CVBS format M/NTSC defined
 * 
 */
static const cvbsStaticInfo_t NTSC_M = {
    // format NTSC
    .cvbsEncode = CVBS_ENCODE_NTSC,
    // standard M
    .ituStandard = ITU_STANDARD_M,

    // total line number 525
    .lines = 525,
    // odd & even field
    .fields = 2,
    // field-synchronizing equalizing pulse(low) number, check ITU-R BT.470-6 FIGURE 2-2a(or b)
    .equalizingPulses = 12,
    // field-synchronizing pulse(low) number, check ITU-R BT.470-6 FIGURE 2-2a(or b)
    .synchronizingPulses = 6,
    // field-blanking line number
    .fieldBlankingLines = 21,

    // Nominal line period: Nominally 63.5555 µs
    .sTickLinePeriod = _INSERT_TIMENERR_HELPER(63.5, 3.0),
    // Duration of synchronizing pulse: 4.7 ± 0.1 µs
    .sTickHsyncPulses = _INSERT_TIMENERR_HELPER(4.7, 0.8),
    // Duration of equalizing pulse: 2.3 ± 0.1 µs
    .sTickEqualizingPulses = _INSERT_TIMENERR_HELPER(2.3, 0.8),
    // Duration of field-synchronizing pulse: 27.1 ± 0.1 µs
    .sTickSynchronizingPulses = _INSERT_TIMENERR_HELPER(27.1, 3.0),
};

#define CVBS_STARTUP_STANDARD ITU_STANDARD_M

void CVBS_Init(void);

void CVBS_SetVideoStandard(ITU_Standards_TypeDef standard);

uint16_t CVBS_GetFrameLines(void);
uint16_t CVBS_GetActiveLines(void);
CVBS_Sync_TypeDef CVBS_GetVideoSyncStatus(void);

void CVBS_Hanler(uint16_t rising_time, uint16_t falling_time);

#endif /* __CVBS_H */
