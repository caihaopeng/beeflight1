/**
 * @file max7456_character.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2021-12-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __MAX7456_CHARACTER_H
#define __MAX7456_CHARACTER_H

#include <stdint.h>

/*
 * !< DO NOT MODIFY VALUE IN THIS PAGE IF YOU DON'T KNOW WHAT IT WAS >!
 */

#define CHARACTER_PIXEL_PER_LINE        (12)
#define CHARACTER_PIXEL_PER_ROW         (18)
#define CHARACTERS_NUMBER               (256)

#define U8_BITS                         (8)

#define CHARACTER_PIXEL_BITS            (1.0f)
#define CHARACTER_BYTE_PER_LINES        ((CHARACTER_PIXEL_PER_LINE * CHARACTER_PIXEL_BITS) / U8_BITS)
#define _E_CHARACTER_BYTE_SIZE          (CHARACTER_BYTE_PER_LINES * CHARACTER_PIXEL_PER_ROW) // It's a float variable, using enum to cancel warning, see: \
                                                                                             // https://stackoverflow.com/questions/18435302/variable-length-array-folded-to-constant-array

// #define CHARACTER_ATTRIBUTE_BITS        (4)
// #define CHARACTER_ATTRIBUTE_PER_BYTE    (U8_BITS / CHARACTER_ATTRIBUTE_BITS)

enum { CHARACTER_BYTE_SIZE = (uint8_t)_E_CHARACTER_BYTE_SIZE };

#endif /* __MAX7456_CHARACTER_H */
