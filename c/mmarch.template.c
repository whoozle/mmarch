static mmarch_error mmarch_context_load_impl(struct mmarch_context * context, const uint8_t * buf)
{
	struct mmarch_file_header *header = (struct mmarch_file_header *)buf;

	int version = L32(header->version);
	if (!COMPATIBLE(version))
		return EMMARCH_INCOMPATIBLE_VERSION;

	context->page_size = L32(header->page_size);

	uint32_t header_size = L32(header->header_size);
	context->header_size = header_size;

	uint32_t object_table_offset = L32(header->object_table_offset);
	uint32_t filename_table_offset = L32(header->filename_table_offset);
	uint32_t readdir_table_offset = L32(header->readdir_table_offset);

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
	struct mmarch_file_object_table *object_table = context->_object_table = (struct mmarch_file_object_table *)(context->header + object_table_offset);
	context->_filename_table = (struct mmarch_file_filename_table *)(context->header + filename_table_offset);
	context->_readdir_table = (uint32_t *)(context->header + readdir_table_offset);

	//fixme: more validation here
	uint32_t dir_count = L32(object_table->dir_count);
	uint32_t object_count = L32(object_table->total_count);

	if (dir_count > object_count ||
		(object_table_offset + 12 + object_count * L32(object_table->field_count)) > header_size)
	{
		return EMMARCH_INVALID_OFFSET_IN_HEADER;
	}
	context->_dir_count = dir_count;
	context->_object_count = object_count;
	context->_bucket_count = L32(context->_filename_table->bucket_count);

	context->hash_func = mmarch_get_hash_func(L32(context->_filename_table->hash_func_id));
	if (!context->hash_func)
		return EMMARCH_HASH_FUNCTION_UNSUPPORTED;

	return EMMARCH_OK;
}

mmarch_id mmarch_readdir_iterator_get(const struct mmarch_context * context, const struct mmarch_readdir_iterator * iter, const char ** name_ptr, size_t * name_length_ptr)
{
	const struct mmarch_file_readdir_table_entry * entry = (const struct mmarch_file_readdir_table_entry *)iter->_ptr;
	mmarch_id id = L32(entry->object_id);
	uint32_t name_offset = L32(entry->name_offset);
	uint32_t name_length = L32(entry->name_length);
	if (name_offset + name_length > context->header_size)
	{
		if (name_length_ptr)
			*name_length_ptr = 0;
		if (name_ptr)
			*name_ptr = NULL;
		return id;
	}

	if (name_length_ptr)
		*name_length_ptr = name_length;
	if (name_ptr)
		*name_ptr = (const char *)context->header + name_offset;
	return id;
}

void mmarch_context_readdir(const struct mmarch_context * context, const char *path, size_t len, struct mmarch_readdir_iterator * begin, struct mmarch_readdir_iterator * end)
{
	mmarch_id id = mmarch_context_find(context, path, len);
	if (id < 0)
		goto error;

	if (id >= context->_dir_count)
		goto error;

	if ((const uint8_t *)(context->_readdir_table + id + 1) >= context->header + context->header_size)
		goto error;

	uint32_t begin_off = L32(context->_readdir_table[id]);
	uint32_t end_off = L32(context->_readdir_table[id + 1]);

	begin->_ptr = context->header + begin_off;
	end->_ptr = context->header + end_off;

	return;
error:
	begin->_ptr = end->_ptr = NULL;
}

mmarch_id mmarch_context_find(const struct mmarch_context * context, const char *name, size_t len)
{
	uint32_t index = context->hash_func(name, len) % context->_bucket_count;

	if ((const uint8_t *)(context->_filename_table->bucket_offset + index + 1) >= context->header + context->header_size)
		return -1;

	uint32_t begin = L32(context->_filename_table->bucket_offset[index]);
	uint32_t end = L32(context->_filename_table->bucket_offset[index + 1]);

	const struct mmarch_file_filename_table_entry * begin_ptr = (const struct mmarch_file_filename_table_entry *)(context->header + begin);
	const struct mmarch_file_filename_table_entry * end_ptr = (const struct mmarch_file_filename_table_entry *)(context->header + end);
	for(; begin_ptr != end_ptr; ++begin_ptr)
	{
		mmarch_id id = L32(begin_ptr->object_id);
		if (mmarch_context_filename_cmp(context, id, name, len) == 0)
			return id;
	}
	return -1;
}

void mmarch_context_get_object_metadata(const struct mmarch_context * context, mmarch_id id, const char **name, size_t *name_length, off_t *offset, off_t *size)
{
	const struct mmarch_file_object_table_entry * entry = context->_object_table->entries + id;
	if  ((const uint8_t *)entry >= context->header + context->header_size)
		goto error;

	uint32_t name_offset = L32(entry->name_offset);
	uint32_t name_size = L32(entry->name_size);

	if (name_offset + name_size > context->header_size)
		goto error;

	if (name)
		*name = (const char *)context->header + name_offset;
	if (name_length)
		*name_length = name_size;
	if (offset)
		*offset = L64(entry->data_offset);
	if (size)
		*size = L64(entry->data_size);

	return;

error:
	if (name)
		*name = NULL;
	if (name_length)
		*name_length = 0;
	if (offset)
		*offset = 0;
	if (size)
		*size = 0;
}
