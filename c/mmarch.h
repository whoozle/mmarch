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

    struct mmarch_context
    {
        int (*map)  (struct mmarch_context *context, void **ptr, off_t offset, size_t size);
        int (*unmap)(struct mmarch_context *context, void * data, size_t size);
    };

    struct mmarch_loader
    {
        void *context;
    };

#ifdef __cplusplus
}
#endif


#endif
