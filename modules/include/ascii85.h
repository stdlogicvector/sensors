/*
 * ascii85.h
 *
 *  Created on: 08.09.2014
 *      Author: Konstantin
 */

#ifndef ASCII85_H_
#define ASCII85_H_

#include <stdint.h>

typedef struct
{
	uint8_t b[5];
} ascii85_t;


uint32_t ascii85_decode(ascii85_t data);
ascii85_t ascii85_encode(uint32_t data);

uint16_t swap_uint16(uint16_t val);
int16_t swap_int16(int16_t val);

#endif /* ASCII85_H_ */
