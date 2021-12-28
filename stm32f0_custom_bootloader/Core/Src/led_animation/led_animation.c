#include "led_animation.h"
#include "peripherals_init.h"

led_animation_fsm_t led1_fsm;
led_animation_fsm_t led2_fsm;
led_animation_fsm_t led3_fsm;

void led_breath_init(void)
{
  led_animation_t breath = {
    .brightness = LED_MAX_BRIGHTNESS,
    .execution_time = LED_ENDLESS_EXEC_TIME,
    .period = 100,  // if period == time on, it means always on
    .time_on = 100  
  };

  led_pin_port_t gpio = {
    .pin =LED1_Pin,
    .port = LED1_GPIO_Port
  };

  led_animation_init(&led1_fsm, &gpio, &breath);
  led_animation_start(&led1_fsm);
}


void led_breath_run(void)
{
  led_animation_run(&led1_fsm);

  static uint32_t millis_counter = 0;
  static uint8_t  brightness = 0;
  static int  fade_amount = 1;

  /*Update the brightness every 30ms */
  if (HAL_GetTick() - millis_counter > 30)
  {
    millis_counter = HAL_GetTick();
    
    brightness = (brightness + fade_amount) % LED_MAX_BRIGHTNESS;
    if (brightness >= (LED_MAX_BRIGHTNESS-1) || brightness <= 0)
      fade_amount = ~fade_amount;

    led_animation_set_brightness(&led1_fsm, brightness);
  }
}
