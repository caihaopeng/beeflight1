/**
 * @file defined.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief This file is use for control project features
 * @version 0.1
 * @date 2021-12-10
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __DEFINED_H
#define __DEFINED_H

/**
 * @brief Feature define sector
 */
#define _USE_DYNAMIC_VTX

#define _AUTO_CVBS_FORMAT
#define _AUTO_COMP_ADJUST

/**
 * @brief Debug define sector
 */
#ifdef _DEBUG_MODE
    // #define _DEBUG_TIM8_COMP_CAPTURE
    // #define _DEBUG_WITH_IO
    #ifdef _DEBUG_WITH_IO
        // #define _IO_DEBUG_PACKET_HANDLER
        // #define _IO_DEBUG_CVBS_LINE
    #endif
#endif

#ifdef _USE_DYNAMIC_VTX
    #define _VTX_USE_GPIO_CTRL
    // #define _VTX_USE_SPI_CTRL
#endif

#endif /* __DEFINED_H */