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

#define READ_BYTE(b, o, inst, member) \
  if (read_byte(b, o+_offset(__typeof__(*inst), member), &inst->member) != 0) { \
  printf("Error reading byte\n"); \
  return -1; \
  }
#define READ_WORD(b, o, inst, member) \
  if (read_word(b, o+_offset(__typeof__(*inst), member), &inst->member) != 0) { \
  printf("Error reading word\n"); \
  return -1; \
  }
#define READ_DWORD(b, o, inst, member) \
  if (read_dword(b, o+_offset(__typeof__(*inst), member), &inst->member) != 0) { \
  printf("Error reading dword\n"); \
  return -1; \
  }
#define READ_QWORD(b, o, inst, member) \
  if (read_qword(b, o+_offset(__typeof__(*inst), member), &inst->member) != 0) { \
  printf("Error reading qword\n"); \
  return -1; \
  }

typedef struct parser_s parser_t;

parser_t *parse_from_file(const char *file_path);
int parse_header(parser_t *parser);
int parse_nt_header(buffer_t *ntbuf, nt_header_32_t *header);
int parse_file_header(buffer_t *b, file_header_t *header);

void parser_destroy(parser_t **ptr);
