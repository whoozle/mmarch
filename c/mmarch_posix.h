#ifndef _MMARCH_POSIX_H
#define _MMARCH_POSIX_H

#include "mmarch.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct mmarch_posix_context
    {
        int fd;
    };

    void * mmarch_posix_op_map(void *context, off_t offset, size_t size);
    void mmarch_posix_op_unmap(void *context, void * data, size_t size);
    void mmarch_posix_op_init(mmarch_ops * ops);

#ifdef __cplusplus
}
#endif


#endif
