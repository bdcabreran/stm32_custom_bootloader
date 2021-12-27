
#ifndef PERIPHERALS_INIT_H
#define PERIPHERALS_INIT_H

#include "stm32f0xx_hal.h"
#include "uart_driver.h"

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_3
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_4
#define LED3_GPIO_Port GPIOB
#define USER_BTN_Pin GPIO_PIN_9
#define USER_BTN_GPIO_Port GPIOB

extern uart_driver_t uart1;
extern uart_driver_t uart2;

/* Public function prototypes -----------------------------------------------*/
void peripherals_init(void);

#endif