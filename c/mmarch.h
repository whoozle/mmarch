#ifndef _MMARCH_H
#define _MMARCH_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif
    enum error
    {
        MERROR
    };

    struct mmarch_ops
    {
        int (*map)  (void **ptr, void *context, off_t offset, size_t size);
        int (*unmap)(void *context, void * data, size_t size);
    };

    struct mmarch_loader
    {
        void *context;
    };

#ifdef __cplusplus
}
#endif


#endif
