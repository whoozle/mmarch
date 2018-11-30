#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "mmarch_posix.h"

int main(int argc, char ** argv)
{
	struct option options[] =
	{
		{ "list", optional_argument, 0, 'l' },
		{ "help", no_argument, 0, 'h' },
		{ "extract", required_argument, 0, 'e' },
	};
	const char *list_dir = NULL;
	int help = 0;

	char * extract = NULL;
	while(1)
	{
		int c = getopt_long(argc, argv, "l::he:", options, NULL);
		if (c == -1)
			break;

		switch(c)
		{
			case 'l':
				list_dir = optarg? optarg: "";
				break;
			case 'h':
				help = 1;
				break;
			case 'e':
				extract = optarg;
				break;
			default:
				exit(1);
				break;
		}
	}

	if (help)
	{
		fprintf(stderr, "-h --help\t\t\thelp\n");
		fprintf(stderr, "-e --extract file\t\textracts specific file\n");
		fprintf(stderr, "-l --list\t\t\tlists file from the archive\n");
		exit(0);
	}

	if (optind + 1 != argc)
	{
		fprintf(stderr, "expected single archive name after options\n");
		exit(1);
	}

	int fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		exit(1);
	}

	struct mmarch_context_posix context;
	mmarch_context_posix_init(&context);
	context.fd = fd;
	mmarch_error err;

	err = mmarch_context_posix_load(&context);
	if (err)
		mmarch_fail(err);

	if (list_dir)
	{
		fprintf(stderr, "readdir(\"%s\"):\n", list_dir);
		struct mmarch_readdir_iterator begin, end;
		mmarch_context_readdir(&context.base.context, list_dir, strlen(list_dir), &begin, &end);
		while(!mmarch_readdir_iterator_equals(&begin, &end))
		{
			const char *name;
			size_t name_len;

			mmarch_id id = mmarch_readdir_iterator_get(&context.base.context, &begin, &name, &name_len);
			char type = mmarch_context_is_directory(&context.base.context, id)? 'd': 'f';

			if (name)
				printf("%c:%-6d %.*s\n", type, id, (int)name_len, name);
			else
				printf("%c:%6d --no-name--\n", type, id);
			mmarch_readdir_iterator_next(&begin);
		}
	}
	if (extract)
	{
		fprintf(stderr, "extracting %s...\n", extract);
		mmarch_id id = mmarch_context_find(&context.base.context, extract, strlen(extract));
		fprintf(stderr, "object id = %d\n", id);
		if (id >= 0)
		{
			struct mmarch_mapping mapping;
			mmarch_error r = mmarch_context_map_object(&context.base.context, &mapping, id);
			if (r)
				mmarch_fail(r);

			const char * out = strrchr(extract, '/');
			int fd = open(out? out + 1: extract, O_WRONLY | O_CREAT, 0600);
			if (fd == -1)
			{
				perror("open");
				exit(1);
			}

			const uint8_t * src = (const uint8_t *)mapping.data;
			const uint8_t * end = (const uint8_t *)mapping.data + mapping.size;

			static const size_t write_size = 1024 * 1024;

			while(src < end)
			{
				size_t n = end - src;
				if (n > write_size)
					n = write_size;
				ssize_t r = write(fd, src, n);
				src += r;
				if (r != n)
				{
					if (r < 0)
						perror("write");
					break;
				}
			}

			close(fd);
			mmarch_context_unmap_object(&context.base.context, &mapping);
		}
	}
	mmarch_context_posix_deinit(&context);
	exit(0);
}
