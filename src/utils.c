#include <direct.h> // For _getcwd
#include <windows.h>
#include "utils.h"


SectionHeader* getLastExeSection(SectionHeader* first, SectionHeader* last) {
  SectionHeader* biggest = last;
  SectionHeader* iter = first;
  while (iter != last) {
    if (iter->PointerToRawData > biggest->PointerToRawData)
      biggest = iter;
    iter += 1;
  }
  return biggest;
}

uint32_t getExeSize(SectionHeader* first, SectionHeader* last) {
  SectionHeader* last_ = getLastExeSection(first, last);
  return (last_->PointerToRawData + last_->SizeOfRawData);
}

uint32_t getDataDir(buffer_t* buff, PeHeader* peH, DataDir** ret, size_t* minSize) {
  PeOptHeader32* optH32 = (PeOptHeader32*)(peH + 1);
  PeOptHeader64* optH64 = (PeOptHeader64*)(peH + 1);
  int is64Bits = (optH32->Magic == OPT_HEADER_MAGIC64);
  // Get the data directories
  uint32_t dataDirLen = is64Bits ? optH64->NumberOfRvaAndSizes : optH32->NumberOfRvaAndSizes;
  DataDir* dataDir = is64Bits ? (DataDir*)(optH64 + 1) : (DataDir*)(optH32 + 1);
  *minSize += sizeof(DataDir) * dataDirLen;
  if (buff->len < *minSize) {
    printf("\t[-] Corrupted file (too small for DataDirectory).\n");
    return 0;
  }
  *ret = dataDir;
  return dataDirLen;
}

PeHeader* getPEHeader(buffer_t* buff, size_t* minSize) {
  DosHeader* dosH = (DosHeader*)(buff->buf);
  size_t _minSize;
  if (minSize == NULL) minSize = &_minSize;
  *minSize = sizeof(DosHeader);

  // Note that it should start as a valid MS-DOS executable.
  if (buff->len < *minSize || dosH->e_magic != MSDOS_MAGIC) {
    printf("\t[-] The file passed is not valid MSDOS executable.\n");
    return NULL;
  }

  PeHeader* h = (PeHeader*)(buff->buf + dosH->e_lfanew);
  *minSize = dosH->e_lfanew + sizeof(PeHeader);
  if (buff->len < *minSize) {
    printf("\t[-] The file passed is too small (expected %I64X, got %X).\n",
	   *minSize, buff->len);
    return NULL;
  }

  // Sanity checks for the PE file.
  if (h->Magic != PE_MAGIC) {
    printf("\t[-] The file passed is not a valid PE file.\n");
    return NULL;
  }
  if (h->SizeOfOptionalHeader == 0) {
    printf("\t[-] Optional header is not present, unable to infect file.\n");
    return NULL;
  }
  *minSize += sizeof(PeOptHeader32);
  if (buff->len < *minSize) {
    printf("\t[-] Optional header should be present, but the file is too small.\n");
    return NULL;
  }

  PeOptHeader32* optH32 = (PeOptHeader32*)(h + 1);
  int is64Bits = (optH32->Magic == OPT_HEADER_MAGIC64);
  if (!is64Bits && optH32->Magic != OPT_HEADER_MAGIC32) {
    printf("\t[-] File is neither described as a 32 or 64 bits PE file ?\n");
    return NULL;
  }

  // Adjust size...
  if (is64Bits) {
    *minSize += (sizeof(PeOptHeader64) - sizeof(PeOptHeader32));
    if (buff->len < *minSize) {
      printf("\t[-] Corrupted file (too small for 64 bits opt header).\n");
      return NULL;
    }
  }
  
  return h;
}

/////////////////////////////////////////////////////////////////////////////////////
// Create a temp file to write the patched file to...                              //
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa363875(v=vs.85).aspx  //
/////////////////////////////////////////////////////////////////////////////////////
HANDLE createTempFile(char* originalName) {
  HANDLE ret = INVALID_HANDLE_VALUE;
  char tempPath[MAX_PATH];
  char tempFileName[MAX_PATH];
  int retVal = GetTempPath(MAX_PATH, tempPath);
  if (retVal > MAX_PATH || (retVal == 0)) {
    // If GetTempPath failed, we default to current working directory.
    if (NULL == _getcwd(tempPath, MAX_PATH)) {
      printf("[-] Call to GetTempPath failed.\n");
      return ret;
    }
  }

  // Generates a temporary file name.
  uint32_t uRetVal = GetTempFileName(tempPath, originalName, 0, tempFileName);
  if (uRetVal == 0) {
    printf("[-] Call to GetTempFileName failed.\n");
    return ret;
  }
  size_t length = strlen(tempFileName);
  tempFileName[length - 3] = 'e';
  tempFileName[length - 2] = 'x';
  tempFileName[length - 1] = 'e';

  // Creates the new file to write to for the upper-case version.
  ret = CreateFile((LPTSTR)tempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		   0/*FILE_FLAG_DELETE_ON_CLOSE*/, NULL);
  if (ret == INVALID_HANDLE_VALUE)
    printf("[-] Creating temporary file failed.\n");
  strcpy(originalName, tempFileName);
  return ret;
}

/// Must be pre-allocated
void getFilePath(String* buff) {
  //assert(buff != NULL);
  //assert(buff->ptr != NULL);
  HMODULE mod = GetModuleHandle(0);
  TCHAR strbuff[MAX_PATH];
  GetModuleFileName(mod, strbuff, MAX_PATH);
  CloseHandle(mod);
  buff->length = strlen(strbuff);
  strncpy(buff->ptr, strbuff, buff->length + 1);
}

void getFileName(String* buff) {
  String b;
  int it;
  char str_buffer[MAX_PATH];
  b.ptr = &(str_buffer[0]);
  getFilePath(&b);
  //assert(b.length > 0);
  for (it = b.length; it >= 0; --it) {
    if (b.ptr[it] == '\\') {
      ++it;
      break;
    }
  }
  // "a\\b\0" => it == 1, b.length == 3, length must be 1.
  buff->length = b.length - it;
  strncpy(buff->ptr, b.ptr + it, buff->length);
}
