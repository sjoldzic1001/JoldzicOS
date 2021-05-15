#define BLACK 0x07 // white on black text
#define BLUE 0x1f // blue background

#define DEFAULT_BG BLUE

#define VIDEO_MEM 0xb8000
#define MAX_CHAR_WIDTH 80
#define MAX_CHAR_HEIGHT 25

// kstring (kernel string, just a generic string type) is a char[8192]
typedef char kstring[256];

unsigned int line = 0;

void kprint(int color, kstring string);
void kclearscr();
int kmain(unsigned int kargc, kstring kargv);
//void kstrcat(kstring string1, kstring string2);

void kclearscr() {
	volatile char *video = (volatile char*)VIDEO_MEM;
	unsigned int i = 0;
	while(i < (MAX_CHAR_WIDTH * MAX_CHAR_HEIGHT * 2)){
		video[i] = ' ';
		i++;
		video[i] = DEFAULT_BG;
		i++;
	}
}

void kprint(int color, kstring string) {
	volatile char *video = (volatile char*)VIDEO_MEM;
	unsigned int i = (line*MAX_CHAR_WIDTH*2);
	unsigned int j = 0;
	
	while (string[j] != '\0'){
		if(string[j] == '\n') {
			//const char newlinestr[] = "A";
			//printscr(color, newlinestr, line);
			line++;
			i=(line*MAX_CHAR_WIDTH*2);
		} else {
			video[i] = string[j];
			i++;
			video[i] = color;
			i++;
		}
		j++;
	}
	
	line++;
}

//this is a concatenation operation. string2 is concatenated into string1. Result stored in string1.
/*void kstrcat(kstring string1, kstring string2) {
	int str1_index = 0;//index of string 1
	int str2_index = 0;//index of string 2
	
	//find where string1 ends by searching for its null terminator.
	while(string1[str1_index] != '\0') {
		str1_index++;
	}
	
	//start copying string2 where string1 ends, replacing string1's null terminator and moving it to the
	//data copied from string2.
	while(string2[str2_index] != '\0') {
		string1[str1_index] = string2[str2_index];
		str1_index++;
		str2_index++;
	}
	
	//set new null terminator to the end of the copied data from string2.
	string1[str1_index] = '\0';
}*/

