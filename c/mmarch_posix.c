#include "mmarch_posix.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

static mmarch_error mmarch_posix_op_map(struct mmarch_context *context, void **ptr, off_t offset, size_t size)
{
	return EMMARCH_OK;
}

static mmarch_error mmarch_posix_op_unmap(struct mmarch_context *context, void * data, size_t size)
{
	return EMMARCH_OK;
}


void mmarch_context_posix_init(struct mmarch_context_posix * context)
{
	struct mmarch_context * base = & context->base.context;
	mmarch_context_init(base);
	base->map = &mmarch_posix_op_map;
	base->unmap = &mmarch_posix_op_unmap;
	context->fd = -1;
	context->flags = MAP_SHARED;
	context->protection = PROT_READ;
}

void mmarch_context_posix_deinit(struct mmarch_context_posix * context)
{
	mmarch_context_deinit(&context->base.context);
}

mmarch_error mmarch_context_posix_load(struct mmarch_context_posix * context)
{
	lseek(context->fd, 0, SEEK_SET);

	uint8_t buf[MMARCH_HEADER_SIZE];
	if (read(context->fd, buf, sizeof(buf)) != sizeof(buf))
		return EMMARCH_READ_FAILED;

	return mmarch_context_load(&context->base.context, buf);
}
