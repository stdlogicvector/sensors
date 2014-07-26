/*
 * delay_module.h
 *
 *  Created on: 15.03.2014
 *      Author: Konstantin
 */

#ifndef DELAY_MODULE_H_
#define DELAY_MODULE_H_

#include "LPC17xx.h"
#include "lpc_types.h"

void delay_init(void);
void delay_ms(uint32_t delay);

#endif /* DELAY_MODULE_H_ */
