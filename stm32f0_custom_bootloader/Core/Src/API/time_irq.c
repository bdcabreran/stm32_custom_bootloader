
#include "time_event.h"
#include "stm32f0xx_hal.h"
#include "led_animation.h"


/**
 * @brief Systick Callback Function 
 * @note  This callback is executed every ms
 */
void HAL_SYSTICK_Callback(void)
{
    /* update FSM time events*/
    led_animation_update_timers(&led1_fsm);
    led_animation_update_timers(&led2_fsm);
    led_animation_update_timers(&led3_fsm);

}
