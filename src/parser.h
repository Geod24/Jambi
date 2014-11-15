#pragma once

#include "nt-headers.h"

struct pe_header_s {
  struct nt_header_32 nt;
};

typedef struct pe_header_s pe_header_t;

struct parser_s
{
  buffer_t *buffer;
  pe_header_t pe_header;
};

typedef struct parser_s parser_t;

parser_t *parse_from_file(const char *file_path);
int parse_header(parser_t *parser);
int parse_nt_header(buffer_t *ntbuf, nt_header_32_t *header);

void parser_destroy(parser_t **ptr);
