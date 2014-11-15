#ifndef UTILS_H__
#define UTILS_H__

#include <stdio.h>
#include "pe-headers.h"
#include "read_file.h"

/// D FTW
typedef struct s_String {
  char* ptr;
  size_t length;
} String;

// utils.c
void getFileName(String* buff);
void getFilePath(String* buff);
HANDLE createTempFile(char* originalName);
uint32_t getExeSize(SectionHeader* first, SectionHeader* last);
PeHeader* getPEHeader(buffer_t* buff, size_t* minSizeNullable);
uint32_t getDataDir(buffer_t* buff, PeHeader* peH, DataDir** ret, size_t* minSize);

#endif /* UTILS_H__ */
