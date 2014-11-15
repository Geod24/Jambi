#pragma once

#include <stdint.h>
#include <windows.h>

struct buffer_s
{
  uint8_t *buf;
  uint32_t len;
  HANDLE file;
  HANDLE file_map;
};

typedef struct buffer_s buffer_t;

buffer_t *readFileToFileBuffer(const char *filePath);
void buffer_destroy(buffer_t **ptr);
