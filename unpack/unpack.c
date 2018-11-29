#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mmarch_posix.h"

int main(int argc, char ** argv)
{
	int list = 0;
	int help = 0;
	struct option options[] =
	{
		{ "list", no_argument, 0, 'l' },
		{ "help", no_argument, 0, 'h' },
		{ "extract", required_argument, 0, 'e' },
	};

	char * extract = NULL;
	while(1)
	{
		int c = getopt_long(argc, argv, "lhe:", options, NULL);
		if (c == -1)
			break;

		switch(c)
		{
			case 'l':
				list = 1;
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

	if (list)
	{
		fprintf(stderr, "list\n");
	}
	if (extract)
	{
		fprintf(stderr, "extract %s\n", extract);
	}
	mmarch_context_posix_deinit(&context);
	exit(0);
}