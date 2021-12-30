
#ifndef LED_ANIMATION_H
#define LED_ANIMATION_H

#include "led_animation_fsm.h"

extern led_animation_fsm_t led1_fsm;
extern led_animation_fsm_t led2_fsm;
extern led_animation_fsm_t led3_fsm;

void led_bootloader_init(void);
void led_bootloader_run(void);



#endif 
