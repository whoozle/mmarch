#ifndef _MMARCH_POSIX_H
#define _MMARCH_POSIX_H

#include "mmarch.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct mmarch_posix_context
    {
        union { struct mmarch_context context; } base;
        int fd;
        int protection;
        int flags;
    };

    int mmarch_posix_op_map(struct mmarch_context *context, void **ptr, off_t offset, size_t size);
    int mmarch_posix_op_unmap(struct mmarch_context *context, void * data, size_t size);

    void mmarch_posix_context_init_default(struct mmarch_posix_context * context);

#ifdef __cplusplus
}
#endif


#endif
