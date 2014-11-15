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
    
  }
  return 0;
}
