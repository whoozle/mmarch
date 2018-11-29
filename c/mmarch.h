#ifndef _MMARCH_H
#define _MMARCH_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define MMARCH_HEADER_SIZE (36)

#ifdef __cplusplus
extern "C"
{
#endif

	typedef int mmarch_id;
	typedef uint32_t (*hash_func) (const char *str, size_t len);

	hash_func mmarch_get_hash_func(uint32_t type);

	typedef enum
	{
		EMMARCH_OK = 0,
		EMMARCH_READ_FAILED = -1,
		EMMARCH_INVALID_HEADER_MAGIC = -2,
		EMMARCH_INCOMPATIBLE_VERSION = -3,
		EMMARCH_INVALID_OFFSET_IN_HEADER = -4,
		EMMARCH_PLATFORM_MAP_FAILED = -5,
		EMMARCH_PLATFORM_UNMAP_FAILED = -6,
		EMMARCH_HASH_FUNCTION_UNSUPPORTED = -7,
	} mmarch_error;

	const char * mmarch_get_error(mmarch_error error); /*< gets human readable error message */
	void mmarch_fail(mmarch_error error); /*< prints error and exit(1) */

	size_t mmarch_get_header_size();

	struct mmarch_file_object_table;
	struct mmarch_file_filename_table;
	struct mmarch_file_readdir_table;

	struct mmarch_context
	{
		void *			user; /*< any particular value for user usage (owning c++ class) */
		mmarch_error    (*map)  (struct mmarch_context *context, void **ptr, off_t offset, size_t size);
		mmarch_error    (*unmap)(struct mmarch_context *context, void * data, size_t size);

		int				native; /*< native endianess */
		uint32_t		page_size; /*< page size used for archive creation */

		uint8_t *		header;
		uint32_t		header_size;

		uint64_t		total_size;

		hash_func		hash_func;

		struct mmarch_file_object_table *		_object_table;
		struct mmarch_file_filename_table *		_filename_table;
		struct mmarch_file_readdir_table *		_readdir_table;
	};

	struct mmarch_readdir_iterator
	{
		uint32_t * _ptr;
	};

	void mmarch_context_init(struct mmarch_context * context);
	void mmarch_context_deinit(struct mmarch_context * context);

	mmarch_error mmarch_context_load(struct mmarch_context * context, const uint8_t * buf);
	mmarch_id mmarch_context_find(struct mmarch_context * context, const char *path, size_t len);

	static inline int mmarch_readdir_iterator_equals(const struct mmarch_readdir_iterator * a, const struct mmarch_readdir_iterator * b)
	{ return a->_ptr == b->_ptr; }
	static inline void mmarch_readdir_iterator_next(struct mmarch_readdir_iterator * iter)
	{ ++ iter->_ptr; }
	static inline mmarch_id mmarch_readdir_iterator_get(const struct mmarch_readdir_iterator * iter)
	{ return *iter->_ptr; }

	void mmarch_readdir(struct mmarch_context * context, const char *path, size_t len, struct mmarch_readdir_iterator * begin, struct mmarch_readdir_iterator * end);

#ifdef __cplusplus
}
#endif


#endif
