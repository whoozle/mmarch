#ifndef _MMARCH_H
#define _MMARCH_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define MMARCH_HEADER_SIZE (54)

#ifdef __cplusplus
extern "C"
{
#endif
	typedef enum
	{
		EMMARCH_OK = 0,
		EMMARCH_READ_FAILED = -1,
		EMMARCH_INVALID_HEADER = -2,
	} mmarch_error;

	const char * mmarch_get_error(mmarch_error error); ///< gets human readable error message
	void mmarch_fail(mmarch_error error); ///<prints error and exit(1)

	size_t mmarch_get_header_size();

	struct mmarch_context
	{
		void *			user; ///< any particular value for user usage (owning c++ class)
		mmarch_error    (*map)  (struct mmarch_context *context, void **ptr, off_t offset, size_t size);
		mmarch_error    (*unmap)(struct mmarch_context *context, void * data, size_t size);

		uint32_t		page_size;

		void *			header;
		uint32_t		header_size;

		uint64_t		total_size;

		uint32_t		object_table_offset;
		uint32_t		filename_table_offset;
		uint32_t		readdir_table_offset;
	};

	void mmarch_context_init(struct mmarch_context * context);
	void mmarch_context_deinit(struct mmarch_context * context);

	mmarch_error mmarch_context_load(struct mmarch_context * context, const uint8_t * buf);

#ifdef __cplusplus
}
#endif


#endif
