#pragma once

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
typedef struct {
	char *input;
	char *output;
	uint8_t pad_byte;
} options_t;
extern char* PROGRAM;
static void usage() {
	fprintf(stderr, "usage: %s -i <in_filename> -o <out_filename> [-p <pad_byte>]\n", PROGRAM);
}

static void version() {
	printf("%s %s\n\n%s\n", PROGRAM, VERSION, LICENSE_TEXT);
}

static options_t *parse_options(int argc, char *argv[])
{
	options_t *options = malloc(sizeof(options_t));
	if (options == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	memset(options, 0, sizeof(options_t));

	options->pad_byte = 0xFF; // Set the default padding byte to 0xFF.

	/* Parameters for getopt_long() function */
	static const char short_options[] = "i:o:p:v";

	static const struct option long_options[] = {
		{ "input",			required_argument,	NULL,	'i' },
		{ "output",			required_argument,	NULL,	'o' },
		{ "pad_byte",		required_argument,	NULL,	'p' },
		{ "help",			no_argument,		NULL,	 1  },
		{ "version",		no_argument,		NULL,	'v' },
		{ NULL,				0,					NULL, 	 0  }
	};

	int c, ind;

	while ((c = getopt_long(argc, argv, short_options, long_options, &ind)))
	{
		if (c < 0)
			break;

		switch (c)
		{
			case 'i':
				options->input = strdup(optarg);
				break;
			case 'o':
				options->output = strdup(optarg);
				break;
			case 'p':
			{
				unsigned long value = strtoul(optarg, NULL, 0);
				if (value == ULONG_MAX && errno == ERANGE) {
					fprintf(stderr, "invalid pad_byte value (non-negated value would overflow)\n");
					exit(EXIT_FAILURE);
				}
				if (value > 0xFF) {
					fprintf(stderr, "invalid pad_byte value (inform a value between 0 and 255, inclusive)\n");
					exit(EXIT_FAILURE);
				}
				options->pad_byte = value & 0xFF;
				break;
			}
			case 1: // For --help
				usage();
				exit(EXIT_SUCCESS);
			case 'v':
				version();
				exit(EXIT_SUCCESS);
			default:
				fprintf(stderr, "%s: try '--help' for more information\n", PROGRAM);
				exit(EXIT_FAILURE);
		}
	}

	return options;
}

static void free_options(options_t *options)
{
	if (options == NULL)
		return;

	if (options->input != NULL)
		free(options->input);
	if (options->output != NULL)
		free(options->output);

	free(options);
}
