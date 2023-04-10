/**
 * @file encryption.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __ENCRYPTION_H
#define __ENCRYPTION_H

#include <stdbool.h>
#include <stdint.h>

uint32_t Encryption_GenerateKeyFromUID(uint8_t *uid, uint16_t length);

#endif /* __ENCRYPTION_H */
