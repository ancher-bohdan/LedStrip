#include "ws2812.h"
#include "recurrent.h"

#include <math.h>

static struct ws2812_operation __external_functions;

static struct ws2812_list_handler led_buffer = {
    .read = NULL,
    .write = NULL
};

static struct __led_buffer_node **__alloc_ring_buffer(struct __led_buffer_node **prev)
{
    static int recursion_count = 0;

    *prev = (struct __led_buffer_node *)malloc(sizeof(struct __led_buffer_node));

    if(*prev == NULL)
        return NULL;

    (*prev)->buffer = led_buffer.buffer.dma_buffer[recursion_count];
    (*prev)->rgb = led_buffer.buffer.rgb_buffer[recursion_count];
    (*prev)->hsv = led_buffer.buffer.hsv_buffer[recursion_count];
    (*prev)->state = LB_STATE_BUSY;

    if(recursion_count != (BUFFER_COUNT - 1)) {
        recursion_count++;
        return __alloc_ring_buffer(&((*prev)->next));
    } else {
        return prev;
    }
}

static void __rgb2dma(struct __rgb_buffer *src, struct __dma_buffer *dst)
{
    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        dst->R[7 - i] = ((src->r) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        dst->G[7 - i] = ((src->g) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        dst->B[7 - i] = ((src->b) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
    }
}

static void __rgb2hsv(struct __rgb_buffer *src, struct __hsv_buffer *dst)
{
    double      min, max, delta;

    min = src->r < src->g ? src->r : src->g;
    min = min  < src->b ? min  : src->b;

    max = src->r > src->g ? src->r : src->g;
    max = max  > src->b ? max  : src->b;

    dst->v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        dst->s = 0;
        dst->h = 0; // undefined, maybe nan?
        return;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        dst->s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        dst->s = 0.0;
        dst->h = NAN;                            // its now undefined
        return;
    }
    if( src->r >= max )                           // > is bogus, just keeps compilor happy
        dst->h = ( src->g - src->b ) / delta;        // between yellow & magenta
    else
    if( src->g >= max )
        dst->h = 2.0 + ( src->b - src->r ) / delta;  // between cyan & yellow
    else
        dst->h = 4.0 + ( src->r - src->g ) / delta;  // between magenta & cyan

    dst->h *= 60.0;                              // degrees

    if( dst->h < 0.0 )
        dst->h += 360.0;
}

static void __hsv2rgb(struct __hsv_buffer *src, struct __rgb_buffer *dst)
{
    double      hh, p, q, t, ff;
    long        i;

    if(src->s <= 0.0) {       // < is bogus, just shuts up warnings
        dst->r = src->v;
        dst->g = src->v;
        dst->b = src->v;
    }
    hh = src->h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = src->v * (1.0 - src->s);
    q = src->v * (1.0 - (src->s * ff));
    t = src->v * (1.0 - (src->s * (1.0 - ff)));

    switch(i) {
    case 0:
        dst->r = src->v;
        dst->g = t;
        dst->b = p;
        break;
    case 1:
        dst->r = q;
        dst->g = src->v;
        dst->b = p;
        break;
    case 2:
        dst->r = p;
        dst->g = src->v;
        dst->b = t;
        break;

    case 3:
        dst->r = p;
        dst->g = q;
        dst->b = src->v;
        break;
    case 4:
        dst->r = t;
        dst->g = p;
        dst->b = src->v;
        break;
    case 5:
    default:
        dst->r = src->v;
        dst->g = p;
        dst->b = q;
        break;
    }
}

static void __prepare_list_handle(void)
{
    struct __led_buffer_node *tmp = led_buffer.read;
    uint8_t i;

    for(i = 0; i < BUFFER_COUNT; i++)
    {
        if(tmp->buffer == led_buffer.buffer.dma_buffer) led_buffer.write = tmp;
        tmp->state = LB_STATE_BUSY;
        tmp = tmp->next;
    }

    led_buffer.read = led_buffer.write;
}

int initialise_buffer(void (*start_dma)(void *ptr, uint16_t size), void (*stop_dma)())
{
    struct __led_buffer_node **last;

    last = __alloc_ring_buffer(&led_buffer.read);

    if(last == NULL) return ENOMEM;

    (*last)->next = led_buffer.read;

    led_buffer.write = led_buffer.read;

    __external_functions.__start_dma_fnc = start_dma;
    __external_functions.__stop_dma_fnc = stop_dma;

    return 0;
}

static struct update_context *parse_recurrent_param(char *param)
{
    struct update_context *result;
    char *token;
    int code = 0, k = 0, b = 0;
    char func[4];

    if(param == NULL)
    {
        result = (struct update_context *)malloc(sizeof(struct update_context_linear));

        if(result == NULL) goto exit;

        result->k = 0;
        result->b = 0;
        result->x_prev = 0;
        TO_LINEAR_CONTEXT(result)->xmax = 255;
        TO_LINEAR_CONTEXT(result)->is_convergens = 1;
        result->update_fnc = recurent_linear_update;
        return result;
    }

    token = strtok(param, ";");
    code = sscanf(token, "%d*%3s+%d", &k, func, &b);

    if(code != 3) goto error;

    if(!strcmp(func, "lin"))
    {
        result = (struct update_context *)malloc(sizeof(struct update_context_linear));

        if(result == NULL) goto exit;

        result->update_fnc = recurent_linear_update;
        result->b = b;
        result->k = k;
        
        token = strtok(NULL, ";");
        code = sscanf(token, "%d...%d", &k, &b);

        if(code != 2) goto error;

        result->x_prev = k;
        TO_LINEAR_CONTEXT(result)->xmax = b;
        TO_LINEAR_CONTEXT(result)->is_convergens = 1;
    }
    else if( (!strcmp(func, "sin")) || (!strcmp(func, "cos")) )
    {
        result = (struct update_context *)malloc(sizeof(struct update_context_trigonometric));

        if(result == NULL) goto exit;

        result->update_fnc = recurent_sin_update;
        result->b = b;
        result->k = k;
        
        token = strtok(NULL, ";");
        code = sscanf(token, "%d...%d", &k, &b);

        if(code != 2) goto error;

        result->x_prev = k;
        TO_TRIGONOMETRIC_CONTEXT(result)->step = b;
        if(!strcmp(func, "cos"))
        {
            result->x_prev += 90;
            if(result->x_prev >= 360) result->x_prev -= 360; 

        }
    }
    else
        goto error;

    goto exit;

error:
    free(result);
    result = NULL;
exit:
    return result;
}

int ws2812_transfer_recurrent(char *r_exp, char *g_exp, char *b_exp, uint8_t count)
{
    uint8_t i, j;
    struct update_context *update_r, *update_b, *update_g;
    struct __led_buffer_node *tmp = led_buffer.read;

    update_r = parse_recurrent_param(r_exp);
    update_g = parse_recurrent_param(g_exp);
    update_b = parse_recurrent_param(b_exp);

    if(update_b == NULL || update_r == NULL || update_g == NULL) return 1;

    __prepare_list_handle();

    memset(tmp->buffer, 0, BUFFER_SIZE * WORDS_PER_LED * 4);
    tmp = tmp->next;

    for(i = 1; i < BUFFER_COUNT; i++)
    {
        for(j = 0; j < BUFFER_SIZE; j++)
        {
            tmp->rgb[j].b = update_b->update_fnc(update_b);
            tmp->rgb[j].g = update_g->update_fnc(update_g);
            tmp->rgb[j].r = update_r->update_fnc(update_r);

            __rgb2dma(&(tmp->rgb[j]), &(tmp->buffer[j]));
        }
        tmp = tmp->next;
    }

    led_buffer.read->state = LB_STATE_IN_PROGRESS;
    __external_functions.__start_dma_fnc((uint32_t *)(led_buffer.buffer.dma_buffer), 
                    BUFFER_COUNT * BUFFER_SIZE * WORDS_PER_LED);
    while(1)
    {
        if(led_buffer.write != led_buffer.read)
        {
            count -= BUFFER_SIZE;
            if(count <= 0) break;

            assert_param(led_buffer.write->state == LB_STATE_FREE);

            for(i = 0; i < BUFFER_SIZE; i++)
            {
                led_buffer.write->rgb[i].b = update_b->update_fnc(update_b);
                led_buffer.write->rgb[i].g = update_g->update_fnc(update_g);
                led_buffer.write->rgb[i].r = update_r->update_fnc(update_r);
                __rgb2dma(&(led_buffer.write->rgb[i]), &(led_buffer.write->buffer[i]));
            }

            led_buffer.write->state = LB_STATE_BUSY;
            led_buffer.write = led_buffer.write->next;
        }
    }

    __external_functions.__stop_dma_fnc();

    free(update_r);
    free(update_g);
    free(update_b);

    return 0;
}

void ws2812_interrupt()
{
    assert_param(led_buffer.read->state == LB_STATE_IN_PROGRESS);

    led_buffer.read->state = LB_STATE_FREE;
    led_buffer.read = led_buffer.read->next;
    led_buffer.read->state = LB_STATE_IN_PROGRESS;
}

