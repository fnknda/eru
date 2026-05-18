#include <eru/eru.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <unistd.h>

#define OFFSET_ABS -1

Elf64_Off offset;
static Elf *elf = NULL;
static Phdr *phdr = NULL;
static Shdr *shdr = NULL;

Offset getAbsOffset(Elf64_Off addend)
{
	Offset offset;
	offset.index = OFFTYPE_ABSOLUTE;
	offset.addend = addend;

	return offset;
}

void beginElf(void)
{
	if (elf != NULL)
		freeElf(endElf());

	elf = calloc(1, sizeof(Elf));

	elf->ehdr.e_ident[EI_MAG0] = ELFMAG0;
	elf->ehdr.e_ident[EI_MAG1] = ELFMAG1;
	elf->ehdr.e_ident[EI_MAG2] = ELFMAG2;
	elf->ehdr.e_ident[EI_MAG3] = ELFMAG3;
	elf->ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	elf->ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	elf->ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	elf->ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;
	elf->ehdr.e_ident[EI_ABIVERSION] = EV_CURRENT;
	elf->ehdr.e_type = ET_EXEC;
	elf->ehdr.e_machine = EM_X86_64;
	elf->ehdr.e_version = EV_CURRENT;
	elf->ehdr.e_flags = 0;
	elf->ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	elf->ehdr.e_phentsize = sizeof(Elf64_Phdr);
	elf->ehdr.e_phnum = 0;
	elf->ehdr.e_shentsize = sizeof(Elf64_Shdr);
	elf->ehdr.e_shnum = 0;
	elf->ehdr.e_shstrndx = -1;

	elf->entry.type = OFFTYPE_ABSOLUTE;
	elf->entry.index = 0;
	elf->entry.addend = 0;
	elf->phdr_list = NULL;
	elf->shdr_list = NULL;
}

Elf *endElf(void)
{
	if (phdr != NULL || shdr != NULL) {
		fprintf(stderr, "[ERU] Error: Could not end Elf, Phdr/Shdr still open\n");
		return NULL;
	}

	Elf *aux = elf;
	elf = NULL;
	return aux;
}

void freeElf(Elf *ielf)
{
	if (elf == NULL)
		return;

	free(ielf->phdr_list);
	free(ielf->shdr_list);
	ielf->phdr_list = NULL;
	ielf->shdr_list = NULL;
}

void elfSetEntry(Offset entry)
{
	elf->entry = entry;
}

void elfSetBaseAddr(Elf64_Addr base_addr)
{
	elf->base_addr = base_addr;
}

void elfSetType(Elf64_Half type)
{
	elf->ehdr.e_type = type;
}

void elfSetMachine(Elf64_Half machine)
{
	elf->ehdr.e_machine = machine;
}

void elfSetData(unsigned char data)
{
	elf->ehdr.e_ident[EI_DATA] = data;
}

void elfSetShstrndx(Elf64_Half strndx)
{
	elf->ehdr.e_shstrndx = strndx;
}

void beginPhdr(void)
{
	if (phdr != NULL)
		endPhdr();

	phdr = calloc(1, sizeof(Phdr));
	phdr->offset = phdrGetOffset(0);
}

Phdr *endPhdr(void)
{
	if (phdr == NULL)
		return NULL;

	size_t new_size = (elf->ehdr.e_phnum + 1) * sizeof(Phdr);

	Phdr *new = realloc(elf->phdr_list, new_size);
	if (new == NULL)
		return NULL;

	elf->phdr_list = new;
	Phdr *new_phdr = &new[elf->ehdr.e_phnum];
	memcpy(new_phdr, phdr, sizeof(Phdr));
	elf->ehdr.e_phnum += 1;

	free(phdr);
	phdr = NULL;

	return new_phdr;
}

void phdrSetType(Elf64_Word type)
{
	phdr->phdr.p_type = type;
}

void phdrSetFlags(Elf64_Word flags)
{
	phdr->phdr.p_flags = flags;
}

void phdrSetAlign(Elf64_Word align)
{
	phdr->phdr.p_align = align;
}

unsigned char *phdrSetData(unsigned char *data, size_t data_size)
{
	unsigned char *data_cpy = malloc(data_size);
	if (data_cpy == NULL)
		return NULL;

	memcpy(data_cpy, data, data_size);

	phdr->data = data_cpy;
	phdr->data_size = data_size;
	if (phdr->size == 0)
		phdr->size = data_size;

	return data_cpy;
}

Offset phdrGetOffset(Elf64_Off addend)
{
	Offset offset;
	offset.type = OFFTYPE_PHDR;
	offset.index = elf->ehdr.e_phnum;
	offset.addend = addend;

	return offset;
}

Elf64_Half phdrGetIndex(void)
{
	return elf->ehdr.e_phnum;
}

void beginShdr(void)
{
	if (shdr != NULL)
		endShdr();

	shdr = calloc(1, sizeof(Shdr));
	shdr->offset = shdrGetOffset(0);
}

Shdr *endShdr(void)
{
	if (shdr == NULL)
		return NULL;

	size_t new_size = (elf->ehdr.e_shnum + 1) * sizeof(Shdr);

	Shdr *new = realloc(elf->shdr_list, new_size);
	if (new == NULL)
		return NULL;

	elf->shdr_list = new;
	Shdr *new_shdr = &new[elf->ehdr.e_shnum];
	memcpy(new_shdr, shdr, sizeof(Shdr));
	elf->ehdr.e_shnum += 1;

	free(shdr);
	shdr = NULL;

	return new_shdr;
}

Elf64_Half shdrGetIndex(void)
{
	return elf->ehdr.e_shnum;
}

Offset shdrGetOffset(Elf64_Off addend)
{
	Offset offset;
	offset.type = OFFTYPE_SHDR;
	offset.index = elf->ehdr.e_shnum;
	offset.addend = addend;

	return offset;
}

void beginStrtab(void)
{
	if (shdr == NULL || shdr->shdr.sh_type != SHT_NULL)
		return;

	shdr->shdr.sh_type = SHT_STRTAB;
	shdr->data = malloc(1);
	shdr->data_size = 1;
}

void endStrtab(void)
{
	if (shdr == NULL || shdr->shdr.sh_type != SHT_STRTAB)
		return;
}

Elf64_Off strtabAddString(char *string)
{
}

void writeToMemory(Elf *ielf, unsigned char **buffer, size_t *buffer_size)
{
	Elf64_Off phoff = sizeof(Elf64_Ehdr);
	Elf64_Off phdataoff = phoff + ielf->ehdr.e_phnum * sizeof(Elf64_Phdr);
	Elf64_Off shoff = phdataoff;

	for (int ph = 0; ph < ielf->ehdr.e_phnum; ph++) {
		Phdr *iphdr = &ielf->phdr_list[ph];
		iphdr->data_offset = shoff;
		iphdr->phdr.p_offset = shoff;

		Elf64_Off p_offset;
		switch (iphdr->offset.type) {
			case OFFTYPE_ABSOLUTE:
				p_offset = 0;
				break;
			case OFFTYPE_PHDR:
				p_offset = ielf->phdr_list[iphdr->offset.index].phdr.p_offset;
				break;
			case OFFTYPE_SHDR:
				// TODO: Figure something out...
				break;
		}
		p_offset += iphdr->offset.addend;

		iphdr->phdr.p_offset = p_offset;
		iphdr->phdr.p_vaddr = p_offset + ielf->base_addr;
		iphdr->phdr.p_paddr = p_offset + ielf->base_addr;
		iphdr->phdr.p_filesz = iphdr->size;
		iphdr->phdr.p_memsz = iphdr->size;

		shoff += iphdr->data_size;
	}

	Elf64_Off shdataoff = shoff + ielf->ehdr.e_shnum * sizeof(Elf64_Shdr);
	*buffer_size = shdataoff;

	for (int sh = 0; sh < ielf->ehdr.e_shnum; sh++) {
		Shdr *ishdr = &ielf->shdr_list[sh];
		ishdr->data_offset = *buffer_size;
		ishdr->shdr.sh_offset = *buffer_size;

		Elf64_Off sh_offset;
		switch (ishdr->offset.type) {
			case OFFTYPE_ABSOLUTE:
				sh_offset = 0;
				break;
			case OFFTYPE_PHDR:
				sh_offset = ielf->phdr_list[ishdr->offset.index].phdr.p_offset;
				break;
			case OFFTYPE_SHDR:
				sh_offset = ielf->shdr_list[ishdr->offset.index].shdr.sh_offset;
				break;
		}
		sh_offset += ishdr->offset.addend;

		ishdr->shdr.sh_offset = sh_offset;
		ishdr->shdr.sh_size = ishdr->size;

		*buffer_size += ishdr->data_size;
	}

	switch (ielf->entry.type) {
		case OFFTYPE_ABSOLUTE:
			ielf->ehdr.e_entry = 0;
			break;
		case OFFTYPE_PHDR:
			ielf->ehdr.e_entry = ielf->phdr_list[ielf->entry.index].phdr.p_offset;
			break;
		case OFFTYPE_SHDR:
			ielf->ehdr.e_entry = ielf->shdr_list[ielf->entry.index].shdr.sh_offset;
			break;
	}
	ielf->ehdr.e_entry += ielf->entry.addend;
	ielf->ehdr.e_entry += ielf->base_addr;
	if (ielf->ehdr.e_phnum > 0)
		ielf->ehdr.e_phoff = phoff;
	if (ielf->ehdr.e_shnum > 0)
		ielf->ehdr.e_shoff = shoff;

	char *out = malloc(*buffer_size);
	*buffer = out;
	if (out == NULL)
		return;

	memcpy(&out[0], &ielf->ehdr, sizeof(Elf64_Ehdr));
	for (int ph = 0; ph < ielf->ehdr.e_phnum; ph++) {
		Phdr *iphdr = &ielf->phdr_list[ph];
		Elf64_Off iphoff = phoff + ph * sizeof(Elf64_Phdr);
		memcpy(&out[iphoff], &iphdr->phdr, sizeof(Elf64_Phdr));
		memcpy(&out[iphdr->data_offset], iphdr->data, iphdr->data_size);
	}
	for (int sh = 0; sh < ielf->ehdr.e_shnum; sh++) {
		Shdr *ishdr = &ielf->shdr_list[sh];
		Elf64_Off ishoff = shoff + sh * sizeof(Elf64_Shdr);
		memcpy(&out[ishoff], &ishdr->shdr, sizeof(Elf64_Shdr));
		memcpy(&out[ishdr->data_offset], ishdr->data, ishdr->data_size);
	}
}

void writeToFile(Elf *ielf, char *filename)
{
	unsigned char *buffer = NULL;
	size_t size = 0;
	writeToMemory(ielf, &buffer, &size);
	if (buffer == NULL) {
		return;
	}

	int fout = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0755);
	if (fout < 0) {
		return;
	}
	write(fout, buffer, size);
	close(fout);
}
