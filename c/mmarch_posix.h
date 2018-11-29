#ifndef _MMARCH_POSIX_H
#define _MMARCH_POSIX_H

#include "mmarch.h"

#ifdef __cplusplus
extern "C"
{
#endif

	struct mmarch_context_posix
	{
		union { struct mmarch_context context; } base;
		int fd;
		int protection;
		int flags;
	};

	void mmarch_context_posix_init(struct mmarch_context_posix * context);
	void mmarch_context_posix_deinit(struct mmarch_context_posix * context);
	mmarch_error mmarch_context_posix_load(struct mmarch_context_posix * context);

#ifdef __cplusplus
}
#endif


#endif
