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
#define BUFFER_COUNT    3

/* Size of each buffer */
#define BUFFER_SIZE     12

/* How many bytes need for each led */
#define BYTE_PER_LED    3

/* ---------------------------------
 * Macros for parameters validation
 * !!!! DO NOT MODIFY !!!!
 * -------------------------------- */
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

struct __led_buffer {
    uint8_t *buffer;
    struct __led_buffer *next;
    enum __led_buffer_state state;
};

struct ws2812_list_handler {
    struct __led_buffer *read;
    struct __led_buffer *write;
    uint8_t buffer[BUFFER_COUNT][BUFFER_SIZE * BYTE_PER_LED];
    uint8_t flags;
};

typedef struct __uint24_t {
    uint8_t data[3];
} uint24_t;

static inline void increment24(uint24_t *number)
{
    if(++(*(uint16_t *)number) == 0) number->data[3]++;
}

int initialise_buffer(void);

#endif /* WS2812_H_ */
