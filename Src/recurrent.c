#include "recurrent.h"
#include "usr_math.h"

uint8_t recurent_linear_update(struct update_context *ctx)
{
    int16_t result;
    struct update_context_linear *lin_ctx = TO_LINEAR_CONTEXT(ctx);

    if(lin_ctx->is_convergens)
    {
        result = ctx->k * ctx->x_prev + ctx->b;
        ctx->x_prev++;
        if(result >= lin_ctx->xmax) {
            lin_ctx->is_convergens = 0;
            return lin_ctx->xmax;
        } 
        return (uint8_t)result;
    }
    else
    {
        result = (-1) * ctx->x_prev * ctx->k - ctx->b + (lin_ctx->xmax << 1);
        ctx->x_prev++;
        if(result <= 1) {
            lin_ctx->is_convergens = 1;
            ctx->x_prev = (-1) * (ctx->b / ctx->k);
            return 1;
        }
        return (uint8_t)result;
    }
}