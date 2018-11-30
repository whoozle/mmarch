#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

			if (name)
				printf("%d:\t%.*s\n", id, (int)name_len, name);
			else
				printf("%d:\t--no-name--\n", id);
			mmarch_readdir_iterator_next(&begin);
		}
	}
	if (extract)
	{
		fprintf(stderr, "extract %s\n", extract);
	}
	mmarch_context_posix_deinit(&context);
	exit(0);
}
