/*
 * ascii85.c
 *
 *  Created on: 08.09.2014
 *      Author: Konstantin
 */

#include "ascii85.h"


uint32_t ascii85_decode(ascii85_t data)
{
	uint8_t i;
	uint32_t m = 1;
	uint32_t result = 0;

	for (i = 5; i--;)
	{
		result += (data.b[i] - 33) * m;
		m *= 85;
	}

	return result;
}

ascii85_t ascii85_encode(uint32_t data)
{
    uint8_t i;
    ascii85_t a;

    for (i = 5; i--;)
    {
        a.b[i] = (data % 85) + 33;
        data  /= 85;
    }

    return a;
}

inline uint16_t swap_uint16(uint16_t val)
{
	return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}

inline int16_t swap_int16(int16_t val)
{
	return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}
