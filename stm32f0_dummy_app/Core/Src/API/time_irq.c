
#include "time_event.h"
#include "stm32f0xx_hal.h"
#include "led_animation.h"
#include "host_comm_rx_fsm.h"

/**
 * @brief Systick Callback Function 
 * @note  This callback is executed every ms
 */
void HAL_SYSTICK_Callback(void)
{
    /* update FSM time events*/
    led_animation_update_timers(&led_fsm);
    host_comm_rx_fsm_time_event_update(&host_comm_rx_handle);
    host_comm_tx_fsm_time_event_update(&host_comm_tx_handle);
}
