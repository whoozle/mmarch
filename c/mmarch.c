#include "mmarch.h"
#include "mmarch_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t mmarch_get_header_size()
{
	STATIC_ASSERT(sizeof(struct mmarch_file_header) == MMARCH_HEADER_SIZE);
	return sizeof(struct mmarch_file_header);
}

const char * mmarch_get_error(mmarch_error error)
{
	switch(error)
	{
		case EMMARCH_READ_FAILED:
			return "read failed";
		case EMMARCH_INVALID_HEADER:
			return "invalid header";
		default:
			return "unknown error";
	}
}

void mmarch_fail(mmarch_error error)
{
	fprintf(stderr, "mmarch error[%d]: %s\n", (int)error, mmarch_get_error(error));
	exit(1);
}

void mmarch_context_init(struct mmarch_context * context)
{ memset(context, 0, sizeof(*context)); } //don't blame me, too many fields there lol

mmarch_error mmarch_context_load(struct mmarch_context * context, const uint8_t * buf)
{
	return EMMARCH_INVALID_HEADER;
}
