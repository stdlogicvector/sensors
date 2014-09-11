/*
 * serial_module.h
 *
 *  Created on: 15.03.2014
 *      Author: Konstantin
 */

#ifndef SERIAL_MODULE_H_
#define SERIAL_MODULE_H_

#include "LPC17xx.h"
#include <stdint.h>

/*
*  UART0 RXD(P0_3)	 TXD(P0_2)
*
*  UART1 RXD(P0_16)	 TXD(P0_15)
*		 RXD(P2_1)	 TXD(P2_0)
*
*  UART2 RXD(P0_10)	 TXD(P0_11)
*		 RXD(P2_8)	 TXD(P2_9)
*
*  UART3 RXD(P0_1)	 TXD(P0_0)
*	 	 RXD(P0_26)	 TXD(P0_25)
*		 RXD(P4_29)	 TXD(P4_28)
*/

#define SERIAL_UART_PORT	0

#define SERIAL_TXD_PORT		0
#define SERIAL_TXD_PIN		2

#define SERIAL_RXD_PORT		0
#define SERIAL_RXD_PIN		3

#define SERIAL_BUFSIZE		0x80

void serial_init(uint32_t baud_rate);

void serial_enableEcho(void);
void serial_disableEcho(void);

void serial_putc(uint8_t c);
void serial_puts(const void *str);
void serial_send(const void *buf, uint32_t size);

uint32_t serial_receive(uint8_t *buf, uint32_t buflen);

#if (SERIAL_UART_PORT==0)
void UART0_IRQHandler(void);
#elif (SERIAL_UART_PORT==1)
void UART1_IRQHandler(void);
#elif (SERIAL_UART_PORT==2)
void UART2_IRQHandler(void);
#elif (SERIAL_UART_PORT==3)
void UART3_IRQHandler(void);
#endif


#endif /* SERIAL_MODULE_H_ */
