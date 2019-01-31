#include "ws2812.h"

static struct ws2812_operation __external_functions;

static struct ws2812_list_handler led_buffer = {
    .read = NULL,
    .write = NULL,
    .flags = 0
};

static struct __led_buffer_node **__alloc_ring_buffer(struct __led_buffer_node **prev)
{
    static int recursion_count = 0;

    *prev = (struct __led_buffer_node *)malloc(sizeof(struct __led_buffer_node));

    if(*prev == NULL)
        return NULL;

    (*prev)->buffer = led_buffer.buffer[recursion_count];
    (*prev)->state = LB_STATE_BUSY;

    if(recursion_count != (BUFFER_COUNT - 1)) {
        recursion_count++;
        return __alloc_ring_buffer(&((*prev)->next));
    } else {
        return prev;
    }
}

static void __fill_led_buffer(void)
{
    uint8_t i = 0, j = 0;
    uint8_t *buf = led_buffer.buffer[0];
    for(i = 0; i < BUFFER_COUNT; i++)
        for(j = 0; j < BUFFER_SIZE; j++)
        {
            increment24((uint24_t *)buf);
            buf += 3;
        }
}

int initialise_buffer(void (*start_dma)(void *ptr, uint8_t size), void (*stop_dma)())
{
    struct __led_buffer_node **last;

    __fill_led_buffer();

    last = __alloc_ring_buffer(&led_buffer.read);

    if(last == NULL) return ENOMEM;

    (*last)->next = led_buffer.read;

    led_buffer.write = led_buffer.read;

    __external_functions.__start_dma_fnc = start_dma;
    __external_functions.__stop_dma_fnc = stop_dma;

    return 0;
}

