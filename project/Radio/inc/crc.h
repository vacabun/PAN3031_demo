#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

// CRC types
#define CRC_TYPE_CCITT 0
#define CRC_TYPE_IBM 1
// Polynomial = X^16 + X^12 + X^5 + 1
#define POLYNOMIAL_CCITT 0x1021
// Polynomial = X^16 + X^15 + X^2 + 1
#define POLYNOMIAL_IBM 0x8005
// Seeds
#define CRC_IBM_SEED 0xFFFF
#define CRC_CCITT_SEED 0x1D0F


uint16_t RadioComputeCRC( uint8_t *buffer, uint8_t length, uint8_t crcType );
uint16_t ComputeCrc( uint16_t crc, uint8_t dataByte, uint16_t polynomial );



#endif

