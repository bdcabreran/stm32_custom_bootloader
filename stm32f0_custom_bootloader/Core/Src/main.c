/**
  ******************************************************************************
  * @file           : main.c
  * @author         : Bayron Cabrera (bdcabreran@unal.edu.co)
  * @brief          : Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "peripherals_init.h"
#include "led_animation.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/


void print_startup_message(void)
{
	printf("**************************************\r\n");
	printf("Brief:\t Custom Bootloader\r\n");
	printf("Author:\t Bayron Cabrera \r\n");
	printf("Board:\t STM32F0 - M0 \r\n");
	printf("Date:\t %s\r\n", __DATE__);
	printf("**************************************\r\n");
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  peripherals_init();
  print_startup_message();
  led_breath_init();
  
  while (1)
  {
	  led_breath_exec();
  }
}

 


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
