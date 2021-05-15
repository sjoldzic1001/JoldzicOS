#ifndef _LOAD_KERNEL_H
#define _LOAD_KERNEL_H

#include <uefi/stdio.c>
#include "stop.h"

#include "kernel/bootparam.h"

/*** ELF64 defines and structs ***/
#define ELFMAG      "\177ELF"
#define SELFMAG     4
#define EI_CLASS    4       /* File class byte index */
#define ELFCLASS64  2       /* 64-bit objects */
#define EI_DATA     5       /* Data encoding byte index */
#define ELFDATA2LSB 1       /* 2's complement, little endian */
#define ET_EXEC     2       /* Executable file */
#define PT_LOAD     1       /* Loadable program segment */
#ifdef __x86_64__
#define EM_MACH     62      /* AMD x86-64 architecture */
#endif
#ifdef __aarch64__
#define EM_MACH     183     /* ARM aarch64 architecture */
#endif

typedef struct
{
    uint8_t  e_ident[16];   /* Magic number and other info */
    uint16_t e_type;        /* Object file type */
    uint16_t e_machine;     /* Architecture */
    uint32_t e_version;     /* Object file version */
    uint64_t e_entry;       /* Entry point virtual address */
    uint64_t e_phoff;       /* Program header table file offset */
    uint64_t e_shoff;       /* Section header table file offset */
    uint32_t e_flags;       /* Processor-specific flags */
    uint16_t e_ehsize;      /* ELF header size in bytes */
    uint16_t e_phentsize;   /* Program header table entry size */
    uint16_t e_phnum;       /* Program header table entry count */
    uint16_t e_shentsize;   /* Section header table entry size */
    uint16_t e_shnum;       /* Section header table entry count */
    uint16_t e_shstrndx;    /* Section header string table index */
} Elf64_Ehdr;

typedef struct
{
    uint32_t p_type;        /* Segment type */
    uint32_t p_flags;       /* Segment flags */
    uint64_t p_offset;      /* Segment file offset */
    uint64_t p_vaddr;       /* Segment virtual address */
    uint64_t p_paddr;       /* Segment physical address */
    uint64_t p_filesz;      /* Segment size in file */
    uint64_t p_memsz;       /* Segment size in memory */
    uint64_t p_align;       /* Segment alignment */
} Elf64_Phdr;


char** splitString(char* string, char delim);
int ldrLoadKernel(char* kernel_path, int kargc, char** kargv);
struct KERNEL_CONFIG ldrLoadKernelConfig(char* kconf_path);

struct KERNEL_CONFIG {
	char* KernelFile;
	char** KernelCmdLine;
	int KernelCmdArgc;
	int success;
};

char** splitString(char* string, char delim) {
		size_t counter = 0;
		size_t stringLength = strlen(string);
		int i = 1;
	
		for(i; i < stringLength; i++) {
			if(string[i] == delim) {
				counter++;
			}
		}
		
		char** result = malloc(counter * sizeof(char*));
		
		for(int j = 0; j < counter; j++) {
			result[j] = malloc(sizeof(char*));
		}
		
		counter = 0;
		
		int copyStart = 0;
		int copyEnd = 0;

		for(i = 0; i < stringLength; i++) {
			if(string[i] == delim)
			{
				copyEnd = i;
				//printf("cs: %d, ce: %d\n", copyStart, copyEnd);
				strncpy(result[counter], string + copyStart, copyEnd - copyStart);
				counter++;
				copyStart = i + 1;
			}
		}
		
		return result;
}	

int ldrLoadKernel(char* kernel_path, int kargc, char** kargv) {
	FILE *f;
    char *buff;
    long int size;
    Elf64_Ehdr *elf;
    Elf64_Phdr *phdr;
    uintptr_t entry;
    bootparam_t bootp;
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    efi_gop_mode_info_t *info = NULL;
    uintn_t isiz = sizeof(efi_gop_mode_info_t);
    int i, j;

    /* load the file */
    if((f = fopen(kernel_path, "r"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buff = malloc(size + 1);
        if(!buff) {
            return ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000002, 0xFFFFFFFF, 0x00000000, "OSLDR");;
        }
        fread(buff, size, 1, f);
        fclose(f);
    } else {
        return ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000002, 0x00000001, 0x00000000, "OSLDR");;
    }

    /* set up boot parameters passed to the "kernel" */
    memset(&bootp, 0, sizeof(bootparam_t));
    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) {
        status = gop->SetMode(gop, 0);
        ST->ConOut->Reset(ST->ConOut, 0);
        ST->StdErr->Reset(ST->StdErr, 0);
        if(EFI_ERROR(status)) {
            return ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000002, 0x00000002, 0x00000000, "OSLDR");;
        }
        bootp.framebuffer = (unsigned int*)gop->Mode->FrameBufferBase;
        bootp.width = gop->Mode->Information->HorizontalResolution;
        bootp.height = gop->Mode->Information->VerticalResolution;
        bootp.pitch = sizeof(unsigned int) * gop->Mode->Information->PixelsPerScanLine;
    } else {
        fprintf(stderr, "unable to get graphics output protocol\n");
        return ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000002, 0x00000003, 0x00000000, "OSLDR");;
    }
    if(kargc > 1) {
        bootp.argc = kargc - 1;
        bootp.argv = (char**)malloc(kargc * sizeof(char*));
        if(bootp.argv) {
            for(i = 0; i < bootp.argc; i++)
                if((bootp.argv[i] = (char*)malloc(strlen(kargv[i + 1]) + 1)))
                    strcpy(bootp.argv[i], kargv[i + 1]);
            bootp.argv[i] = NULL;
        }
    }

    /* is it a valid ELF executable for this architecture? */
    elf = (Elf64_Ehdr *)buff;
    if(!memcmp(elf->e_ident, ELFMAG, SELFMAG) &&    /* magic match? */
        elf->e_ident[EI_CLASS] == ELFCLASS64 &&     /* 64 bit? */
        elf->e_ident[EI_DATA] == ELFDATA2LSB &&     /* LSB? */
        elf->e_type == ET_EXEC &&                   /* executable object? */
        elf->e_machine == EM_MACH &&                /* architecture match? */
        elf->e_phnum > 0) {                         /* has program headers? */
            /* load segments */
            for(phdr = (Elf64_Phdr *)(buff + elf->e_phoff), i = 0;
                i < elf->e_phnum;
                i++, phdr = (Elf64_Phdr *)((uint8_t *)phdr + elf->e_phentsize)) {
                    if(phdr->p_type == PT_LOAD) {
                        memcpy((void*)phdr->p_vaddr, buff + phdr->p_offset, phdr->p_filesz);
                        memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
                    }
                }
            entry = elf->e_entry;
    } else {
        fprintf(stderr, "not a valid ELF executable for this architecture\n");
        return 0;
    }
    /* free resources */
    free(buff);

    /* exit this UEFI bullshit */
    if(exit_bs()) {
        return ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000002, 0x00000000, 0x00000000, "OSLDR");
    }

    /* execute the "kernel" */
    (*((void(* __attribute__((sysv_abi)))(bootparam_t *))(entry)))(&bootp);
    /* failsafe, should never return just in case */
    while(1);

    return 0;

}

struct KERNEL_CONFIG ldrLoadKernelConfig(char* kconf_path) {
	
	struct KERNEL_CONFIG kConf;
	
	kConf.KernelCmdLine = malloc(1 * sizeof(char*));
	
	printf("Loading kernel configuration...\n");
    
	FILE* kconf_file = fopen(kconf_path, "r");
	
	if(kconf_file == NULL) {
		ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0x00000001, 0x00000000, "OSLDR");
		kConf.KernelFile = NULL;
		kConf.KernelCmdLine = NULL;
		kConf.KernelCmdArgc = 0;
		kConf.success = 0;
		return kConf;
	}
	
    char *buff;
    long int size;

	fseek(kconf_file, 0, SEEK_END);
    size = ftell(kconf_file);
    fseek(kconf_file, 0, SEEK_SET);
    buff = malloc(size + 1);
    if(!buff) {
        ldrPrintStopCode(0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 0x00000000, "OSLDR");
		kConf.KernelFile = NULL;
		kConf.KernelCmdLine = NULL;
		kConf.KernelCmdArgc = 0;
		kConf.success = 0;
		return kConf;
    }
    fread(buff, size, 1, kconf_file);
    buff[size] = 0;
	
	char *confFileOutput;
	memcpy(confFileOutput, buff, strlen(buff));
	
	fclose(kconf_file);
	
	const char s[2] = "\n";
	char *token;
   
	/* get the first token */
	token = strtok(confFileOutput, s);
   
	/* walk through other tokens */
	while( token != NULL ) {
      printf( " %s\n", token );
    
      token = strtok(NULL, s);
	}
	
	kConf.KernelFile = buff;
	kConf.KernelCmdLine[0] = buff;
	kConf.KernelCmdArgc = 0;
	kConf.success = 1;
	
	return kConf;
}

#endif /* _LOAD_KERNEL_H */