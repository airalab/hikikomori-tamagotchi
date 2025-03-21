#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include "led_strip.h"

#define BLINK_GPIO 8

void RGB_Init(void);
void Set_RGB( uint8_t red_val, uint8_t green_val, uint8_t blue_val);
void RGB_Example(void);
void RGB_turn_off(void);
void RGB_turn_on(void);

#ifdef __cplusplus
}
#endif