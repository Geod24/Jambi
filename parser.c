#include <assert.h>
#include "read_file.h"
#include "parser.h"


parser_t *parse_from_file(const char *file_path)
{
  parser_t *parser = malloc(sizeof(parser_t));
  assert(parser);

  parser->buffer = readFileToFileBuffer(file_path);
  assert(parser->buffer);
}

void parser_destroy(parser_t **ptr)
{
  assert(ptr);
  parser_t *me = *ptr;

  if (!me)
    return;
  buffer_destroy(&me->buffer);
  *ptr = NULL;
}
