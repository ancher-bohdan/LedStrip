#ifndef WS2812_H_
#define WS2812_H_

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ----------------------
 * Config parameters
 * ---------------------*/

/* Number of leds in len strip */
#define LED_NUMBERS     144

/* Number of buffer that will be allocating */
#define BUFFER_COUNT    2

/* Size of each buffer */
#define BUFFER_SIZE     6

/* How many bytes need for each led */
#define WORDS_PER_LED    24

/* ---------------------------------
 * Macros for parameters validation
 * !!!! DO NOT MODIFY !!!!
 * -------------------------------- */
/* Value, that represent bit 1 in __dma_buffer struct */
#define LED_CODE_ONE    59

/* Value, that represent bit 0 in __dma_buffer struct */
#define LED_CODE_ZERO 26

#define NUMBER_OF_BUFFERS   LED_NUMBERS / BUFFER_SIZE

#if (NUMBER_OF_BUFFERS * BUFFER_SIZE < LED_NUMBERS)
#warning "Not all leds in led strip will be use. Modify config parameters, pls"
#endif

#if (NUMBER_OF_BUFFERS < BUFFER_COUNT)
#error "Redundant memory allocation. Correct config parameters, pls"
#endif

#define ENOMEM      -1


enum __led_buffer_state {
    LB_STATE_FREE = 0,
    LB_STATE_IN_PROGRESS,
    LB_STATE_BUSY,

    LB_COUNT_STATES,
};

struct __dma_buffer {
    uint32_t G[8];
    uint32_t R[8];
    uint32_t B[8];
};

struct __abstract {
    uint16_t first;
    uint8_t second;
    uint8_t third;
};

struct __rgb_buffer {
        uint16_t r;
        uint8_t g;
        uint8_t b;
};

struct __hsv_buffer {
        uint16_t h;
        uint8_t s;
        uint8_t v;
};

union __color {
    struct __rgb_buffer rgb;
    struct __hsv_buffer hsv;
    struct __abstract abstract;
};

struct __led_buffers {
    struct __dma_buffer dma_buffer[BUFFER_COUNT][BUFFER_SIZE];
    union  __color col[BUFFER_COUNT][BUFFER_SIZE];
};

struct __led_buffer_node {
    struct __dma_buffer *buffer;
    union  __color *col;
    struct __led_buffer_node *next;
    enum __led_buffer_state state;
};

struct ws2812_operation {
    void (*__start_dma_fnc)(void *ptr, uint16_t size);
    void (*__stop_dma_fnc)();

    void (*to_dma)(union __color *in, struct __dma_buffer *dst);
};

struct ws2812_list_handler {
    struct __led_buffer_node *read;
    struct __led_buffer_node *write;
    struct __led_buffers buffer;

    struct ws2812_operation wops;
};

int initialise_buffer(void (*start_dma)(void *ptr, uint16_t size), void (*stop_dma)());
int ws2812_transfer_recurrent(char *r_exp, char *g_exp, char *b_exp, uint8_t count);
void ws2812_interrupt();

#endif /* WS2812_H_ */
