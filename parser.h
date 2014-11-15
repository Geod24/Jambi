#pragma once

struct parser_s
{
  buffer_t *buffer;
};

typedef struct parser_s parser_t;

parser_t *parse_from_file(const char *file_path);

void parser_destroy(parser_t **ptr);
