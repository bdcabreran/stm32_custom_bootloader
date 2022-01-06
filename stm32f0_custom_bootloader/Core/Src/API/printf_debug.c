/////////////////////////////////////////////////////////////////////////////////////////////////////////
//					Implementation of printf like feature using ARM Cortex M3/M4/ ITM functionality
//					This function will not work for ARM Cortex M0/M0+
//					If you are using Cortex M0, then you can use semihosting feature of openOCD
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stm32f0xx.h"
#include "uart_driver.h"

#define DEBUG_ENABLE
extern uart_driver_t uart1;


#ifdef DEBUG_ENABLE
#include "stdio.h"
int _write(int file, char *ptr, int len)
{

	uart_transmit_it(&uart1, (uint8_t*)ptr, len);
	return len;
}

#else
#define printf(format, ...) do{ /*Do nothing */ }while(0); 
#endif