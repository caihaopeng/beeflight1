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
 * @file n32g030_it.c
 * @author Nations
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include "n32g030_it.h"

#include "main.h"

#include "cvbs.h"
#include "max7456.h"

/** @addtogroup N32G030_StdPeriph_Template
 * @{
 */

/** @addtogroup CRC_Calculation
 * @{
 */

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                           */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles SVCall exception.
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles PendSV_Handler exception.
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 */
void SysTick_Handler(void)
{
    systick++;
}

/******************************************************************************/
/*                 N32G030 Peripherals Interrupt Handlers                     */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_n32g030.s).                                                 */
/******************************************************************************/
#ifdef _DEBUG_TIM8_COMP_CAPTURE
#define RESULT_DEBUG_COUNT 525
volatile static uint8_t debug_result_array[RESULT_DEBUG_COUNT] = {0};
volatile static uint16_t debug_result_count = 0;
#endif /* _DEBUG_TIM8_COMP_CAPTURE */

/**
 * @brief  This function handles TIM8 TRG interrupt request.
 */
void TIM8_CC_IRQHandler(void)
{
    uint16_t line;
#ifdef _DEBUG_TIM8_COMP_CAPTURE

    uint16_t result = 0;
    result = TIM_GetCap2(TIM8) - TIM_GetCap1(TIM8);
    debug_result_array[debug_result_count++] = result;
    if (debug_result_count > RESULT_DEBUG_COUNT) {
        debug_result_count = 0;
    }
#else
#if defined(_DEBUG_WITH_IO) && defined(_IO_DEBUG_CVBS_LINE)
    GPIO_WriteBit(DEBUG_IO_GPIO_Port, DEBUG_IO_PIN, Bit_SET);
#endif
    CVBS_Hanler(TIM_GetCap1(TIM8), TIM_GetCap2(TIM8));

    line = CVBS_GetActiveLines();
    if (line != 0) {
        MAX7456_ReloadOutput();
        DMA_EnableChannel(SPI1_TX_DMA_CHANNEL, DISABLE);
        DMA_SetCurrDataCounter(SPI1_TX_DMA_CHANNEL, MAX7456_GetOutputPixelNumber());
        DMA_EnableChannel(SPI1_TX_DMA_CHANNEL, ENABLE); // Start DMA transfer, then start calculate next line (can be done on main loop or interrupt)
        MAX7456_OutputSingleLine(line);
    }
#if defined(_DEBUG_WITH_IO) && defined(_IO_DEBUG_CVBS_LINE)
    GPIO_WriteBit(DEBUG_IO_GPIO_Port, DEBUG_IO_PIN, Bit_RESET);
#endif

#endif /* _DEBUG_TIM8_COMP_CAPTURE */
}
