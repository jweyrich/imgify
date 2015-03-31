#pragma once

#include <stdbool.h>
#include <stdint.h>

bool png_load(const char *filename, uint8_t **out_buffer, size_t *out_buffer_size, uint32_t *out_width, uint32_t *out_height, uint8_t *out_channels, uint32_t *out_padding, uint8_t pad_byte);
bool png_save(const char *filename, uint8_t *data, uint32_t width, uint32_t height, uint8_t channels, uint32_t padding, uint8_t pad_byte);
