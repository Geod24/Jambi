#pragma once
#include <stdint.h>

extern const uint16_t MZ_MAGIC;
extern const uint32_t NT_MAGIC;

struct dos_header {
  uint16_t e_magic;
  uint16_t e_cblp;
  uint16_t e_cp;
  uint16_t e_crlc;
  uint16_t e_cparhdr;
  uint16_t e_minalloc;
  uint16_t e_maxalloc;
  uint16_t e_ss;
  uint16_t e_sp;
  uint16_t e_csum;
  uint16_t e_ip;
  uint16_t e_cs;
  uint16_t e_lfarlc;
  uint16_t e_ovno;
  uint16_t e_res[4];
  uint16_t e_oemid;
  uint16_t e_oeminfo;
  uint16_t e_res2[10];
  uint32_t e_lfanew;
};

typedef struct dos_header dos_header_t;

struct file_header {
  uint16_t Machine;
  uint16_t NumberOfSections;
  uint32_t TimeDateStamp;
  uint32_t PointerToSymbolTable;
  uint32_t NumberOfSymbols;
  uint16_t SizeOfOptionalHeader;
  uint16_t Characteristics;
};

struct data_directory {
  uint32_t VirtualAddress;
  uint32_t Size;
};


struct nt_header_32 {
  int32_t Signature;
  struct file_header FileHeader;
  //  optional_header_32 OptionalHeader;
  //  optional_header_64 OptionalHeader64;
  uint16_t OptionalMagic;
};
