
#include "time_event.h"
#include "stm32f0xx_hal.h"
#include "led_animation.h"
#include "host_comm_fsm.h"
#include "bootloader_fsm.h"


/**
 * @brief Systick Callback Function 
 * @note  This callback is executed every ms
 */
void HAL_SYSTICK_Callback(void)
{
    /* update FSM time events*/
    led_bootloader_time_event_update();

    /* update host comm fsm time events */
    host_comm_fsm_time_event_update();

    /* bootloader fsm time event update */
    boot_fsm_fsm_time_event_update(&boot_handle);
}
