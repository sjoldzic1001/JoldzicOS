#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <uefi.h>
#define ldrClearScreen() ST->ConOut->ClearScreen(ST->ConOut) //clear screen

void ldrInitDisplayColors(uint8_t background, uint8_t text, int clearScreen);

// initialize display, set background and text colors, and clear screen
void ldrInitDisplayColors(uint8_t background, uint8_t text, int clearScreen) {
	ST->ConOut->SetAttribute(ST->ConOut, background); //set background color
	if(clearScreen) ldrClearScreen();
	ST->ConOut->SetAttribute(ST->ConOut, text); //set text color
}



#endif /* _DISPLAY_H */