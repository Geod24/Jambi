#include <assert.h>
#include "nt-headers.h"
#include "read_file.h"
#include "parser.h"

int main(int ac, char **av)
{
  parser_t *parser;

  assert(ac > 1);
  parser = parse_from_file(av[1]);
  assert(parser);

  return 0;
}
