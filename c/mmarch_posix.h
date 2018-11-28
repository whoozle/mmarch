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

    void mmarch_context_posix_init(struct mmarch_posix_context * context);

#ifdef __cplusplus
}
#endif


#endif
