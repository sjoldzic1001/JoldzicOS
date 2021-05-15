#include <uefi.h>
#include "stop.h"
#include "display.h"
#include "efifunc.h"
#include "load_kernel.h"
#include "sysver.h"

void write_string( int color, const char *string );

void write_string( int color, const char *string )
{
    volatile char *video = (volatile char*)0xB8000;
    while( *string != 0 )
    {
        *video++ = *string++;
        *video++ = color;
    }
}

// main entry point function
int main (int argc, char **argv)
{
  
  mainPtr = main;
  
  ldrDisableUEFIWatchdog();
  ldrInitDisplayColors(EFI_BACKGROUND_BLACK, EFI_WHITE, 1);
  
  printf("OS Loader V%s\n\nPress ESC key to invoke \"Last Known Good\" configuration menu...\n\n", OSLDR_VERSION);
  
  sleep(5);

  struct KERNEL_CONFIG kConfig = ldrLoadKernelConfig("\\EFI\\BOOT\\config.txt");
  
  if(!kConfig.success) {
		ldrInitDisplayColors(EFI_BACKGROUND_BLACK, EFI_WHITE, 1);
		printf("Rebooting system now...");
	  	ldrReboot();
  }
  
  
  printf("kfile: \"%s\"\n", kConfig.KernelFile);
  printf("kcmdline: \"%s\"", kConfig.KernelCmdLine[0]);
  ldrWaitForKey();
  
  
  ldrLoadKernel(kConfig.KernelFile, 1, kConfig.KernelCmdLine);
}