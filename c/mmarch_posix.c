#include "mmarch_posix.h"
#include <sys/mman.h>


static int mmarch_posix_op_map(struct mmarch_context *context, void **ptr, off_t offset, size_t size)
{
	return MMARCH_OK;
}

static int mmarch_posix_op_unmap(struct mmarch_context *context, void * data, size_t size)
{
	return MMARCH_OK;
}


void mmarch_context_posix_init(struct mmarch_posix_context * context)
{
	struct mmarch_context * base = & context->base.context;
	mmarch_context_init(base);
	base->map = &mmarch_posix_op_map;
	base->unmap = &mmarch_posix_op_unmap;
	context->fd = -1;
	context->flags = MAP_SHARED;
	context->protection = PROT_READ;
}
