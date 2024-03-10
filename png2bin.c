/*
	imgify - Convert any file to PNG and back.

	png2bin.c

	Copyright (C) 2015 - Jardel Weyrich <jweyrich@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define PROGRAM "png2bin"

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <math.h>

#include "imgify.h"
#include "common.h"
#include "common_options.h"
#include "version.h"

static void do_work(const options_t *options, void *data, size_t filesize) {
	uint8_t *buffer;
	size_t buffer_size;
	uint32_t width;
	uint32_t height;
	uint8_t channels;
	uint32_t padding;

	const bool ok = png_load(options->input, &buffer, &buffer_size, &width, &height, &channels, &padding, options->pad_byte);

	if (ok) {
		printf("Input file => %s\n  size => %zu bytes\n  image => %ux%u px, %d bpp, %upx padding\n",
			options->input, filesize, width, height, channels * 8, padding / channels);

		FILE *outf = fopen(options->output, "wb");
		if (outf == NULL) {
			free(buffer);
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		size_t block_size = 8192;
		size_t remaining = buffer_size;
		size_t total_written = 0;

		while (remaining > 0) {
			if (remaining < block_size)
				block_size = remaining;
			size_t written = fwrite(buffer + total_written, 1, block_size, outf);
			if (written == 0)
				break;
			remaining -= written;
			total_written += written;
		}

		fclose(outf);
		free(buffer);

		printf("Output file => %s\n  size => %zu bytes\n", options->output, fsize(options->output));
	}
}

#include "common_main.h"
