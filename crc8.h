//------------------------------------------------------------------------------
// Status CRC8 prototypes
//------------------------------------------------------------------------------
#pragma once
#include <stdint.h>
uint8_t CSV_CalcCRC8_Add(const uint8_t* pucData, uint32_t lBytes, uint8_t ucCrc);
uint8_t CSV_CalcCRC8(const uint8_t* pucData, uint32_t lBytes);
