#ifndef __RECURRENT__INIT__
#define __RECURRENT__INIT__

#include <stdint.h>

typedef uint8_t (*update_fnc)(struct update_context *);

struct update_context {
    uint8_t k;
    uint8_t b;
    uint8_t xmax;
    uint8_t is_convergens;
    int16_t x_prev;
};

uint8_t recurent_linear_update(struct update_context *ctx);

#endif /* __RECURRENT__INIT__ */