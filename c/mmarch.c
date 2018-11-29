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
	return sizeof(struct mmarch_file_header);
}

const char * mmarch_get_error(mmarch_error error)
{
	switch(error)
	{
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

#define L(STRUCT, FIELD, SWAP) ((context->native)? (STRUCT -> FIELD): SWAP ( STRUCT -> FIELD ))
#define L32(STRUCT, FIELD) L(STRUCT, FIELD, bswap_32)
#define L64(STRUCT, FIELD) L(STRUCT, FIELD, bswap_64)

mmarch_error mmarch_context_load(struct mmarch_context * context, const uint8_t * buf)
{
	struct mmarch_file_header *header = (struct mmarch_file_header *)buf;

	context->native = 1;
	if (L32(header, magic) != 0x4D415243)
	{
		context->native = 0;
		if (L32(header, magic) != 0x4D415243)
		{
			return EMMARCH_INVALID_HEADER_MAGIC;
		}
	}
	int version = L32(header, version);
	if (!COMPATIBLE(version))
		return EMMARCH_INCOMPATIBLE_VERSION;

	context->page_size = L32(header, page_size);

	uint32_t header_size = L32(header, header_size);
	context->header_size = header_size;

	uint32_t object_table_offset = L32(header, object_table_offset);
	uint32_t filename_table_offset = L32(header, filename_table_offset);
	uint32_t readdir_table_offset = L32(header, readdir_table_offset);

	if (object_table_offset < MMARCH_HEADER_SIZE || object_table_offset >= header_size || ((object_table_offset & 3) != 0) ||
		filename_table_offset < MMARCH_HEADER_SIZE || filename_table_offset >= header_size || ((filename_table_offset & 3) != 0) ||
		readdir_table_offset < MMARCH_HEADER_SIZE || readdir_table_offset >= header_size || ((readdir_table_offset & 3) != 0)
	)
	{
		return EMMARCH_INVALID_OFFSET_IN_HEADER;
	}

	void *mapped_header;
	mmarch_error err = context->map(context, &mapped_header, 0, header_size);
	if (err)
		return err;

	context->header = mapped_header;
	struct mmarch_file_object_table *object_table = context->object_table = (struct mmarch_file_object_table *)(context->header + object_table_offset);
	context->filename_table = (struct mmarch_file_filename_table *)(context->header + filename_table_offset);
	context->readdir_table = (struct mmarch_file_readdir_table *)(context->header + readdir_table_offset);

	//fixme: more validation here
	if (object_table->dir_count > object_table->total_count ||
		(object_table_offset + 12 + object_table->total_count * object_table->field_count) > header_size)
	{
		return EMMARCH_INVALID_OFFSET_IN_HEADER;
	}

	return EMMARCH_OK;
}

void mmarch_context_deinit(struct mmarch_context * context)
{
	if (context->header)
		context->unmap(context, context->header, context->header_size);
}
