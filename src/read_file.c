#include <assert.h>
#include "read_file.h"
#include <stdio.h>

/**
 * Read a byte from buffer at offset and store it in out;
 */
int read_byte(buffer_t *buffer, uint32_t offset, uint8_t *out)
{
  assert(buffer);
  if (offset >= buffer->len)
    return -1;

  *out = *(buffer->buf + offset);
  return 0;
}

/**
 * Read a word (2 bytes) from buffer at offset and store them in out;
 */
int read_word(buffer_t *buffer, uint32_t offset, uint16_t *out)
{
  assert(buffer);
  if (offset >= buffer->len)
    return -1;

  *out = *(uint16_t *)(buffer->buf + offset);
  return 0;
}

/**
 * Read a word (4 bytes) from buffer at offset and store them in out;
 */
int read_dword(buffer_t *buffer, uint32_t offset, uint32_t *out)
{
  assert(buffer);
  if (offset >= buffer->len)
    return -1;

  *out = *(uint32_t *)(buffer->buf + offset);
  return 0;
}

/**
 * Read a word (8 bytes) from buffer at offset and store them in out;
 */
int read_qword(buffer_t *buffer, uint32_t offset, uint64_t *out)
{
  assert(buffer);
  if (offset >= buffer->len)
    return -1;

  *out = *(uint64_t *)(buffer->buf + offset);
  return 0;
}

buffer_t *readFileToFileBuffer(const char *filePath)
{
  assert(filePath);
  HANDLE h = CreateFileA(filePath,
			 GENERIC_READ,
			 0,
			 NULL,
			 OPEN_EXISTING,
			 FILE_ATTRIBUTE_NORMAL,
			 NULL);
  if(h == INVALID_HANDLE_VALUE)
    {
      printf("Cannot open file: %ld\n", GetLastError());
      return NULL;
    }
  
  DWORD fileSize = GetFileSize(h, NULL);
  if(fileSize == INVALID_FILE_SIZE)
    {
      printf("Invalid file size: %ld\n", GetLastError());
      CloseHandle(h);
      return NULL;
    }

  buffer_t *buf = malloc(sizeof(buffer_t));
  buf->is_copy = 0;
  memset(buf, 0, sizeof(buffer_t));
  if (!buf)
    {
      CloseHandle(h);
      return NULL;
    }
  buf->file = h;
  HANDLE hMap = CreateFileMapping(h, NULL, PAGE_READONLY, 0, 0, NULL);
  if(hMap == NULL)
    {
      printf("CreateFileMapping failed.\n");
      buffer_destroy(&buf);
      return NULL;
    }
  buf->file_map = hMap;
  LPVOID ptr = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
  if(ptr == NULL)
    {
      printf("MapViewOfFile failed.\n");
      buffer_destroy(&buf);
      return NULL;
    }
  buf->buf = (uint8_t *)ptr;
  buf->len = fileSize;
  return buf;
}

void buffer_destroy(buffer_t **ptr)
{
  assert(ptr);
  buffer_t *me = *ptr;
  
  if (!me)
    return;
  if (!me->is_copy)
    {
      CloseHandle(me->file);
      CloseHandle(me->file_map);
    }
  free(me);
  *ptr = NULL;
}

buffer_t *split_buffer(buffer_t *b, uint32_t from, uint32_t to)
{
  assert(b);
  assert(!(to < from || to > b->len));
	 
  buffer_t *newBuff = malloc(sizeof(buffer_t));
  assert(newBuff);
  
  newBuff->is_copy = 1;
  newBuff->buf = b->buf+from;
  newBuff->len = (to-from);
  return newBuff;
}
