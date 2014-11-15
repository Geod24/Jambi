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

  buffer_t *remaining = NULL;
  int ret = parse_header(parser, &remaining);
  
  assert(ret == 0);

  return parser;
}

int parse_header(parser_t *parser, buffer_t **remaining)
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

  // read nt header
  int ret = parse_nt_header(buffer_nt, &parser->pe_header.nt);
  assert(ret == 0);  

  /*
   * Need to determine if this is a PE32 or PE32+ binary and use the
   # correct size.
  */
  uint32_t rem_size;
  if (parser->pe_header.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC)
    {
      printf("sorry 32 bits\n");
      return -1;
    }
  else if (parser->pe_header.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
    // signature + file_header + optional_header_64
    rem_size = sizeof(uint32_t) + sizeof(file_header_t) + sizeof(optional_header_64_t);
  }
  else
    {
      printf("Magic error\n");
      return -1;
    }
  //update 'remaing' to point to the space after the header
  *remaining = split_buffer(buffer_nt, rem_size, buffer_nt->len);
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
  int ret = read_word(ohb, 0, &header->OptionalMagic);
  assert(ret == 0);
  if (header->OptionalMagic == NT_OPTIONAL_32_MAGIC)
    {
      printf("Sorry 32bits\n");
      return -1;
    }
  else if (header->OptionalMagic == NT_OPTIONAL_64_MAGIC)
    {
      printf("Ok, 64 bits\n");      
      if (read_optional_header_64(ohb, &header->OptionalHeader64) != 0)
	{
	    buffer_destroy(&fhb);
	    buffer_destroy(&ohb);
	    return -1;
	}
    }
  else
    {
      printf("Invalid NT MAGIC\n");
      return -1;
    }
  buffer_destroy(&fhb);
  buffer_destroy(&ohb);
  return 0;
}

int read_optional_header_64(buffer_t *b, optional_header_64_t *header)
{
  READ_WORD(b, 0, header, Magic);
  READ_BYTE(b, 0, header, MajorLinkerVersion);
  READ_BYTE(b, 0, header, MinorLinkerVersion);
  READ_DWORD(b, 0, header, SizeOfCode);
  READ_DWORD(b, 0, header, SizeOfInitializedData);
  READ_DWORD(b, 0, header, SizeOfUninitializedData);
  READ_DWORD(b, 0, header, AddressOfEntryPoint);
  READ_DWORD(b, 0, header, BaseOfCode);
  READ_QWORD(b, 0, header, ImageBase);
  READ_DWORD(b, 0, header, SectionAlignment);
  READ_DWORD(b, 0, header, FileAlignment);
  READ_WORD(b, 0, header, MajorOperatingSystemVersion);
  READ_WORD(b, 0, header, MinorOperatingSystemVersion);
  READ_WORD(b, 0, header, MajorImageVersion);
  READ_WORD(b, 0, header, MinorImageVersion);
  READ_WORD(b, 0, header, MajorSubsystemVersion);
  READ_WORD(b, 0, header, MinorSubsystemVersion);
  READ_DWORD(b, 0, header, Win32VersionValue);
  READ_DWORD(b, 0, header, SizeOfImage);
  READ_DWORD(b, 0, header, SizeOfHeaders);
  READ_DWORD(b, 0, header, CheckSum);
  READ_WORD(b, 0, header, Subsystem);
  READ_WORD(b, 0, header, DllCharacteristics);
  READ_QWORD(b, 0, header, SizeOfStackReserve);
  READ_QWORD(b, 0, header, SizeOfStackCommit);
  READ_QWORD(b, 0, header, SizeOfHeapReserve);
  READ_QWORD(b, 0, header, SizeOfHeapCommit);
  READ_DWORD(b, 0, header, LoaderFlags);
  READ_DWORD(b, 0, header, NumberOfRvaAndSizes);


  for (uint32_t i = 0; i < header->NumberOfRvaAndSizes; i++) {
    uint32_t c = (i*sizeof(data_directory_t));
    c += _offset(optional_header_64_t, DataDirectory[0]);
    uint32_t o;
    o = c + _offset(data_directory_t, VirtualAddress);
    if(read_dword(b, o, &header->DataDirectory[i].VirtualAddress) != 0) {
      return -1;
    }
    o = c + _offset(data_directory_t, Size);
    if(read_dword(b, o, &header->DataDirectory[i].Size) != 0) {
      return -1;
    }
  }

  return 0;
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
