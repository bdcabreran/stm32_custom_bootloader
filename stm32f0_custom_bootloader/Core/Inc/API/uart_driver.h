#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "circular_buffer.h"
#include "stm32f0xx_hal.h"

#define MAX_DATA_CHUNK_SIZE           (20) 


typedef struct
{
    struct
    {
        uint8_t *buffer;        /* Received Data over UART are stored in this buffer */
        c_buff_handle_t cb;     /* pointer typedef to circular buffer struct */
        uint8_t byte;           /* used to active RX reception interrupt mode */ 
    } rx;

    struct
    {
        uint8_t *buffer;       /* Data to be transmitted via UART are stored in this buffer */
        c_buff_handle_t cb;    /* pointer typedef to circular buffer struct */
    } tx;

}uart_data_t;

/**
 * @brief Uart Driver Definition
 * 
 */
typedef struct
{
    uart_data_t data;
    UART_HandleTypeDef handle;
    
}uart_driver_t;


uint8_t uart_init_it(uart_driver_t *driver, uint8_t *rx_buff, uint16_t rx_len,
                     uint8_t *tx_buff, uint16_t tx_len);
uint8_t uart_get_rx_data_len(uart_driver_t *driver);
uint8_t uart_read_rx_data(uart_driver_t *driver, uint8_t *data, uint8_t len);
uint8_t uart_fetch_rx_data(uart_driver_t *driver, uint8_t *data, uint8_t len);
uint8_t uart_clear_rx_data(uart_driver_t *driver);
uint8_t uart_transmit(uart_driver_t *driver, uint8_t *data, uint8_t len);
uint8_t uart_transmit_it(uart_driver_t *driver, uint8_t *data, uint8_t len);
uint8_t uart_write_rx_data(uart_driver_t *driver, uint8_t *data, uint8_t len);

#endif