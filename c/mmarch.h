#ifndef _MMARCH_H
#define _MMARCH_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
		MMARCH_OK = 0,
        MMARCH_SYSTEM = -1,
		MMARCH_UNSUPPORTED = -2
    } mmarch_error;

    struct mmarch_context
    {
		void *			user;
		mmarch_error    (*map)  (struct mmarch_context *context, void **ptr, off_t offset, size_t size);
		mmarch_error    (*unmap)(struct mmarch_context *context, void * data, size_t size);
    };

	void mmarch_context_init(struct mmarch_context * context);

    struct mmarch_loader
    {
        struct mmarch_context * context;
    };

#ifdef __cplusplus
}
#endif


#endif
