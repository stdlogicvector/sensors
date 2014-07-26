#ifndef LPC17XX_LIBCFG_H_
#define LPC17XX_LIBCFG_H_

#include "lpc_types.h"

/******************* PERIPHERAL FW LIBRARY CONFIGURATION DEFINITIONS ***********************/

/* Comment the line below to disable the specific peripheral inclusion */


/* GPIO ------------------------------- */
#define _GPIO

/* EXTI ------------------------------- */
//#define _EXTI

/* UART ------------------------------- */
#define _UART
#define _UART0
//#define _UART1
//#define _UART2
//#define _UART3

/* SPI ------------------------------- */
//#define _SPI

/* SSP ------------------------------- */
//#define _SSP
//#define _SSP0
//#define _SSP1

/* SYSTICK --------------------------- */
//#define _SYSTICK

/* I2C ------------------------------- */
#define _I2C
#define _I2C0
//#define _I2C1
//#define _I2C2

/* TIMER ------------------------------- */
#define _TIM

/* WDT ------------------------------- */
//#define _WDT


/* GPDMA ------------------------------- */
//#define _GPDMA


/* DAC ------------------------------- */
//#define _DAC

/* DAC ------------------------------- */
#define _ADC


/* PWM ------------------------------- */
//#define _PWM
//#define _PWM1

/* RTC ------------------------------- */
//#define _RTC

/* I2S ------------------------------- */
//#define _I2S

/* USB device ------------------------------- */
//#define _USBDEV
//#define _USB_DMA

/* QEI ------------------------------- */
//#define _QEI

/* MCPWM ------------------------------- */
//#define _MCPWM

/* CAN--------------------------------*/
//#define _CAN

/* RIT ------------------------------- */
//#define _RIT

/* EMAC ------------------------------ */
//#define _EMAC


#endif /* LPC17XX_LIBCFG_H_ */
