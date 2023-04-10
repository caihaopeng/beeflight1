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
 * @file main.h
 * @author Nations 
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "n32g030.h"
#include "defined.h"

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#ifdef _USE_DYNAMIC_VTX
    #define VTX_CTRL_PIN                GPIO_PIN_4
    #define VTX_CTRL_GPIO_Port          GPIOB

    #ifdef _VTX_USE_GPIO_CTRL
        #define VTX_SLAVE0_PIN              GPIO_PIN_0
        #define VTX_SLAVE0_GPIO_Port        GPIOB

        #define VTX_SLAVE1_PIN              GPIO_PIN_1
        #define VTX_SLAVE1_GPIO_Port        GPIOB
    #endif
#endif

#define UNUSED_SPI1_SCK_Pin             GPIO_PIN_5
#define UNUSED_SPI1_SCK_GPIO_Port       GPIOA

#define UNUSED_SPI_MISO_Pin             GPIO_PIN_6
#define UNUSED_SPI_MISO_GPIO_Port       GPIOA

#define CVBS_WHITE_OVERLAY_Pin          GPIO_PIN_7
#define CVBS_WHITE_OVERLAY_GPIO_Port    GPIOA

#define CVBS_COMP_PIN                   GPIO_PIN_4
#define CVBS_COMP_GPIO_Port             GPIOA

#ifdef _DEBUG_WITH_IO
    #define DEBUG_IO_PIN                GPIO_PIN_2
    #define DEBUG_IO_GPIO_Port          GPIOA
#endif

#define SPI2_SCK_PIN                    GPIO_PIN_9
#define SPI2_SCK_GPIO_Port              GPIOA

#define SPI2_MISO_PIN                   GPIO_PIN_10
#define SPI2_MISO_GPIO_Port             GPIOA

#define SPI2_MOSI_PIN                   GPIO_PIN_11
#define SPI2_MOSI_GPIO_Port             GPIOA

#define SPI2_NSS_PIN                    GPIO_PIN_8
#define SPI2_NSS_GPIO_Port              GPIOA

#define SPI1_TX_DMA_CHANNEL             DMA_CH1
#define SPI2_RX_DMA_CHANNEL             DMA_CH3


#define MAX_PACKET_SIZE                 3000

#define MAX7456_CHARACTER_MEMORY_ADDR   0x08006000
#define MAX7456_SINGLE_CHAR_SIZE        27

#define ENCRYPTION_KEY_MEMORY_ADDR      0x08005E00

/* Exported variable ---------------------------------------------------------*/
extern volatile uint32_t systick;

extern volatile uint8_t packet_recv_buffer[MAX_PACKET_SIZE];

/* Exported functions prototypes ---------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */
