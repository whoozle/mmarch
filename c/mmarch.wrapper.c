#define FUNC(NAME) (context->native? NAME##_native: NAME##_foreign)

static mmarch_error mmarch_context_load_impl(struct mmarch_context * context, const uint8_t * buf)
{ return FUNC(mmarch_context_load_impl)(context, buf); }

mmarch_id mmarch_readdir_iterator_get(const struct mmarch_context * context, const struct mmarch_readdir_iterator * iter, const char ** name_ptr, size_t * name_length_ptr)
{ return FUNC(mmarch_readdir_iterator_get)(context, iter, name_ptr, name_length_ptr); }

void mmarch_context_readdir(const struct mmarch_context * context, const char *path, size_t len, struct mmarch_readdir_iterator * begin, struct mmarch_readdir_iterator * end)
{ FUNC(mmarch_context_readdir)(context, path, len, begin, end); }

mmarch_id mmarch_context_find(const struct mmarch_context * context, const char *name, size_t len)
{ return FUNC(mmarch_context_find)(context, name, len); }

void mmarch_context_get_object_metadata(const struct mmarch_context * context, mmarch_id id, const char **name, size_t *name_length, off_t *offset, off_t *size)
{ FUNC(mmarch_context_get_object_metadata)(context, id, name, name_length, offset, size); }

#undef FUNC
