#ifndef _STOP_H
#define _STOP_H

#include <uefi.h>
#include "display.h"
#include "efifunc.h"
#include "sysver.h"

void *mainPtr;

int ldrPrintStopCode(uint32_t ExceptionApplicationCode, uint32_t ExceptionParam1, uint32_t ExceptionParam2,
	uint32_t ExceptionParam3, uint32_t ExceptionParam4, char* applicationName);
void ldrHexDump(char *desc, void *addr, int len);
	
void ldrHexDump(char *desc, void *addr, int len) 
{
	/* Adapted from from https://gist.github.com/domnikl/af00cc154e3da1c5d965
	
	Copyright 2018 Dominik Liebler

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
    
	int i, j;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
	
	j = 0;

    // Output description if given.
    if (desc != NULL)
        printf ("Beginning memory minidump...\n\n  Addr  Value                                            Application     \n");

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
			j = 0;
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);
		
		int descLen = strlen(desc);
		
		if(j < descLen)
		{
			buff[i % 16] = desc[j];
		}
		else if (j >= descLen && descLen < 16) {
			buff[i % 16] = ' ';
		}
			
		j++;

        // And store a printable ASCII character for later.
        /*if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = ' ';
        } else {
            buff[i % 16] = ' ';
        }*/

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n\n", buff);
}

//stop code reporting function (exception and error reporting)
int ldrPrintStopCode(

	uint32_t 	ExceptionApplicationCode,
	uint32_t 	ExceptionParam1,
	uint32_t 	ExceptionParam2,
	uint32_t	ExceptionParam3,
	uint32_t 	ExceptionParam4,
	char* 		applicationName
	
	) {
	ldrInitDisplayColors(EFI_BACKGROUND_BLUE, EFI_WHITE, 1);
	
	char* description = "OSLDR_UNKNOWN_ERROR";
	//get exception code and print out description of it (BSoD) to user and then return a status.
	switch(ExceptionApplicationCode) {
		case 0xFFFFFFFF:
			// OSLDR error (0xFFFFFFFF)
			switch(ExceptionParam1) {
				// unknown error
				case 0x00000000:
					sprintf(description, "%s_UNKNOWN_ERROR", applicationName);
					break;
			
				// not implemented error
				case 0x00000001:
					sprintf(description, "%s_NOT_IMPLEMENTED", applicationName);
					break;
			
				// kernel load error
				case 0xFFFFFFFF:
					switch(ExceptionParam2) {
						//kernel unknown error
						case 0x00000000:
							sprintf(description, "%s_KERNEL_UNKNOWN_ERROR", applicationName);
							break;
						//kernel config load error
						case 0x00000001:
							switch(ExceptionParam3) {
								//kernel config unkown error
								case 0x00000000:
									sprintf(description, "%s_KERNEL_CONFIG_UNKOWN_ERROR", applicationName);
									break;
								
								//kernel config file not found
								case 0x00000001:
									sprintf(description, "%s_KERNEL_CONFIG_NOT_FOUND", applicationName);
									break;
								
								//kernel config malloc failed
								case 0xFFFFFFFF:
									sprintf(description, "%s_KERNEL_CONFIG_OUT_OF_MEMORY", applicationName);
									break;
								
								//default to kernel config unknown error
								default:
									sprintf(description, "%s_KERNEL_CONFIG_UNKOWN_ERROR", applicationName);
									break;
							}
							break;
							
						//kernel file load error
						case 0x00000002:
							switch(ExceptionParam3) {
								//kernel config unkown error
								case 0x00000000:
									sprintf(description, "%s_KERNEL_FILE_UNKOWN_ERROR", applicationName);
									break;
								
								//kernel config file not found
								case 0x00000001:
									sprintf(description, "%s_KERNEL_NOT_FOUND", applicationName);
									break;
									
								case 0x00000002:
									sprintf(description, "%s_KERNEL_VMODESET_ERROR", applicationName);
									break;
									
								case 0x00000003:
									sprintf(description, "%s_KERNEL_GRAPHICS_OUTPUT_PROTOCOL_ERROR", applicationName);
									break;
								
								//kernel config malloc failed
								case 0xFFFFFFFF:
									sprintf(description, "%s_KERNEL_OUT_OF_MEMORY", applicationName);
									break;
								
								//default to kernel config unknown error
								default:
									sprintf(description, "%s_KERNEL_UNKOWN_ERROR", applicationName);
									break;
							}
							break;
						//default to kernel unknown error
						default:
							sprintf(description, "%s_KERNEL_UNKNOWN_ERROR", applicationName);
							break;
					}
					break;
			
				//default to error status and don't print any errors if no error matches.
				default:
					sprintf(description, "%s_UNKNOWN_ERROR", applicationName);
					break;
			}
			break;
			
		//default to unknown error
		default:
			sprintf(description, "%s_UNKNOWN_ERROR", applicationName);
			break;
	}
	
	char* cpuid_vendor = get_cpuid_vendor();
	printf("*** STOP: 0x%08X (0x%08X, 0x%08X, 0x%08X, 0x%08X)\n%s\n\nCPUID: %s  irql: 03  SYSVER: 0x%08X\n\n", ExceptionApplicationCode, ExceptionParam1, ExceptionParam2, ExceptionParam3, ExceptionParam4, description, cpuid_vendor, SYSVER);
	
	ldrHexDump(applicationName, mainPtr, 128);
	
	printf("The application \"%s\" has encountered a serious problem and %s has\nbeen shut down to prevent damage to your computer.\n\nIf this is the first time you have encountered this Stop error message, try\nrestarting your computer. If the issue persists, consult your IT administrator,\nthe %s forums, or your hardware manufacturer for more assistance.\n\nPress any key to reboot the system...",
		applicationName, OSNAME, OSNAME);
		
	free(cpuid_vendor);
	free(applicationName);
	
	ldrWaitForKey();
	return -1;
}

#endif /* _STOP_H */