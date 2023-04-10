/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file main.c
 * @author Nations
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "user_flash.h"
#include "cvbs.h"
#include "max7456.h"

#ifdef _USE_DYNAMIC_VTX
    #include "vtx.h"
#endif

#ifndef _DEBUG_MODE
    #include "user_encryption.h"
#endif

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define SYSTICK_100US                       ((uint32_t)10000)

#define TASKTICK_OSD                        10

#ifdef _USE_DYNAMIC_VTX
    #define TASKTICK_DYNAMIC_VTX            5000

    #define TEMP_V25                        1.3f    // voltage value at 25 degree Celsius
    #define TEMP_AVG_SLOPE                  0.0042f // mv per degree Celsius
#endif /* _USE_DYNAMIC_VTX */

#define BETAFLIGHT_OSD_INITIALIZE_CODE      0x1B // Default value of @OSDM register. (BF 4.2)

#define PACKET_SEPARATE_TIME                8

#define CVBS_SIGNAL_DEFAULT_COMP_LEVEL      COMP_INVREF_VREFSEL_1
#ifdef _AUTO_COMP_ADJUST
    #define CVBS_SIGNAL_LOSS_THRESHOLD      10
    #define CVBS_SIGNAL_MIN_COMP_LEVEL      COMP_INVREF_VREFSEL_1
    #define CVBS_SIGNAL_MAX_COMP_LEVEL      COMP_INVREF_VREFSEL_4
#endif /* _AUTO_COMP_ADJUST */

/* Private variables ---------------------------------------------------------*/
volatile uint32_t systick = 0;

volatile uint8_t packet_recv_buffer[MAX_PACKET_SIZE];

/* Private function prototypes -----------------------------------------------*/
void GPIO_Configuration(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void SPI1_DMA_Configuration(void);
void SPI2_DMA_Configuration(void);
void COMP_Configuration(void);
void TIM8_Configuration(void);

#ifdef _USE_DYNAMIC_VTX
void ADC_Temp_Configuration(void);
void TIM3_Configuration(void);
void DynamicVtx_Handler(void);
#endif /* _USE_DYNAMIC_VTX */

void Packet_Handler(void);
void OSD_Hanler(void);

/**
 * @brief  Main program.
 */
int main(void)
{
#ifndef _DEBUG_MODE
    if (FLASH_GetReadOutProtectionSTS() != SET) {
        FLASH_Unlock();
        FLASH_ReadOutProtectionL1(ENABLE);
        FLASH_Lock();
        NVIC_SystemReset();
    }
    User_EncryptionVerify();
#endif
    /*SystemInit() function has been called by startup file startup_n32g030.s*/
    RCC_Configuration();
    NVIC_Configuration();

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / SYSTICK_100US);

    CVBS_Init();
    MAX7456_Init();

    GPIO_Configuration();

    SPI1_DMA_Configuration();
    SPI2_DMA_Configuration();
    COMP_Configuration();
    TIM8_Configuration();
#ifdef _USE_DYNAMIC_VTX
    ADC_Temp_Configuration();
    TIM3_Configuration();
#endif /* _USE_DYNAMIC_VTX */

    DMA_EnableChannel(SPI2_RX_DMA_CHANNEL, ENABLE);

    while (1) {
        OSD_Hanler();
        Packet_Handler();
#ifdef _USE_DYNAMIC_VTX
        DynamicVtx_Handler();
#endif
    }
}

/**
 * @brief SPI slave packet handler
 */
void Packet_Handler(void)
{
    static bool packet_transmit_start_flag = false;
    static uint32_t packet_separate_tick = 0;
    static uint16_t packet_recv_count_latest = 0;
    uint16_t packet_recv_count = MAX_PACKET_SIZE - DMA_GetCurrDataCounter(SPI2_RX_DMA_CHANNEL);

    if (packet_recv_count_latest != packet_recv_count) {
        packet_transmit_start_flag = true;
        packet_recv_count_latest = packet_recv_count;
        packet_separate_tick = systick;
    } else {
        if ((packet_transmit_start_flag == true) && (packet_recv_count != 0) && ((systick - packet_separate_tick) >= PACKET_SEPARATE_TIME)) {
#if defined(_DEBUG_WITH_IO) && defined(_IO_DEBUG_PACKET_HANDLER)
            GPIO_WriteBit(DEBUG_IO_GPIO_Port, DEBUG_IO_PIN, Bit_SET);
#endif
            DMA_EnableChannel(SPI2_RX_DMA_CHANNEL, DISABLE);

            for (uint16_t loop = 0; loop < packet_recv_count; loop++) {
                MAX7456_SpiDataHandler(packet_recv_buffer[loop]);
            }

            MAX7456_ResetDataHandler();

            DMA_SetCurrDataCounter(SPI2_RX_DMA_CHANNEL, MAX_PACKET_SIZE);
            DMA_EnableChannel(SPI2_RX_DMA_CHANNEL, ENABLE);

            packet_transmit_start_flag = false;
#if defined(_DEBUG_WITH_IO) && defined(_IO_DEBUG_PACKET_HANDLER)
            GPIO_WriteBit(DEBUG_IO_GPIO_Port, DEBUG_IO_PIN, Bit_RESET);
#endif
        }
    }
}

/**
 * @brief OSD function handler
 */
void OSD_Hanler(void)
{
    static uint32_t taskTick;
    static ITU_Standards_TypeDef tmp_video_standard;

#ifdef _AUTO_CVBS_FORMAT
    static ITU_Standards_TypeDef last_video_standard;
#endif

#ifdef _AUTO_COMP_ADJUST
    static uint16_t cvbs_signal_loss_count = 0;
    static COMP_INVREF_VREFXSEL_ENUM vref_best_value = CVBS_SIGNAL_DEFAULT_COMP_LEVEL;
#endif /* _AUTO_COMP_ADJUST */

    if ((systick - taskTick) >= TASKTICK_OSD) {
        if (MAX7456_GetRegisterChange()) {

            if (MAX7456_GetEnableStatus()) {
                COMP_Enable(COMP_CTRL_EN_ENABLE);
            } else {
                COMP_Enable(COMP_CTRL_EN_DISABLE);
            }

            if (MAX7456_GetResetStatus()) {
                MAX7456_Reset();
            }

            // if (MAX7456_GetClearDisplay()) {
            //     MAX7456_ClearDisplayMemory();
            // }
#ifndef _AUTO_CVBS_FORMAT
            if (tmp_video_standard != (ITU_Standards_TypeDef)MAX7456_GetVideoStandard()) {
                CVBS_SetVideoStandard(tmp_video_standard);
                tmp_video_standard = (ITU_Standards_TypeDef)MAX7456_GetVideoStandard();
            }
#endif /* _AUTO_CVBS_FORMAT */

            if (MAX7456_GetWriteNVMFlag()) {
                uint8_t *pData = MAX7456_GetNVMDataPoint();
                uint32_t address = MAX7456_GetNVMAddress();

                if (address == 0x00) { // Start address
                    UserFlash_StartContinuousWrite((MAX7456_CHARACTER_MEMORY_ADDR - FLASH_START_ADDR) / FLASH_PAGE_SIZE);
                }

                if (address < 0xA0) { // character memory < 0xA0, logo memory >= 0xA0
                    UserFlash_WriteContinuousRomData(pData, MAX7456_SINGLE_CHAR_SIZE);
                } else {
                    if (MAX7456_GetWriteLogoFlag()) {
                        UserFlash_WriteContinuousRomData(pData, MAX7456_SINGLE_CHAR_SIZE);
                    }
                }

                if (address == 0xFF) { // End
                    UserFlash_StopContinuousWrite();
                }

                MAX7456_ClearNVMWriteFlag();
            }
        }

#ifdef _AUTO_CVBS_FORMAT
        if (last_video_standard != tmp_video_standard) {
            CVBS_SetVideoStandard(tmp_video_standard);
            last_video_standard = tmp_video_standard;
        }
#endif

        /* Set Status */
        switch (CVBS_GetVideoSyncStatus()) {

        case CVBS_SYNC_FAILED:
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_LOS, true);
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_NTSC, false);
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_PAL, false);

#ifdef _AUTO_COMP_ADJUST
            cvbs_signal_loss_count++;
            if (cvbs_signal_loss_count > CVBS_SIGNAL_LOSS_THRESHOLD) {
                vref_best_value += COMP_INVREF_VREFSEL_1;
                if (vref_best_value > CVBS_SIGNAL_MAX_COMP_LEVEL) {
                    vref_best_value = CVBS_SIGNAL_MIN_COMP_LEVEL;
                }
                COMP_ConfigVREFx(VREF, vref_best_value, ENABLE);
                cvbs_signal_loss_count = 0;
            }
#endif /* _AUTO_COMP_ADJUST */
            break;

        case CVBS_SYNC_SUCCEED_NTSC:
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_LOS, false);
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_NTSC, true);
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_PAL, false);
#ifdef _AUTO_CVBS_FORMAT
            tmp_video_standard = ITU_STANDARD_M;
#endif
#ifdef _AUTO_COMP_ADJUST
            cvbs_signal_loss_count = 0;
#endif
            break;

        case CVBS_SYNC_SUCCEED_PAL:
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_LOS, false);
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_NTSC, false);
            MAX7456_SetStatusExternal(MAX7456_STATTYPE_PAL, true);
#ifdef _AUTO_CVBS_FORMAT
            tmp_video_standard = ITU_STANDARD_N;
#endif
#ifdef _AUTO_COMP_ADJUST
            cvbs_signal_loss_count = 0;
#endif
            break;

        default:
            break;
        }

        taskTick = systick;
    }
}

#ifdef _USE_DYNAMIC_VTX
void DynamicVtx_Handler(void)
{
    static uint32_t taskTick;
    vtxPower_e power_level;

    if ((systick - taskTick) >= TASKTICK_DYNAMIC_VTX) {
#ifdef _VTX_USE_SPI_CTRL
        power_level = (vtxPower_e)MAX7456_GetDynamicVTxPower();
#elif defined(_VTX_USE_GPIO_CTRL)
        power_level = (vtxPower_e)(((uint8_t)GPIO_ReadInputDataBit(VTX_SLAVE1_GPIO_Port, VTX_SLAVE1_PIN) << 1) |
                                   (uint8_t)GPIO_ReadInputDataBit(VTX_SLAVE0_GPIO_Port, VTX_SLAVE0_PIN));
#endif
        if (power_level == VTX_POWER_MAX) {
            ADC_EnableSoftwareStartConv(ADC, ENABLE);
            while (ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC) != SET);
            ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
            VTx_PID_SetTemperature((TEMP_V25 - ADC_GetDat(ADC) * (3.3 / 4095)) / TEMP_AVG_SLOPE + 25.0f);
        }

        TIM_SetCmp1(TIM3, VTx_PID_Converter(power_level));

        taskTick = systick;
    }
}
#endif /* _USE_DYNAMIC_VTX */

/**
 * ---------------------------------------------------------------------------------------------------
 * |                                       System Config                                             |
 * ---------------------------------------------------------------------------------------------------
 */

/**
 * @brief  Configures the different system clocks.
 */
void RCC_Configuration(void)
{
    RCC_ConfigPclk1(RCC_HCLK_DIV2); // Set APB1 clock lower than 48 MHz
    RCC_ConfigPclk2(RCC_HCLK_DIV2); // Set APB2 clock lower than 48 MHz
    /* Enable peripheral clocks
     * ------------------------------------------------*/

    /* Enable DMA clock */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB |
                            RCC_APB2_PERIPH_SPI1 | RCC_APB2_PERIPH_SPI2 |
                            RCC_APB2_PERIPH_TIM8 |
                            RCC_APB2_PERIPH_AFIO,
                            ENABLE);

    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_COMP |
                            RCC_APB1_PERIPH_COMPFILT, ENABLE);

#ifdef _USE_DYNAMIC_VTX
    /* Enable ADC clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV8);
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV2);
    /* TIM3 clock enable */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM3, ENABLE);
#endif /* _USE_DYNAMIC_VTX */
}

/**
 * @brief  Configure SPI1 & DMA channel
 *
 */
void SPI1_DMA_Configuration(void)
{
    SPI_InitType SPI_InitStructure = {0};
    DMA_InitType DMA_InitStructure = {0};

    SPI_InitStruct(&SPI_InitStructure);
    SPI_InitStructure.DataDirection     = SPI_DIR_SINGLELINE_TX;
    SPI_InitStructure.SpiMode           = SPI_MODE_MASTER;
    SPI_InitStructure.DataLen           = SPI_DATA_SIZE_8BITS;
    SPI_InitStructure.CLKPOL            = SPI_CLKPOL_LOW;
    SPI_InitStructure.CLKPHA            = SPI_CLKPHA_FIRST_EDGE;
    SPI_InitStructure.NSS               = SPI_NSS_SOFT;
    SPI_InitStructure.BaudRatePres      = SPI_BR_PRESCALER_4;
    SPI_InitStructure.FirstBit          = SPI_FB_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);

    /* SPI_MASTER_TX_DMA_CHANNEL configuration */
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr        = (uint32_t)&SPI1->DAT; // SPI1_MASTER_DR_BASE;
    DMA_InitStructure.MemAddr           = (uint32_t)MAX7456_GetOutputPixelAddress();
    DMA_InitStructure.Direction         = DMA_DIR_PERIPH_DST;
    DMA_InitStructure.PeriphInc         = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc     = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize    = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize       = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode      = DMA_MODE_NORMAL;
    DMA_InitStructure.Priority          = DMA_PRIORITY_LOW;
    DMA_InitStructure.Mem2Mem           = DMA_M2M_DISABLE;
    DMA_Init(SPI1_TX_DMA_CHANNEL, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_SPI1_TX, DMA, SPI1_TX_DMA_CHANNEL, ENABLE);

    /* Enable SPI_MASTER DMA Tx request */
    SPI_I2S_EnableDma(SPI1, SPI_I2S_DMA_TX, ENABLE);

    /* Enable SPI_MASTER */
    SPI_Enable(SPI1, ENABLE);
}

/**
 * @brief  Configure SPI2 & DMA channel
 *
 */
void SPI2_DMA_Configuration(void)
{
    SPI_InitType SPI_InitStructure = {0};
    DMA_InitType DMA_InitStructure = {0};

    SPI_InitStruct(&SPI_InitStructure);
    SPI_InitStructure.DataDirection     = SPI_DIR_DOUBLELINE_FULLDUPLEX;
    SPI_InitStructure.SpiMode           = SPI_MODE_SLAVE;
    SPI_InitStructure.DataLen           = SPI_DATA_SIZE_8BITS;
    SPI_InitStructure.CLKPOL            = SPI_CLKPOL_HIGH;
    SPI_InitStructure.CLKPHA            = SPI_CLKPHA_SECOND_EDGE;
    SPI_InitStructure.NSS               = SPI_NSS_HARD;
    SPI_InitStructure.BaudRatePres      = SPI_BR_PRESCALER_2;
    SPI_InitStructure.FirstBit          = SPI_FB_MSB;
    SPI_Init(SPI2, &SPI_InitStructure);

    /* SPI_MASTER_RX_DMA_CHANNEL configuration */
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr        = (uint32_t)&SPI2->DAT; // SPI2_SLAVE_DR_BASE;
    DMA_InitStructure.MemAddr           = (uint32_t)&packet_recv_buffer;
    DMA_InitStructure.Direction         = DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize           = MAX_PACKET_SIZE;
    DMA_InitStructure.PeriphInc         = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc     = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize    = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize       = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode      = DMA_MODE_NORMAL;
    DMA_InitStructure.Priority          = DMA_PRIORITY_MEDIUM;
    DMA_InitStructure.Mem2Mem           = DMA_M2M_DISABLE;
    DMA_Init(SPI2_RX_DMA_CHANNEL, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_SPI2_RX, DMA, SPI2_RX_DMA_CHANNEL, ENABLE);

    /* Enable SPI_MASTER DMA Tx request */
    SPI_I2S_EnableDma(SPI2, SPI_I2S_DMA_RX, ENABLE);

    /* Reload SPI_DAT at the first time */
    SPI_I2S_TransmitData(SPI2, BETAFLIGHT_OSD_INITIALIZE_CODE);
    /* Enable SPI_MASTER */
    SPI_Enable(SPI2, ENABLE);
}

/**
 * @brief  Configures the comp module.
 */
void COMP_Configuration(void)
{
    COMP_InitType COMP_Initial;

    /*Initial comp*/
    COMP_StructInit(&COMP_Initial);
    COMP_Initial.InpSel     = COMPx_CTRL_INPSEL_VREF;
    COMP_Initial.InmSel     = COMP_CTRL_INMSEL_PA4;
    COMP_Initial.OutTrg     = COMP_CTRL_OUTTRG_TIM8_IC1;
    COMP_Initial.SampWindow = COMPX_FILC_SAMPW_30;
    COMP_Initial.Thresh     = COMPX_FILC_THRESH_16;
    COMP_Initial.FilterEn   = COMPX_FILC_FILEN_ENABLE;
    COMP_Initial.ClkPsc     = 0;
    COMP_Init(&COMP_Initial);

    COMP_ConfigVREFx(VREF, CVBS_SIGNAL_DEFAULT_COMP_LEVEL, ENABLE);

    COMP_Enable(COMP_CTRL_EN_ENABLE);
}

/**
 * @brief  Configures tim8 clocks.
 */
void TIM8_Configuration(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    TIM_ICInitType TIM_ICInitStructure;

    TIM_TimeBaseStructure.Period            = 0xFFFF;
    TIM_TimeBaseStructure.Prescaler         = CVBS_CAP_TIM_PSC - 1;
    TIM_TimeBaseStructure.ClkDiv            = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CntMode           = TIM_CNT_MODE_UP;
    TIM_TimeBaseStructure.CapCh1FromCompEn  = true;
    TIM_InitTimeBase(TIM8, &TIM_TimeBaseStructure);

    TIM_ICInitStructure.Channel             = TIM_CH_1;
    TIM_ICInitStructure.IcPolarity          = TIM_IC_POLARITY_RISING;
    TIM_ICInitStructure.IcSelection         = TIM_IC_SELECTION_DIRECTTI;
    TIM_ICInitStructure.IcPrescaler         = TIM_IC_PSC_DIV1;
    TIM_ICInitStructure.IcFilter            = 0x3;
    TIM_ICInit(TIM8, &TIM_ICInitStructure);

    TIM_ICInitStructure.Channel             = TIM_CH_2;
    TIM_ICInitStructure.IcPolarity          = TIM_IC_POLARITY_FALLING;
    TIM_ICInitStructure.IcSelection         = TIM_IC_SELECTION_INDIRECTTI;
    TIM_ICInitStructure.IcPrescaler         = TIM_IC_PSC_DIV1;
    TIM_ICInitStructure.IcFilter            = 0x3;
    TIM_ICInit(TIM8, &TIM_ICInitStructure);

    /* Enable the CC2 Interrupt Request */
    TIM_ConfigInt(TIM8, TIM_INT_CC2, ENABLE);

    /* TIM8 enable counter */
    TIM_Enable(TIM8, ENABLE);
}

#ifdef _USE_DYNAMIC_VTX
/**
 * @brief  Configures tim3 pwm.
 */
void TIM3_Configuration(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;

    TIM_TimeBaseStructure.Period            = VTX_RESOULTION_VAL;
    TIM_TimeBaseStructure.Prescaler         = (uint16_t)(SystemCoreClock / (VTX_PWM_FREQ * VTX_RESOULTION_VAL)) - 1;
    TIM_TimeBaseStructure.ClkDiv            = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CntMode           = TIM_CNT_MODE_UP;
    TIM_TimeBaseStructure.CapCh1FromCompEn  = true;
    TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.OcMode              = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState         = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.OutputNState        = TIM_OUTPUT_NSTATE_DISABLE;
    TIM_OCInitStructure.Pulse               = 0;
    TIM_OCInitStructure.OcPolarity          = TIM_OC_POLARITY_HIGH;
    TIM_OCInitStructure.OcNPolarity         = TIM_OC_POLARITY_HIGH;
    TIM_OCInitStructure.OcIdleState         = TIM_OC_IDLE_STATE_RESET;
    TIM_OCInitStructure.OcNIdleState        = TIM_OCN_IDLE_STATE_RESET;
    TIM_InitOc1(TIM3, &TIM_OCInitStructure);
    TIM_ConfigOc1Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);

    /* TIM3 auto reload */
    TIM_ConfigArPreload(TIM3, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM3, ENABLE);
    TIM_EnableCtrlPwmOutputs(TIM3, ENABLE);
}

/**
 * @brief Configure ADC with internal temperature sample
 */
void ADC_Temp_Configuration(void)
{
    ADC_InitType ADC_InitStructure;

    ADC_InitStructure.MultiChEn             = DISABLE;
    ADC_InitStructure.ContinueConvEn        = DISABLE;
    ADC_InitStructure.ExtTrigSelect         = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign              = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber             = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    /* ADC1 enable temperature  */
    ADC_EnableTempSensorVrefint(ENABLE);
    /* ADC regular channel1 configuration */
    ADC_ConfigRegularChannel(ADC, ADC_CH_TEMP_SENSOR, 1, ADC_SAMP_TIME_600CYCLES5);

    /* Enable ADC */
    ADC_Enable(ADC, ENABLE);
}
#endif /* _USE_DYNAMIC_VTX */

/**
 * @brief  Configure the nested vectored interrupt controller.
 */
void NVIC_Configuration(void)
{
    NVIC_InitType NVIC_InitStructure;

    /* Enable TIM8 CC2 IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel          = TIM8_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority  = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd       = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  Configures the different GPIO ports.
 */
void GPIO_Configuration(void)
{
    GPIO_InitType GPIO_InitStructure = {0};

    /*Configure GPIO pin */
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin              = CVBS_COMP_PIN;
    GPIO_InitStructure.GPIO_Mode        = GPIO_MODE_ANALOG;
    GPIO_InitStructure.GPIO_Current     = GPIO_DC_LOW;
    GPIO_InitPeripheral(CVBS_COMP_GPIO_Port, &GPIO_InitStructure);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin              = UNUSED_SPI1_SCK_Pin | CVBS_WHITE_OVERLAY_Pin | UNUSED_SPI_MISO_Pin;
    GPIO_InitStructure.GPIO_Speed       = GPIO_SPEED_HIGH;
    GPIO_InitStructure.GPIO_Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull        = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate   = GPIO_AF0_SPI1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin              = SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN | SPI2_NSS_PIN;
    GPIO_InitStructure.GPIO_Speed       = GPIO_SPEED_HIGH;
    GPIO_InitStructure.GPIO_Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull        = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate   = GPIO_AF0_SPI2;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

#ifdef _USE_DYNAMIC_VTX
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin              = VTX_CTRL_PIN;
    GPIO_InitStructure.GPIO_Speed       = GPIO_SPEED_HIGH;
    GPIO_InitStructure.GPIO_Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull        = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate   = GPIO_AF2_TIM3;
    GPIO_InitPeripheral(VTX_CTRL_GPIO_Port, &GPIO_InitStructure);

    #ifdef _VTX_USE_GPIO_CTRL
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin              = VTX_SLAVE0_PIN | VTX_SLAVE1_PIN;
    GPIO_InitStructure.GPIO_Speed       = GPIO_SPEED_HIGH;
    GPIO_InitStructure.GPIO_Mode        = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Pull        = GPIO_PULL_DOWN;
    GPIO_InitPeripheral(VTX_SLAVE0_GPIO_Port, &GPIO_InitStructure);
    #endif

#endif /* _USE_DYNAMIC_VTX */

#ifdef _DEBUG_WITH_IO
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin              = DEBUG_IO_PIN;
    GPIO_InitStructure.GPIO_Speed       = GPIO_SPEED_HIGH;
    GPIO_InitStructure.GPIO_Mode        = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.GPIO_Pull        = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate   = GPIO_NO_AF;
    GPIO_InitPeripheral(DEBUG_IO_GPIO_Port, &GPIO_InitStructure);
#endif /* _DEBUG_WITH_IO */
}

/**
 * @brief Assert failed function by user.
 * @param file The name of the call that failed.
 * @param line The source line number of the call that failed.
 */
#ifdef USE_FULL_ASSERT
void assert_failed(const uint8_t *expr, const uint8_t *file, uint32_t line)
{
    while (1) {
    }
}
#endif // USE_FULL_ASSERT
