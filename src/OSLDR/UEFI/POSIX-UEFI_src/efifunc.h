#ifndef _EFIFUNC_H
#define _EFIFUNC_H

#include <cpuid.h>

// EFI function redefinitions for readability
#define ldrWaitForKey() gBS->WaitForEvent(1, &gST->ConIn->WaitForKey,0) //wait for user to press any key
#define ldrDisableUEFIWatchdog() BS->SetWatchdogTimer(0, 0, 0, NULL) /* disable UEFI watchdog so bootloader
																	    doesn't crash after 5 minutes.
																	  */
#define ldrReboot() ST->RuntimeServices->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL); //Reboot system

// method declaration for cpu vendor string
static inline char* get_cpuid_vendor();

// Get CPU vendor string using cpuid asm instruction
static inline char* get_cpuid_vendor()
{
        char* vendor = malloc(13); /* statically allocate string size of 13 for cpu vendor.
									  12 for the CPU vendor string characters and 1 for the
									  null terminator character ('\0').
									*/
		
		int eax,ebx,ecx,edx; //declare integers for cpu registers eax, ebx, ecx, edx
		
		__cpuid(0, eax, ebx, ecx, edx); /* run cpuid instruction with eax being 0 to indicate
										   to the CPU that we went it's vendor string. Store
										   values of eax, ebx, ecx, and edx in their respective
										   integer variables declared above.
										 */
		
		/* The vendor string is stored in three registers in this order: ebx, edx, ecx.
		   Each register stores four characters of text, for a grand total of 12 characters.
		 */
		strncpy(vendor, (const char*)&ebx, 4); //Copy ebx's four characters first to vendor string.
		strncpy(vendor+4, (const char*)&edx, 4); //Copy edx next. Offset copy starting point by 4 characters.
		strncpy(vendor+8, (const char*)&ecx, 4); //Finally, copy ecx. Offset copy starting point by 8 characters.
		vendor[13] = '\0'; //Null terminate vendor string on character number 13.
		
		return vendor; //return completed vendor string.
}

#endif /* _EFIFUNC_H */