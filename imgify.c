/*
	imgify - convert any file to png and back.

	imgify.c

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

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

static uint8_t* pixel_at(uint8_t *data, uint32_t row, uint32_t col, uint32_t width, uint32_t height, uint8_t channels) {
	assert(data != NULL);
	assert(row >= 0 && row < height);
	assert(col >= 0 && col < width);
	const uint32_t offset = (row * width + col) * channels;
	//printf("DEBUG: [pixel_at] row %d -> offset %u\n", row+1, offset);
	return &data[offset];
}

#define PNG_SIG_SIZE 8

bool png_load(const char *filename, uint8_t **out_buffer, size_t *out_buffer_size, uint32_t *out_width, uint32_t *out_height, uint8_t *out_channels, uint32_t *out_padding, uint8_t pad_byte) {
	// Open the file.
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		perror("fopen");
		return false;
	}

	// Check for the signature.
	uint8_t sig[PNG_SIG_SIZE];
	fread(sig, 1, sizeof(sig), fp);
	if (!png_check_sig(sig, sizeof(sig))) {
		fclose(fp);
		perror("png_check_sig: failed");
		return false;
	}

	// Setup PNG structs.
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		perror("png_create_read_struct: out of memory");
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		fclose(fp);
		perror("png_create_info_struct: out of memory");
		return false;
	}

	// Handle libpng errors.
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		perror("setjmp: failed");
		return false;
	}

	// Inform our file stream pointer for later use.
	png_init_io(png_ptr, fp);

	// Let the libpng know that we already checked the signature bytes
	// so it doesn't expect to find it at the current file position.
	png_set_sig_bytes(png_ptr, PNG_SIG_SIZE);

	// Read PNG info up to image data.
	png_read_info(png_ptr, info_ptr);

	int color_type;
	uint32_t width;
	uint32_t height;
	int bit_depth;

	// Read header data.
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	// Create buffer to hold row pointers.
	png_bytepp row_pointers = malloc(height * sizeof(png_bytep));
	if (row_pointers == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		perror("malloc: out of memory");
		return false;
	}

	// Get the number of channels of info for the color type
	const uint8_t channels = png_get_channels(png_ptr, info_ptr);

	// Calculate how much memory it will take.
	const size_t data_size = width * height * bit_depth * channels / 8;

	// Allocate the data buffer.
	uint8_t *data = malloc(data_size);
	if (data == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		free(row_pointers);
		perror("malloc: out of memory");
		return false;
	}

	// The length of one row in bytes - the seme as `width * bit_depth * channels / 8`.
	const uint32_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// Set the row pointers.
	for (size_t i = 0; i < height; ++i) {
		row_pointers[i] = data + i * rowbytes;
	}

	// Read the entire image.
	png_read_image(png_ptr, row_pointers);

	// Clean up.
	free(row_pointers);
	row_pointers = NULL;
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

	// Calculate padding.
	const uint8_t *last_row = pixel_at(data, height-1, 0, width, height, channels);
	uint32_t padding = 0;
	for (uint32_t i = 0; i < rowbytes; ++i) {
		uint32_t column = rowbytes - i;
		// Stop when the padding byte is no longer found.
		if (last_row[column-1] != pad_byte) {
			padding = i;
			// printf("DEBUG: [png_load] Found %u pixels of padding (%u bytes), starting at row %u column %u\n",
			// 	padding / channels, padding, height, column);
			break;
		}
	}

	if (out_buffer != NULL)
		*out_buffer = data;
	if (out_buffer_size != NULL)
		*out_buffer_size = rowbytes * height - padding;
	if (out_width != NULL)
		*out_width = width;
	if (out_height != NULL)
		*out_height = height;
	if (out_channels != NULL)
		*out_channels = channels;
	if (out_padding != NULL)
		*out_padding = padding;

	return true;
}

bool png_save(const char *filename, uint8_t *data, uint32_t width, uint32_t height, uint8_t channels, uint32_t padding, uint8_t pad_byte) {
	assert(width > 0); // 1 column at least
	assert(height > 0); // 1 row at least

	const int bit_depth = 8;

	int color_type;
	switch (channels) {
		case 3: color_type = PNG_COLOR_TYPE_RGB; break;
		case 4: color_type = PNG_COLOR_TYPE_RGBA; break;
		default:
			fprintf(stderr, "Unknown image color type\n");
			return false;
	}

	// Open the file.
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		perror("fopen");
		return false;
	}

	// Setup PNG structs.
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, NULL, 0);
	if (png_ptr == NULL) {
		fclose(fp);
		perror("png_create_write_struct: out of memory");
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		perror("png_create_info_struct: out of memory");
		return false;
	}

	// Handle libpng errors.
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		perror("setjmp: failed");
		return false;
	}

	// Inform our file stream pointer for later use.
	png_init_io(png_ptr, fp);

	// Set the PNG header info.
	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Write the info.
	png_write_info(png_ptr, info_ptr);

	// The length of one row in bytes - the seme as `width * bit_depth * channels / 8`.
	const uint32_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	const bool needs_padding = padding > 0;
	for (uint32_t i=0; i < height; i++) {
		const bool is_lastrow = i == height - 1;
		const bool is_padding_now = needs_padding && is_lastrow;
		const uint8_t *row_ptr = pixel_at(data, i, 0, width, height, channels);

		//printf("DEBUG: [png_save] row=%u width=%u rowbytes=%u\n", i+1, width, rowbytes);

		if (!is_padding_now)
		{
			png_write_row(png_ptr, row_ptr);
		}
		else
		{
			uint8_t *padded_row_ptr = malloc(rowbytes);
			if (padded_row_ptr == NULL) {
				png_destroy_write_struct(&png_ptr, &info_ptr);
				fclose(fp);
				perror("malloc");
				return false;
			}
			memset(padded_row_ptr, pad_byte, rowbytes);
			memcpy(padded_row_ptr, row_ptr, rowbytes - padding);
			png_write_row(png_ptr, padded_row_ptr);
		}
	}

	// Signal we have finishing writing.
	png_write_end(png_ptr, NULL);

	// Cleanup.
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return true;
}
