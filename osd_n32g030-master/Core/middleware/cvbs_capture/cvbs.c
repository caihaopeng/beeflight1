/**
 * @file cvbs.c
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2021-12-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "cvbs.h"

volatile struct {
    cvbsStaticInfo_t *pCvbsStandard;

    CVBS_Capture_State_TypeDef captureState;

    uint16_t pulse;
    uint16_t lastPulse;

    CVBS_Field_TypeDef field;
    uint16_t line;
    uint16_t activeLine;

    uint16_t failurePulse;

} cvbsRuntime;

volatile static uint16_t log_frame_lines = 0;

void CVBS_Init(void)
{
    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_UNKNOW;
    CVBS_SetVideoStandard(CVBS_STARTUP_STANDARD);
}

void CVBS_SetVideoStandard(ITU_Standards_TypeDef standard)
{
    switch (standard) {
    case ITU_STANDARD_M:
        cvbsRuntime.pCvbsStandard = (cvbsStaticInfo_t *)&NTSC_M;
        break;

    case ITU_STANDARD_N:
        cvbsRuntime.pCvbsStandard = (cvbsStaticInfo_t *)&PAL_N;
        break;

    default:
        break;
    }
}

uint16_t CVBS_GetFrameLines(void)
{
    return log_frame_lines;
}

uint16_t CVBS_GetActiveLines(void)
{
    return cvbsRuntime.activeLine;
}

CVBS_Sync_TypeDef CVBS_GetVideoSyncStatus(void)
{
    CVBS_Sync_TypeDef stat = CVBS_SYNC_UNKNOW;

    if (log_frame_lines != 0) {
        if (log_frame_lines == NTSC_M.lines) {
            stat = CVBS_SYNC_SUCCEED_NTSC;
        } else if (log_frame_lines == PAL_N.lines) {
            stat = CVBS_SYNC_SUCCEED_PAL;
        } else {
            stat = CVBS_SYNC_FAILED;
        }

        log_frame_lines = 0;
    }

    return stat;
}

/**
 * @brief CVBS handler, should run on capture falling edge interrupt.
 * 
 * @param rising_time CVBS capture rising edge tick
 * @param falling_time CVBS capture falling edge tick
 */
void CVBS_Hanler(uint16_t rising_time, uint16_t falling_time)
{
    static uint16_t last_rising_time;
    static uint16_t blank_lines;
    static bool standard_n_odd_ready_flag;

    cvbsRuntime.pulse = falling_time - rising_time;

    switch (cvbsRuntime.pCvbsStandard->ituStandard) {
    static uint8_t EPulseDetectTick = 0;
    static uint8_t FSPulseDetectTick = 0;
    static uint8_t SPulseDetectTick = 0;

    case ITU_STANDARD_M: // NTSC 525 lines
        if (cvbsRuntime.captureState != CVBS_CAPTURE_STATE_UNKNOW) {
            if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, NTSC_M.sTickHsyncPulses)) {
                if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_SECOND_EPULSE && SPulseDetectTick++ >= 1) {
                    SPulseDetectTick = 0;
                    cvbsRuntime.line += 5;
                    if (cvbsRuntime.field == CVBS_FIELD_ODD) {
                        blank_lines = NTSC_M.fieldBlankingLines;
                    } else {
                        blank_lines = NTSC_M.fieldBlankingLines;
                    }
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_SPULSE;

                } else if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_SPULSE){
                    SPulseDetectTick = 0;
                    cvbsRuntime.line++;
                    if (!blank_lines) {
                        cvbsRuntime.activeLine++;
                    } else {
                        blank_lines--;
                    }

                } else {
                    SPulseDetectTick++;
                }

            } else if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, NTSC_M.sTickEqualizingPulses)) {
                if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_SPULSE && EPulseDetectTick >= 1) {
                    EPulseDetectTick = 0;
                    if (cvbsRuntime.field == CVBS_FIELD_EVEN) {
                        cvbsRuntime.field = CVBS_FIELD_ODD;
                        log_frame_lines = cvbsRuntime.line;
                        cvbsRuntime.line = 0;
                        cvbsRuntime.failurePulse = 0;
                    } else {
                        cvbsRuntime.field = CVBS_FIELD_EVEN;
                    }
                    cvbsRuntime.activeLine = 0;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_FIRST_EPULSE;

                } else if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_FSPULSE && EPulseDetectTick >= 1) {
                    EPulseDetectTick = 0;
                    cvbsRuntime.line += 3;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_SECOND_EPULSE;

                } else {
                    EPulseDetectTick++;
                }

            } else if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, NTSC_M.sTickSynchronizingPulses)) {
                if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_FIRST_EPULSE && FSPulseDetectTick >= 1) {
                    FSPulseDetectTick = 0;
                    cvbsRuntime.line += 3;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_FSPULSE;

                } else {
                    FSPulseDetectTick++;
                }

            } else {
                cvbsRuntime.failurePulse++;
                if (cvbsRuntime.failurePulse > 10) {
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_UNKNOW;
                }
            }

        } else {
            if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, NTSC_M.sTickEqualizingPulses)) {
                if (_COMP_TIMENERR_HELPER(cvbsRuntime.lastPulse, NTSC_M.sTickHsyncPulses)) {
                    if (_COMP_TIMENERR_HELPER(falling_time - last_rising_time, NTSC_M.sTickLinePeriod)) {
                        cvbsRuntime.field = CVBS_FIELD_ODD;
                        cvbsRuntime.line = 0;
                        cvbsRuntime.failurePulse = 0;
                    }
                    cvbsRuntime.activeLine = 0;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_FIRST_EPULSE;
                }
            }
        }
        break;

    case ITU_STANDARD_N: // PAL 625 lines
        if (cvbsRuntime.captureState != CVBS_CAPTURE_STATE_UNKNOW) {
            if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, PAL_N.sTickHsyncPulses)) {
                if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_SECOND_EPULSE && SPulseDetectTick++ >= 1) {
                    SPulseDetectTick = 0;
                    cvbsRuntime.line += 4;
                    if (cvbsRuntime.field == CVBS_FIELD_ODD) {
                        blank_lines = NTSC_M.fieldBlankingLines;
                    } else {
                        blank_lines = NTSC_M.fieldBlankingLines;
                    }
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_SPULSE;

                } else if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_SPULSE){
                    SPulseDetectTick = 0;
                    cvbsRuntime.line++;
                    if (!blank_lines) {
                        cvbsRuntime.activeLine++;
                    } else {
                        blank_lines--;
                    }

                } else {
                    SPulseDetectTick++;
                }

            } else if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, PAL_N.sTickEqualizingPulses)) {
                if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_SPULSE && EPulseDetectTick >= 1) {
                    EPulseDetectTick = 0;
                    if (cvbsRuntime.field == CVBS_FIELD_EVEN) {
                        standard_n_odd_ready_flag = false;
                    } else {
                        standard_n_odd_ready_flag = true;
                    }
                    cvbsRuntime.activeLine = 0;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_FIRST_EPULSE;

                } else if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_FSPULSE && EPulseDetectTick >= 1) {
                    EPulseDetectTick = 0;
                    cvbsRuntime.line += 3;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_SECOND_EPULSE;

                } else {
                    EPulseDetectTick++;
                }

            } else if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, PAL_N.sTickSynchronizingPulses)) {
                if (cvbsRuntime.captureState == CVBS_CAPTURE_STATE_FIRST_EPULSE && FSPulseDetectTick >= 1) {
                    FSPulseDetectTick = 0;
                    cvbsRuntime.line += 2;
                    if (standard_n_odd_ready_flag) {
                        cvbsRuntime.field = CVBS_FIELD_EVEN;
                        log_frame_lines = cvbsRuntime.line;
                        cvbsRuntime.line = 0;
                        cvbsRuntime.failurePulse = 0;
                    } else {
                        cvbsRuntime.field = CVBS_FIELD_ODD;
                        cvbsRuntime.line += 1;
                    }
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_FSPULSE;

                } else {
                    FSPulseDetectTick++;
                }

            } else {
                cvbsRuntime.failurePulse++;
                if (cvbsRuntime.failurePulse > 10) {
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_UNKNOW;
                }
            }

        } else {
            if (_COMP_TIMENERR_HELPER(cvbsRuntime.pulse, PAL_N.sTickEqualizingPulses)) {
                if (_COMP_TIMENERR_HELPER(cvbsRuntime.lastPulse, PAL_N.sTickHsyncPulses)) {
                    if (_COMP_TIMENERR_HELPER(falling_time - last_rising_time, PAL_N.sTickLinePeriod)) {
                        standard_n_odd_ready_flag = false;
                    }
                    cvbsRuntime.activeLine = 0;
                    cvbsRuntime.captureState = CVBS_CAPTURE_STATE_FIRST_EPULSE;
                }
            }
        }
        break;

    default:
        break;
    }

    cvbsRuntime.lastPulse = cvbsRuntime.pulse;
    last_rising_time = rising_time;
}