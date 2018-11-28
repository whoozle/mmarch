#include "mmarch.h"

void mmarch_context_init(struct mmarch_context * context)
{
	context->map = NULL;
	context->unmap = NULL;
	context->user = NULL;
}
