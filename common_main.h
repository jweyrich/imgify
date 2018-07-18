#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
	options_t *options = parse_options(argc, argv);
	int fd = -1;

	if (options->input == NULL || options->output == NULL) {
		usage();
		goto failed;
	}
	
	fd = open(options->input, O_RDONLY);
	if (fd == -1) {
		perror("open");
		goto failed;
	}

	const size_t filesize = fsize(options->input);

	void *data = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("mmap");
		goto failed;
	}

	close(fd);
	fd = -1;

	do_work(options, data, filesize);

	int ret = munmap(data, filesize);
	if (ret == -1) {
		perror("munmap");
		goto failed;
	}

	free_options(options);

	return EXIT_SUCCESS;
failed:
	if (options != NULL)
		free_options(options);
	if (fd != -1)
		close(fd);
	exit(EXIT_FAILURE);
}
