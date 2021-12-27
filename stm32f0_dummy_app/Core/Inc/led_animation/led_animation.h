
#ifndef LED_ANIMATION_H
#define LED_ANIMATION_H

#include "led_animation_fsm.h"

extern led_animation_fsm_t led_fsm;

void led_pattern_init(void);
void led_pattern_exec(void);


#endif 
