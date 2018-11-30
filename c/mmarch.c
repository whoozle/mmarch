#include "mmarch.h"
#include "mmarch_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include <endian.h>


size_t mmarch_get_header_size()
{
	STATIC_ASSERT(sizeof(struct mmarch_file_header) == MMARCH_HEADER_SIZE);
	STATIC_ASSERT(sizeof(struct mmarch_file_readdir_table_entry) == MMARCH_READDIR_ENTRY_SIZE);
	return sizeof(struct mmarch_file_header);
}

static uint32_t r5a(const char *str, size_t len)
{
	uint32_t value = 0;
	while(len--)
	{
		char c = *str++;
        value += (c << 4) + (c >> 4);
        value = (value + (value << 1) + (value << 3)) ^ (value >> 29); //peasant multiplication by 11 and xor with upper bit of previous value
	}
	return value;
}

hash_func mmarch_get_hash_func(uint32_t type)
{
	switch(type)
	{
		case 3: //R5A
			return &r5a;

		default:
			return NULL;
	}
}


const char * mmarch_get_error(mmarch_error error)
{
	switch(error)
	{
		case EMMARCH_OK:
			return "success";
		case EMMARCH_READ_FAILED:
			return "read failed";
		case EMMARCH_INVALID_HEADER_MAGIC:
			return "invalid header magic";
		case EMMARCH_INCOMPATIBLE_VERSION:
			return "incompatible archive version";
		case EMMARCH_INVALID_OFFSET_IN_HEADER:
			return "invalid offset in header";
		case EMMARCH_PLATFORM_MAP_FAILED:
			return "platform map returned error";
		case EMMARCH_PLATFORM_UNMAP_FAILED:
			return "platform unmap returned error";
		case EMMARCH_HASH_FUNCTION_UNSUPPORTED:
			return "hash function unsupported";
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
{ memset(context, 0, sizeof(*context)); } /*don't blame me, too many fields there lol*/


void mmarch_context_deinit(struct mmarch_context * context)
{
	if (context->header)
		context->unmap(context, context->header, context->header_size);
}

static int mmarch_context_filename_cmp(const struct mmarch_context * context, mmarch_id id, const char *name, size_t len)
{
	const char *stored_name;
	size_t stored_name_length;
	mmarch_context_get_object_name(context, id, &stored_name, &stored_name_length);
	int d = (ssize_t)stored_name_length - (ssize_t)len;
	if (d)
		return d;

	return strncmp(stored_name, name, len);
}

mmarch_error mmarch_context_map_object(struct mmarch_context * context, struct mmarch_mapping * mapping, mmarch_id id)
{
	off_t offset, size;
	mmarch_context_get_object_placement(context, id, &offset, &size);
	if (size != 0)
	{
		void *ptr;
		mmarch_error r = context->map(context, &ptr, offset, size);
		if (r)
			return r;

		mapping->data = ptr;
		mapping->size = size;
	}
	else
	{
		mapping->data = NULL;
		mapping->size = 0;
	}
	return EMMARCH_OK;
}

mmarch_error mmarch_context_unmap_object(struct mmarch_context * context, struct mmarch_mapping * mapping)
{
	if (mapping->data && mapping->size)
		return context->unmap(context, mapping->data, mapping->size);
	else
		return EMMARCH_OK;
}

#define L(VALUE, SWAP) ((context->native)? (VALUE): SWAP (VALUE))
#define L32(VALUE) L(VALUE, bswap_32)
#define L64(VALUE) L(VALUE, bswap_64)

#include "mmarch.template.c"

mmarch_error mmarch_context_load(struct mmarch_context * context, const uint8_t * buf)
{
	struct mmarch_file_header *header = (struct mmarch_file_header *)buf;

	context->native = 1;
	if (L32(header->magic) != 0x4D415243)
	{
		context->native = 0;
		if (L32(header->magic) != 0x4D415243)
		{
			return EMMARCH_INVALID_HEADER_MAGIC;
		}
	}
	return mmarch_context_load_impl(context, buf);
}
