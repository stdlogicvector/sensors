#include "debug_module.h"

inline void UART_Clear(LPC_UART_TypeDef *UARTx)
{
	uint8_t cmd[4] = {27, '[', 2, 'J'};
	UART_Send(UARTx, cmd, 4, BLOCKING);
}

inline void UART_SetCursor(LPC_UART_TypeDef *UARTx, const uint8_t row, const uint8_t col)
{
	uint8_t cmd[5] = {27, '[', row, col, 'H'};
	UART_Send(UARTx, cmd, 4, BLOCKING);
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void UART_PutDec(LPC_UART_TypeDef *UARTx, uint8_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	UART_SendByte(UARTx, '0'+c3);
	UART_SendByte(UARTx, '0'+c2);
	UART_SendByte(UARTx, '0'+c1);
}

/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void UART_PutDec16(LPC_UART_TypeDef *UARTx, uint16_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	uint8_t c4=(decnum/1000)%10;
	uint8_t c5=(decnum/10000)%10;
	UART_SendByte(UARTx, '0'+c5);
	UART_SendByte(UARTx, '0'+c4);
	UART_SendByte(UARTx, '0'+c3);
	UART_SendByte(UARTx, '0'+c2);
	UART_SendByte(UARTx, '0'+c1);
}

/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void UART_PutDec32(LPC_UART_TypeDef *UARTx, uint32_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	uint8_t c4=(decnum/1000)%10;
	uint8_t c5=(decnum/10000)%10;
	uint8_t c6=(decnum/100000)%10;
	uint8_t c7=(decnum/1000000)%10;
	uint8_t c8=(decnum/10000000)%10;
	uint8_t c9=(decnum/100000000)%10;
	uint8_t c10=(decnum/1000000000)%10;
	UART_SendByte(UARTx, '0'+c10);
	UART_SendByte(UARTx, '0'+c9);
	UART_SendByte(UARTx, '0'+c8);
	UART_SendByte(UARTx, '0'+c7);
	UART_SendByte(UARTx, '0'+c6);
	UART_SendByte(UARTx, '0'+c5);
	UART_SendByte(UARTx, '0'+c4);
	UART_SendByte(UARTx, '0'+c3);
	UART_SendByte(UARTx, '0'+c2);
	UART_SendByte(UARTx, '0'+c1);
}

/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (8-bit long)
 * @return		None
 **********************************************************************/
void UART_PutHex (LPC_UART_TypeDef *UARTx, uint8_t hexnum)
{
	uint8_t nibble, i;

	UART_Send(UARTx, (uint8_t *) "0x", 2, BLOCKING);
	i = 1;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		UART_SendByte(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (16-bit long)
 * @return		None
 **********************************************************************/
void UART_PutHex16 (LPC_UART_TypeDef *UARTx, uint16_t hexnum)
{
	uint8_t nibble, i;

	UART_Send(UARTx, (uint8_t *) "0x", 2, BLOCKING);
	i = 3;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		UART_SendByte(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}

/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (32-bit long)
 * @return		None
 **********************************************************************/
void UART_PutHex32 (LPC_UART_TypeDef *UARTx, uint32_t hexnum)
{
	uint8_t nibble, i;

	UART_Send(UARTx, (uint8_t *) "0x", 2, BLOCKING);
	i = 7;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		UART_SendByte(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}
