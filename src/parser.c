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

  // offset of NT header
  uint32_t offset;

  if (read_dword(file, _offset(dos_header_t, e_lfanew), &offset) != 0)
    {
      printf("Cannot read offset of NT header\n");
      return -1;
    }

  // access nc buffer
  current_offset += offset;
  buffer_t *buffer_nt = split_buffer(file, current_offset, file->len);
  int ret = parse_nt_header(buffer_nt, &parser->pe_header.nt);
  assert(ret == 0);  
  
  
  buffer_destroy(&buffer_nt);
  return 0;
}

int parse_nt_header(buffer_t *ntbuf, nt_header_32_t *header)
{
  assert(ntbuf);
  assert(header);

  uint32_t pe_magic;
  uint32_t curOffset =0;

  if(read_dword(ntbuf, curOffset, &pe_magic) != 0 || pe_magic != NT_MAGIC)
    {
      printf("PE_MAGIC failed\n");
      return -1;
    }
  header->Signature = pe_magic;
  
  buffer_t *fhb = split_buffer(ntbuf,
			       _offset(nt_header_32_t, FileHeader), ntbuf->len);
  assert(fhb);
  
  if(parse_file_header(fhb, &header->FileHeader) != 0)
    {
      buffer_destroy(&fhb);
      return -1;
    }
  /*
   * The buffer is split using the OptionalHeader offset, even if it turns
   * out to be a PE32+. The start of the buffer is at the same spot in the
   * buffer regardless.
   */
  buffer_t *ohb =
    split_buffer(ntbuf, _offset(nt_header_32_t, OptionalHeader), ntbuf->len);
  assert(ohb);
  
  /*
   * Read the Magic to determine if it is 32 or 64.
   */
  READ_WORD(ohb, 0, header, OptionalMagic);
  if (header->OptionalMagic == NT_OPTIONAL_32_MAGIC)
    {
      printf("Sorry 32bits\n");
      return -1;
      //    if(readOptionalHeader(ohb, header->OptionalHeader) == false) {
      //      deleteBuffer(ohb);
      //      deleteBuffer(fhb);
      //      return false;
      //    }
    }
  else if (header->OptionalMagic == NT_OPTIONAL_64_MAGIC)
    {
      //    if(readOptionalHeader64(ohb, header->OptionalHeader64) == false) {
      //      deleteBuffer(ohb);
      //      deleteBuffer(fhb);
      //      return false;
      printf("Ok, 64 bits\n");
    }
  else
    {
      printf("Invalid NT MAGIC\n");
      return -1;
    }
  destroy_buffer(&fhb);
  destroy_buffer(&ohb);
}

int parse_file_header(buffer_t *b, file_header_t *header)
{
  READ_WORD(b, 0, header, Machine);
  READ_WORD(b, 0, header, NumberOfSections);
  READ_DWORD(b, 0, header, TimeDateStamp);
  READ_DWORD(b, 0, header, PointerToSymbolTable);
  READ_DWORD(b, 0, header, NumberOfSymbols);
  READ_WORD(b, 0, header, SizeOfOptionalHeader);
  READ_WORD(b, 0, header, Characteristics);

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
