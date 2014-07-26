#ifndef DEBUG_MODULE_H_
#define DEBUG_MODULE_H_

#include "lpc17xx_uart.h"

// Terminal Commands
void UART_Clear(LPC_UART_TypeDef *UARTx);
void UART_SetCursor(LPC_UART_TypeDef *UARTx, const uint8_t row, const uint8_t col);

//Number Formatting
void UART_PutDec(LPC_UART_TypeDef *UARTx, uint8_t decnum);
void UART_PutDec16(LPC_UART_TypeDef *UARTx, uint16_t decnum);
void UART_PutDec32(LPC_UART_TypeDef *UARTx, uint32_t decnum);
void UART_PutHex (LPC_UART_TypeDef *UARTx, uint8_t hexnum);
void UART_PutHex16 (LPC_UART_TypeDef *UARTx, uint16_t hexnum);
void UART_PutHex32 (LPC_UART_TypeDef *UARTx, uint32_t hexnum);

#endif /* DEBUG_MODULE_H_ */
