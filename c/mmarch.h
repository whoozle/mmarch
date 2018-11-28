#ifndef _MMARCH_H
#define _MMARCH_H

#ifdef __cplusplus
extern "C"
{
#endif

    struct mmarch_ops
    {
        void * (*map)(void *ctx, off_t offset, size_t size);
        void (*unmap)(void *ctx, void * data, size_t size);
    };

    struct mmarch_loader
    {
        
    };

#ifdef __cplusplus
}
#endif


#endif
