#include <assert.h>
#include <stdio.h>
#include "read_file.h"
#include "parser.h"


parser_t *parse_from_file(const char *file_path)
{
  parser_t *parser = malloc(sizeof(parser_t));
  assert(parser);

  parser->buffer = readFileToFileBuffer(file_path);
  assert(parser->buffer);

  int ret = parse_header(parser);
  assert(ret == 0);

  return parser;
}

int parse_header(parser_t *parser)
{
  assert(parser);
  buffer_t *file = parser->buffer;

  uint16_t tmp = 0;
  uint32_t current_offset = 0;
  
  // check MZ_MAGIC
  if (read_word(file, current_offset, &tmp) != 0)
    {
      printf("Cannot read MZ_Magic\n");
      return -1;
    }
  if (tmp != MZ_MAGIC)
    {
      printf("MZ_MAGIC doesn't match\n");
      return -1;
    }
  return 0;
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
