#include <efi.h>
#include <efilib.h>
#include "stop

#define OSLDR_VERSION L"V4.01"

//function declarations
int ldrMain();
int ldrPrintStopCode(uint32_t ExceptionCode);

//EFI entry point function
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  //initialize EFI libraries
  InitializeLib(ImageHandle, SystemTable);
  
  //Set background color, text color, and clear screen
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 1, SystemTable->ConOut, EFI_BACKGROUND_BLUE); //SET BACK COLOR
  uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 1, SystemTable->ConOut, EFI_WHITE); //SET TEXT COLOR
  
  //Set watchdog timer to disable so that on some systems, the UEFI bootloader doesn't time out after 5 minutes.
  uefi_call_wrapper(BS->SetWatchdogTimer, 0, 0, 0, NULL); //Disable watchdog timer so OSLDR doesn't hang in UEFI
  
  //Call bootloader main entry point function ldrmain() and check for errors.
  if(ldrMain()) //if ldrMain() successfully ran
  {
    return EFI_SUCCESS; //tell UEFI that our bootloader has successfully executed.
  }
  
  //if there were errors with ldrMain()
  Print(L"Press any key to enter EFI shell...\n"); //Prompt user to press any key to exit and drop to UEFI shell.
  Pause(); //Wait until user presses a key to exit.
  return EFI_LOAD_ERROR; //tell EFI that there was an error running our bootloader.
}

//stop code reporting function (exception and error reporting)
int ldrPrintStopCode(uint32_t ExceptionCode) {
	//get exception code and print out description of it to user and then return a status.
	switch(ExceptionCode) {
		// unknown error
		case 0x00000000:
			Print(L"OSLDR: STOP (0x00000000): Unknown error!\n");
			return 0;
			break;
			
		// not implemented error
		case 0x00000001:
			Print(L"OSLDR: STOP (0x00000001): Not implemented!\n");
			return 0;
			break;
			
		// kernel not found error
		case 0xFFFFFFFF:
			Print(L"OSLDR: STOP (0xFFFFFFFF): Unable to load kernel!\n");
			return 0;
			break;
			
		//default to error status and don't print any errors if no error matches.
		default:
			Print(L"OSLDR: STOP (0x00000000): Unknown error!\n");
			return 0;
			break;
	}
}

//primary bootloader entry point function
int ldrMain() {
	//set default return result (1 is successful boot, 0 is error occured)
	int success = 1;
	
	//print OS Loader version
	Print(L"OS Loader ");
	Print(OSLDR_VERSION);
	Print(L"\n\n");
	
	//we have not implemented anything yet so print stop code for not implemented
	//(STOP code 0x00000001) and store an error value returned from it as success.
	success = ldrPrintStopCode(0xFFFFFFFF);
	
	//return success as result
	return success;
}
	