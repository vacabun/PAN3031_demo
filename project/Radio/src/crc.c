#include "crc.h"


uint16_t ComputeCrc( uint16_t crc, uint8_t dataByte, uint16_t polynomial )
{
  uint8_t i;
  
  for( i = 0; i < 8; i++ )
  {
   if( ( ( ( crc & 0x8000 ) >> 8 ) ^ ( dataByte & 0x80 ) ) != 0 )
   {
     crc <<= 1; // shift left once
     crc ^= polynomial; // XOR with polynomial
   }
   else
   { 
     crc <<= 1; // shift left once
   }
   dataByte <<= 1; // Next data bit
  }
  return crc;
}


uint16_t RadioComputeCRC( uint8_t *buffer, uint8_t length, uint8_t crcType )
{
  uint8_t i = 0;
  uint16_t crc = 0;
  uint16_t polynomial = 0;
  
  polynomial = ( crcType == CRC_TYPE_IBM ) ? POLYNOMIAL_IBM : POLYNOMIAL_CCITT;
  crc = ( crcType == CRC_TYPE_IBM ) ? CRC_IBM_SEED : CRC_CCITT_SEED;
  for( i = 0; i < length; i++ )
  {
   crc = ComputeCrc( crc, buffer[i], polynomial );
  }
  if( crcType == CRC_TYPE_IBM )
  {
   return crc;
  }
  else
  {
   return( ( uint16_t ) ( ~crc ));
   }
}


