
#ifndef WS2812_H_
#define WS2812_H_

//--------------------------------------------------

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//--------------------------------------------------
#define DELAY_LEN 48
#define LED_COUNT 144
#define ARRAY_LEN DELAY_LEN + LED_COUNT*24
//--------------------------------------------------

#define HIGH 59
#define LOW 26

#define BitIsSet(reg, bit) ((reg & (1<<bit)) != 0)

void ws2812_init(void);
void ws2812_pixel_rgb_to_buf_dma(uint8_t Rpixel, uint8_t Gpixel, uint8_t Bpixel, uint16_t posX);
void ws2812_prepareValue (uint8_t r00, uint8_t g00, uint8_t b00,
    uint8_t r01, uint8_t g01, uint8_t b01,
    uint8_t r02, uint8_t g02, uint8_t b02,
    uint8_t r03, uint8_t g03, uint8_t b03,
    uint8_t r04, uint8_t g04, uint8_t b04,
    uint8_t r05, uint8_t g05, uint8_t b05,
    uint8_t r06, uint8_t g06, uint8_t b06,
    uint8_t r07, uint8_t g07, uint8_t b07,
    uint8_t r08, uint8_t g08, uint8_t b08,
    uint8_t r09, uint8_t g09, uint8_t b09,
    uint8_t r10, uint8_t g10, uint8_t b10,
    uint8_t r11, uint8_t g11, uint8_t b11);
void ws2812_setValue(void);
void ws2812_update();
void ws2812_test01(void);
void ws2812_test02(void);
#endif /* WS2812_H_ */
