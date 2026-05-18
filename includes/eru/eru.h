#pragma once

#include <elf.h>

enum OffsetType {
	OFFTYPE_ABSOLUTE,
	OFFTYPE_PHDR,
	OFFTYPE_SHDR
};

typedef struct {
	enum OffsetType type;
	Elf64_Half index;
	Elf64_Off addend;
} Offset;

typedef struct {
	Elf64_Phdr phdr;
	unsigned char *data;
	Elf64_Xword data_size;
	Elf64_Off data_offset;
	Offset offset;
	Elf64_Xword size;
} Phdr;

typedef struct {
	Elf64_Shdr shdr;
	unsigned char *data;
	Elf64_Xword data_size;
	Elf64_Off data_offset;
	Offset offset;
	Elf64_Xword size;
} Shdr;

typedef struct {
	Elf64_Ehdr ehdr;
	Elf64_Addr base_addr;
	Offset entry;
	Phdr *phdr_list;
	Shdr *shdr_list;
} Elf;

Offset getAbsOffset(Elf64_Off addend);

void beginElf(void);
Elf *endElf(void);
void freeElf(Elf *elf);

void elfSetEntry(Offset entry);
void elfSetBaseAddr(Elf64_Addr base_addr);
void elfSetType(Elf64_Half type);
void elfSetMachine(Elf64_Half machine);
void elfSetData(unsigned char data);
void elfSetShstrndx(Elf64_Half strndx);


void beginPhdr(void);
Phdr *endPhdr(void);

void phdrSetType(Elf64_Word type);
void phdrSetFlags(Elf64_Word flags);
void phdrSetAlign(Elf64_Word align);
unsigned char *phdrSetData(unsigned char *data, Elf64_Xword data_size);

Offset phdrGetOffset(Elf64_Off addend);
Elf64_Half phdrGetIndex(void);


void beginShdr(void);
Shdr *endShdr(void);

Elf64_Half shdrGetIndex(void);
Offset shdrGetOffset(Elf64_Off addend);

void beginStrtab(void);
void endStrtab(void);
Elf64_Off strtabAddString(char *string);


void writeToMemory(Elf *elf, unsigned char **buffer, Elf64_Xword *buffer_size);
void writeToFile(Elf *elf, char *filename);
