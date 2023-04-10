/**
 * @file max7456_display.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2021-12-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __MAX7456_DISPLAY_H
#define __MAX7456_DISPLAY_H

#include <stdint.h>

/*
 * !< DO NOT MODIFY VALUE IN THIS PAGE IF YOU DON'T KNOW WHAT IT WAS >!
 */

#define DISPLAY_AREA_NTSC_ROWS      (13)
#define DISPLAY_AREA_PAL_ROWS       (16)
#define DISPLAY_AREA_MAX_ROWS       DISPLAY_AREA_PAL_ROWS
#define DISPLAY_AREA_COLUMNS        (30)

#define DISPLAY_NTSC_LINES          (234)
#define DISPLAY_PAL_LINES           (288)
#define DISPLAY_MAX_LINES           DISPLAY_PAL_LINES
#define DISPLAY_LINES_PIXELS        (360)

#endif /* __MAX7456_DISPLAY_H */