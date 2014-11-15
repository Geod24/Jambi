#include "read_file.h"

buffer_t *readFileToFileBuffer(const char *filePath)
{
  HANDLE h = CreateFileA(filePath,
			 GENERIC_READ,
			 0,
			 NULL,
			 OPEN_EXISTING,
			 FILE_ATTRIBUTE_NORMAL,
			 NULL);
  if(h == INVALID_HANDLE_VALUE)
    {
      return NULL;
    }
  
  DWORD fileSize = GetFileSize(h, NULL);
  if(fileSize == INVALID_FILE_SIZE)
    {
      CloseHandle(h);
      return NULL;
    }

  buffer_t *buf = malloc(sizeof(buffer_t));
  bzero(buf, sizeof(buffer_t));
  if (!buf)
    {
      CloseHandle(h);
      return NULL;
    }
  buf->file = h;
  HANDLE hMap = CreateFileMapping(h, NULL, PAGE_READONLY, 0, 0, NULL);
  if(hMap == NULL)
    {
      buffer_destroy(&buf);
      CloseHandle(h);
      return NULL;
    }
  buf->file_map = hMap;
  LPVOID ptr = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
  if(ptr == NULL)
    {
      buffer_destroy(&buf);
      return NULL;
    }
  buf->buf = (uint8_t *)ptr;
  p->bufLen = fileSize;
  return buf;
}

void buffer_destroy(buffer_t **ptr)
{
  assert(ptr);
  buffer_t *me = *ptr;
  
  if (!me)
    return;
  CloseHandle(me->file);
  CloseHandle(me->file_map);
  free(me);
  *ptr = NULL;
}
