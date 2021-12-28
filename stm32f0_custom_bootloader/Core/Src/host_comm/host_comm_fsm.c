
#include "host_comm_fsm.h"

void host_comm_fsm_init(uart_driver_t *driver)
{
  host_comm_rx_fsm_init(&host_comm_rx_handle, driver);
  host_comm_tx_fsm_init(&host_comm_tx_handle, driver);
}

void host_comm_fsm_run(void)
{
    host_comm_rx_fsm_run(&host_comm_rx_handle);
    host_comm_tx_fsm_run(&host_comm_tx_handle);
}

void host_comm_fsm_time_event_update(void)
{
    host_comm_rx_fsm_time_event_update(&host_comm_rx_handle);
    host_comm_tx_fsm_time_event_update(&host_comm_tx_handle);
}