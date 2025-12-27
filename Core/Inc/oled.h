#ifndef OLED_H
#define OLED_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

void oled_init(void);
void oled_on(void);
void oled_off(void);
void oled_fill(uint16_t color);
void oled_clear_window(void);
void oled_rect_hw_red(void);
void oled_clear_hw(void);
void oled_draw_pixel(uint8_t x, uint8_t y, uint16_t c);
void oled_draw_block(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color); //nw
void oled_block_rgb565(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color); //nw
void test(void); //working
void oled_gac_fill_rect(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1, uint8_t r,uint8_t g,uint8_t b); //working

#define DC_Pin        GPIO_PIN_3
#define DC_GPIO_Port  GPIOB

#define CS_Pin        GPIO_PIN_6
#define CS_GPIO_Port  GPIOB

#define RST_Pin       GPIO_PIN_4
#define RST_GPIO_Port GPIOB

#endif
