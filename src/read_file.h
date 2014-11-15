#pragma once

#include <stdint.h>
#include <windows.h>

struct buffer_s
{
  uint8_t *buf;
  uint32_t len;
  HANDLE file;
  HANDLE file_map;

  int is_copy;
};

typedef struct buffer_s buffer_t;

buffer_t *readFileToFileBuffer(const char *filePath);

void buffer_destroy(buffer_t **ptr);

buffer_t *split_buffer(buffer_t *b, uint32_t from, uint32_t to);
  
int read_byte(buffer_t *buffer, uint32_t offset, uint8_t *out);
int read_word(buffer_t *buffer, uint32_t offset, uint16_t *out);
int read_dword(buffer_t *buffer, uint32_t offset, uint32_t *out);
int read_qword(buffer_t *buffer, uint32_t offset, uint64_t *out);
