/**
 * @file encryption.c
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "encryption.h"

static uint32_t _BKDRHash(uint8_t *str, uint16_t length);

uint32_t Encryption_GenerateKeyFromUID(uint8_t *uid, uint16_t length)
{
    return _BKDRHash(uid, length);
}

static uint32_t _BKDRHash(uint8_t *str, uint16_t length)
{
    uint16_t seed = 131;
    uint32_t hash = 0;

    for (uint16_t i = 0; i < length; str++, i++) {
        hash = (hash * seed) + (*str);
    }

    return hash;
}
