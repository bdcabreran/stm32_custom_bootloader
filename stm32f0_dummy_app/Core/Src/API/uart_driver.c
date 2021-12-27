/**
 * @file uart.c
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "uart_driver.h"

#define USE_UART1 
#define USE_UART2 

/**
 * @brief Extern UART driver Objects 
 * 
 */

#ifdef USE_UART1 
extern uart_driver_t uart1;
#endif

#ifdef USE_UART2
extern uart_driver_t uart2;
#endif


extern void Error_Handler(void);

/**@brief Enable/Disable debug messages */
#define UART_DRIVER_DEBUG 0
#define UART_DRIVER_TAG "uart driver : "

/**@brief uart debug function for server comm operations  */
#if UART_DRIVER_DEBUG
#define uart_driver_dbg(format, ...) printf(UART_DRIVER_TAG format, ##__VA_ARGS__)
#else
#define uart_driver_dbg(format, ...) \
    do                                    \
    { /* Do nothing */                    \
    } while (0)
#endif


/**
 * @brief Init host comm peripheral interface
 * 
 * @param rx_buff buffer in stack reserved for data reception 
 * @param tx_buff buffer in stack reserved for data transmission
 * @return uint8_t 
 */
uint8_t uart_init_it(uart_driver_t *driver, uint8_t *rx_buff, uint16_t rx_len, uint8_t *tx_buff, uint16_t tx_len)
{
    /*Init default Configuration */
    driver->handle.Init.BaudRate = 115200;
    driver->handle.Init.WordLength = UART_WORDLENGTH_8B;
    driver->handle.Init.StopBits = UART_STOPBITS_1;
    driver->handle.Init.Parity = UART_PARITY_NONE;
    driver->handle.Init.Mode = UART_MODE_TX_RX;
    driver->handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    driver->handle.Init.OverSampling = UART_OVERSAMPLING_16;
    driver->handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    driver->handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&driver->handle) != HAL_OK)
    {
        Error_Handler();
    }

    /*Init Circular Buffer*/
    driver->data.rx.buffer = rx_buff;
    driver->data.tx.buffer = tx_buff;
    driver->data.rx.cb = circular_buff_init(driver->data.rx.buffer, rx_len);
    driver->data.tx.cb = circular_buff_init(driver->data.tx.buffer, tx_len);

    /*Start Reception of data*/
    HAL_UART_Receive_IT(&driver->handle, &driver->data.rx.byte, 1);

    uart_driver_dbg("comm driver info : uart it mode initialized\r\n");

    return 1;
}

uint8_t uart_get_rx_data_len(uart_driver_t *driver)
{
    return circular_buff_get_data_len(driver->data.rx.cb);
}


uint8_t uart_read_rx_data(uart_driver_t *driver, uint8_t *data, uint8_t len)
{
    return circular_buff_read(driver->data.rx.cb, data, len);
}


uint8_t uart_fetch_rx_data(uart_driver_t *driver, uint8_t *data, uint8_t len)
{
    return circular_buff_fetch(driver->data.rx.cb, data, len);
}


uint8_t uart_clear_rx_data(uart_driver_t *driver)
{
    circular_buff_reset(driver->data.rx.cb);
    return 1;
}

uint8_t uart_transmit(uart_driver_t *driver, uint8_t *data, uint8_t len)
{
    return HAL_UART_Transmit(&driver->handle, data, len, HAL_MAX_DELAY);
}

uint8_t uart_transmit_it(uart_driver_t *driver, uint8_t *data, uint8_t len)
{
    /* Write data to circular buffer */
    if (circular_buff_write(driver->data.tx.cb, data, len) == CIRCULAR_BUFF_OK)
    {
        if (driver->handle.gState == HAL_UART_STATE_READY)
        {
            uint8_t byte;
            circular_buff_get(driver->data.tx.cb, &byte);
            HAL_UART_Transmit_IT(&driver->handle, &byte, 1);
        }
        else
        {
            uart_driver_dbg("comm driver warning:\t uart busy\r\n");
        }

        return 1;
    }

    uart_driver_dbg("comm driver error:\t circular buffer cannot write request\r\n");
	return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  static uint8_t data_chunk[MAX_DATA_CHUNK_SIZE];
  uart_driver_t *driver = NULL;

#ifdef USE_UART1
  if (huart->Instance == USART1)
      driver = &uart1;
#endif

#ifdef USE_UART2
  if (huart->Instance == USART2)
      driver = &uart2;
#endif

  /*WARNING! add here the uart instances to be used */

  if(driver != NULL)
  {
    /*check for pendings transfers */
    uint16_t data_len = circular_buff_get_data_len(driver->data.tx.cb);

    if(data_len)
    {
        data_len = (data_len >= MAX_DATA_CHUNK_SIZE ) ? (MAX_DATA_CHUNK_SIZE - 1) : data_len;
        circular_buff_read(driver->data.tx.cb, data_chunk, data_len);
        HAL_UART_Transmit_IT(&driver->handle, data_chunk, data_len);
    }

    uart_driver_dbg("comm driver info:\t irq uart tx complete\r\n");
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uart_driver_t *driver = NULL;

#ifdef USE_UART1
  if (huart->Instance == USART1)
      driver = &uart1;
#endif

#ifdef USE_UART2
  if (huart->Instance == USART2)
      driver = &uart2;
#endif

    /*WARNING! add here the uart instances to be used */

    if(driver != NULL)
    {
        /*Set Uart Data reception for next byte*/
        HAL_UART_Receive_IT(&driver->handle, &driver->data.rx.byte, 1);

        if(circular_buff_write(driver->data.rx.cb, &driver->data.rx.byte, 1) !=  CIRCULAR_BUFF_OK)
        {
            /*Reinit ring buffer*/
            circular_buff_reset(driver->data.rx.cb);
        }
    }
}

/* only for dbg*/
uint8_t uart_write_rx_data(uart_driver_t *driver, uint8_t *data, uint8_t len)
{
	circular_buff_st_t status = circular_buff_write(driver->data.rx.cb, data, len);
	if(status != CIRCULAR_BUFF_OK)
	{
	    uart_driver_dbg("comm driver error:\t circular buffer cannot write request\r\n");
	}
    return status;
}



