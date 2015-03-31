#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
	options_t *options = parse_options(argc, argv);

	if (options->input == NULL || options->output == NULL) {
		usage();
		exit(EXIT_FAILURE);
	}

	int fd = open(options->input, O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	const size_t filesize = fsize(options->input);

	void *data = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	close(fd);

	do_work(options, data, filesize);

	int ret = munmap(data, filesize);
	if (ret == -1) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}

	free_options(options);

	return EXIT_SUCCESS;
}
