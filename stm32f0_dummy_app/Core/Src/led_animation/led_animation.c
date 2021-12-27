#include "led_animation.h"
#include "peripherals_init.h"

led_animation_fsm_t led_fsm;

#define LEDn  (3)
#define EXEC_TIME	(1000)

led_pin_port_t LEDs[LEDn] =
{
  {.pin = LED1_Pin, .port = LED1_GPIO_Port},
  {.pin = LED2_Pin, .port = LED2_GPIO_Port},
  {.pin = LED3_Pin, .port = LED3_GPIO_Port}
};

void led_pattern_init(void)
{
  led_animation_t blink = {
    .brightness = 7,
    .execution_time = EXEC_TIME,
    .period = EXEC_TIME/2,
    .time_on = EXEC_TIME/2
  };

  led_animation_init(&led_fsm, &LEDs[0], &blink);
  led_animation_start(&led_fsm);
}


void led_pattern_exec(void)
{
  if(led_animation_ongoing(&led_fsm) == false)
  {
    static uint8_t led_idx = 0;
    led_idx = (led_idx += 1)%LEDn;
    led_animation_set_gpio(&led_fsm, &LEDs[led_idx]);
    led_animation_start(&led_fsm);
  }

  led_animation_run(&led_fsm);
}
