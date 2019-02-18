#ifndef __RECURRENT__INIT__
#define __RECURRENT__INIT__

#include <stdint.h>

enum supported_recurrent
{
    RECURENT_LINEAR = 0,

    RECURENT_COUNTER
};

struct update_context {
    uint8_t k;
    uint8_t b;
    int16_t x_prev;

    uint8_t (*update_fnc)(struct update_context *);
};

struct update_context_linear {
    struct update_context base_ctx;
    uint8_t is_convergens;
    uint8_t xmax;
};

struct update_context_trigonometric {
    struct update_context base_ctx;
    uint8_t step;
};

typedef uint8_t (*update_fnc)(struct update_context *);

uint8_t recurent_linear_update(struct update_context *ctx);

#define TO_LINEAR_CONTEXT(ctx)      ((struct update_context_linear *)(ctx))

#define TO_TRIGONOMETRIC_CONTEXT    ((struct update_context_trigonometric *)(ctx))

#endif /* __RECURRENT__INIT__ */