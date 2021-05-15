#include "KRNLFUNC.H"

/* kernel main entry point. kargc is argument count and kargv is argument string */
int kmain(int kargc, kstring kargv) {
	kstring prompt = "OS Loader v4.01\n";
	kprint(BLACK, prompt);
	/*kstring prompt2 = "Invoking \"Last Known Good\" configuration...\n";
	kprint(BLACK, prompt2);
	kstring prompt3 = "Starting Joldzic OS...STOP 0x01: Not implemented";
	kprint(BLACK, prompt3);*/
	return 0;
}

void main() {
	kclearscr();
	kstring arguments = "";
	//kprint(BLACK,arguments);
	int retval = kmain(0, arguments);
	
	if (retval != 0) {
		kstring error = "OSLDR: The kernel has reported an error. Halting now!";
		kprint(BLACK, error);
		asm("hlt");
	}
}