#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
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

	int optind = 0;
	char * extract = NULL;
	while(1)
	{
		int c = getopt_long(argc, argv, "lhe:", options, &optind);
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
		}
	}

	if (help)
	{
		fprintf(stderr, "-h --help\t\t\thelp\n");
		fprintf(stderr, "-e --extract file\t\textracts specific file\n");
		fprintf(stderr, "-l --list\t\t\tlists file from the archive\n");
		exit(0);
	}

	struct mmarch_posix_context context;
	mmarch_context_posix_init(&context);
	if (list)
	{
		fprintf(stderr, "list\n");
	}
	if (extract)
	{
		fprintf(stderr, "extract %s\n", extract);
	}
	exit(0);
}