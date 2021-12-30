/**
 * @file packet_proc_fsm.h
 * @author Bayron Cabrera (bdcabreran@unal.edu.com)
 * @brief 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef LED_ANIMATION_FSM_H
#define LED_ANIMATION_FSM_H

#include <time_event.h>
#include <stm32f0xx_hal.h>
#include <stdint.h> 
#include <stdbool.h>
#include <string.h>


#define LED_ENDLESS_EXEC_TIME       (0xFFFFFFFF)
#define LED_BRIGHT_REFRESH_RATE     (25)                        // time in milliseconds -> 40Hz
#define LED_MAX_BRIGHTNESS          (LED_BRIGHT_REFRESH_RATE)   // [0 to LED_BRIGHT_REFRESH_RATE]



typedef struct
{
    GPIO_TypeDef* port;
    uint16_t pin;
}led_pin_port_t;

typedef struct
{
    uint8_t  brightness;     // [0 - LED_MAX_BRIGHTNESS] 
    uint16_t period;         // ms 
    uint16_t time_on;        // ms [ time_on < period ]   
    uint32_t execution_time; // ms pattern execution time    
}led_animation_t;

typedef enum
{
    ev_led_animation_invalid = 0x00,
    ev_led_animation_start,
    ev_led_animation_stop,
    ev_led_animation_completed,
    ev_led_animation_last
}led_animation_event_name_t;

typedef enum
{
    st_led_animation_invalid = 0x00,
    st_led_animation_idle,
    st_led_animation_exec,
    st_led_animation_last

}led_animation_state_t;

typedef struct
{
    time_event_t exec_time_expired;
    time_event_t time_on_expired;
    time_event_t period_expired;

    /*reserved used for bright settings*/
    time_event_t bright_amount;          // brightness
    time_event_t bright_refresh;         // refresh rate

}led_animation_event_time_t;

typedef struct
{
    led_animation_event_name_t name;
    led_animation_event_time_t time;

}led_animation_event_t;


typedef struct
{
    led_animation_t animation;
    led_pin_port_t    gpio;
}led_animation_iface_t;

typedef struct
{
    led_animation_event_t event;
    led_animation_state_t state;
    led_animation_iface_t iface;
}led_animation_fsm_t;

void led_animation_init(led_animation_fsm_t *handle, led_pin_port_t *gpio, led_animation_t *animation);

uint8_t led_animation_set(led_animation_fsm_t *handle, led_animation_t *animation);
uint8_t led_animation_set_brightness(led_animation_fsm_t *handle, uint8_t brightness);

uint8_t led_animation_start(led_animation_fsm_t *handle);
void led_animation_stop(led_animation_fsm_t *handle);

void led_animation_run(led_animation_fsm_t *handle);
bool led_animation_ongoing(led_animation_fsm_t *handle);
void led_animation_update_timers(led_animation_fsm_t *handle);



#endif
