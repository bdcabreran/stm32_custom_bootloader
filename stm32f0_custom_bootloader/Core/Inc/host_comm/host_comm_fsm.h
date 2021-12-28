#ifndef HOST_COMM_FSM_H
#define HOST_COMM_FSM_H

#include "host_comm_rx_fsm.h"

void host_comm_fsm_init(uart_driver_t *driver);
void host_comm_fsm_run(void);
void host_comm_fsm_time_event_update(void);

#endif