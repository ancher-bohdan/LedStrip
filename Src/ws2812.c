#include "ws2812.h"

static uint8_t led_buffer[BUFFER_COUNT][BUFFER_SIZE * BYTE_PER_LED];
static struct __led_buffer *read = NULL, *write= NULL;

static struct __led_buffer **__alloc_ring_buffer(struct __led_buffer **prev)
{
    static int recursion_count = 0;

    *prev = (struct __led_buffer *)malloc(sizeof(struct __led_buffer));

    if(*prev == NULL)
        return NULL;

    (*prev)->buffer = led_buffer[recursion_count];
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
    uint8_t *buf = led_buffer[0];
    for(i = 0; i < BUFFER_COUNT; i++)
        for(j = 0; j < BUFFER_SIZE; j++)
        {
            increment24((uint24_t *)buf);
            buf += 3;
        }
}

int initialise_buffer(void)
{
    struct __led_buffer **last = __alloc_ring_buffer(&read);

    if(last == NULL) return ENOMEM;

    __fill_led_buffer();

    (*last)->next = read;

    write = read;

    return 0;
}

