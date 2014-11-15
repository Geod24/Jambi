#include <string.h>
#include <unistd.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

/// D FTW
typedef struct s_String {
  char* ptr;
  size_t length;
} String;

// Decalarations
int runInfector(String* directory);

int main(int ac, char** args) {
  // I miss foreach :(
  String dir;
  for (int i = 1; i < ac; ++i) {
    dir.ptr = args[i];
    dir.length = strlen(args[i]);
    if (!runInfector(&dir))
      return 1;
  }
  if (ac < 2) {
    dir.ptr = _getcwd(NULL, 0);
    dir.length = strlen(dir.ptr);
    return runInfector(&dir);
  }
  return 0;
}

int runInfector(String* directory) {
  return 0;
}
