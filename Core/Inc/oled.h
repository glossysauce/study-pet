#ifndef OLED_H
#define OLED_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

void oled_init(void);
void oled_on(void);
void oled_off(void);
void oled_fill(uint16_t color);
void oled_fill_fixed(uint16_t color);
void oled_clear_window(void);
void oled_rect_hw_red(void);
void oled_clear_hw(void);

#define DC_Pin        GPIO_PIN_3
#define DC_GPIO_Port  GPIOB

#define CS_Pin        GPIO_PIN_6
#define CS_GPIO_Port  GPIOB

#define RST_Pin       GPIO_PIN_4
#define RST_GPIO_Port GPIOB

#endif
