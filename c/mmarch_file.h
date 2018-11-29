#ifndef _MMARCH_FILE_H
#define _MMARCH_FILE_H

/*
	PRIVATE HEADER DO NOT USE
*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACKED(N) __attribute__((packed, aligned(N)))
#define STATIC_ASSERT(expr) { typedef char static_assert_failed[(expr) ? 1 : -1]; static_assert_failed _ops; (void)_ops; }

#define MAGIC (0x4D415243u)
#define COMPATIBLE(version) (version == 1)

struct PACKED(4) mmarch_file_header
{
	uint32_t magic;
	uint32_t version;
	uint32_t page_size;
	uint32_t header_size;
	uint64_t total_size;

	uint32_t object_table_offset;
	uint32_t filename_table_offset;
	uint32_t readdir_table_offset;
};

struct PACKED(4) mmarch_file_object_table_entry
{
	uint64_t data_offset;
	uint64_t data_size;
	uint32_t name_offset;
	uint32_t name_size;
};

struct PACKED(4) mmarch_file_object_table
{
	uint32_t field_count;
	uint32_t total_count;
	uint32_t dir_count;
	struct mmarch_file_object_table_entry entries[];
};

struct PACKED(4) mmarch_file_filename_table_entry
{
	uint32_t object_id;
};

struct PACKED(4) mmarch_file_filename_table
{
	uint32_t hash_func_id;
	uint32_t bucket_count;
	uint32_t bucket_offset[];
};

struct PACKED(4) mmarch_file_readdir_table
{
	uint32_t record_count;
	uint32_t bucket_offset[];
};

#ifdef __cplusplus
}
#endif

#endif
