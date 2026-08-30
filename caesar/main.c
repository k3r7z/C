#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "../TUI/tui.h"


int main(int argc, char *argv[]){
    if(argc == 1){
	tui_print_error("Shift value is needed");
	return -1;
    }
	
    int shift = atoi(argv[1]);
    shift = shift % 26;
    if(shift < 0)
	shift += 26;

    int c;
    while( (c = getchar()) != EOF){
	if(isalpha(c)){
	    char base = islower(c) ? 'a' : 'A';
	    putchar( (c - base + shift) % 26 + base);
	}
	else
	    putchar(c);
    }
    
    return 0;
}
