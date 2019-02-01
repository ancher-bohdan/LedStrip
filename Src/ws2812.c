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

static void __led_set(struct __led *buf, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        buf->R[7 - i] = (r & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        buf->G[7 - i] = (g & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        buf->B[7 - i] = (b & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
    }
}

static void __fill_led_buffer(void)
{
    uint8_t i = 0, j = 0;
    for(i = 0; i < BUFFER_COUNT; i++)
        for(j = 0; j < BUFFER_SIZE; j++)
        {
            if(i == 0) __led_set(&(led_buffer.buffer[i][j]), 0, 0, 0xFF);
            else __led_set(&(led_buffer.buffer[i][j]), 0, 0xFF, 0);
        }
}

static void __prepare_list_handle(struct ws2812_list_handler *handler)
{
    struct __led_buffer_node *tmp = handler->read;
    uint8_t i;

    for(i = 0; i < BUFFER_COUNT; i++)
    {
        tmp->state = LB_STATE_BUSY;
        tmp = tmp->next;
    }

    handler->read = handler->write;
}

int initialise_buffer(void (*start_dma)(void *ptr, uint16_t size), void (*stop_dma)())
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

int ws2812_transfer_recurrent(void (*update)(uint32_t *ptr, uint8_t size), uint32_t count)
{
    __prepare_list_handle(&led_buffer);

    led_buffer.read->state = LB_STATE_IN_PROGRESS;
    __external_functions.__start_dma_fnc((uint32_t *)(led_buffer.read->buffer), 
                    BUFFER_COUNT * BUFFER_SIZE * WORDS_PER_LED);

    while(count > 0)
    {
        if(led_buffer.write != led_buffer.read)
        {
            assert_param(led_buffer.write->state == LB_STATE_FREE);

            if(update != NULL) 
                update((uint32_t *)(led_buffer.write->buffer), BUFFER_SIZE);
        
            led_buffer.write->state = LB_STATE_BUSY;
            led_buffer.write = led_buffer.write->next;

            count -= BUFFER_SIZE;
        }
    }

    __external_functions.__stop_dma_fnc();

    return 0;
}

void ws2812_interrupt()
{
    assert_param(led_buffer.read->state == LB_STATE_IN_PROGRESS);

    led_buffer.read->state = LB_STATE_FREE;
    led_buffer.read = led_buffer.read->next;
    led_buffer.read->state = LB_STATE_IN_PROGRESS;
}

