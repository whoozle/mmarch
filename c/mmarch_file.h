#ifndef _MMARCH_FILE_H
#define _MMARCH_FILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACKED(N) __attribute__((packed, aligned(N)))
#define STATIC_ASSERT(expr) { typedef char static_assert_failed[(expr) ? -1 : 1]; static_assert_failed _ops; (void)_ops; }

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

#ifdef __cplusplus
}
#endif

#endif
