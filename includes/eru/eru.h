#pragma once

#include <elf.h>

typedef struct {
	int phdr_index;
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

void beginPhdr(void);
Phdr *endPhdr(void);

void phdrSetType(Elf64_Word type);
void phdrSetFlags(Elf64_Word flags);
void phdrSetAlign(Elf64_Word align);
unsigned char *phdrSetData(unsigned char *data, Elf64_Xword data_size);

Offset phdrGetOffset(Elf64_Off addend);

void writeToMemory(Elf *elf, unsigned char **buffer, Elf64_Xword *buffer_size);
void writeToFile(Elf *elf, char *filename);
