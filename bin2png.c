/*
	imgify - Convert any file to PNG and back.

	bin2png.c

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

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <math.h>

#include "imgify.h"
#include "common.h"
#include "common_options.h"
#include "version.h"

#define PROGRAM "bin2png"

static void do_work(const options_t *options, void *data, size_t filesize) {
	const uint8_t channels = 4;
	const uint32_t width = ceil(sqrt(filesize / channels));
	const uint32_t height = ceil(filesize / channels / width) + 1;
	const uint32_t padding = width * height * channels - filesize;

	printf("Input file => %s\n  size => %zu bytes\n", options->input, filesize);

	bool ok = png_save(options->output, data, width, height, channels, padding, options->pad_byte);
	if (ok) {
		printf("Output file => %s\n  size => %zu bytes\n  image => %ux%u px, %d bpp, %upx padding\n",
			options->output, fsize(options->output), width, height, channels * 8, padding / channels);
	}
}

#include "common_main.h"
