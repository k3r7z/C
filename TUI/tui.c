#include <stdio.h>
#include "tui.h"

void tui_print_error(const char *msg){
    fprintf(stderr, "%s%s[ FAILED ]%s %s\n", COLOR_BOLD, COLOR_RED, COLOR_RESET, msg);
}

void tui_print_info(const char *msg){
    fprintf(stdout, "%s%s[  INFO  ]%s %s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET, msg);
}

void tui_print_success(const char *msg){
    fprintf(stdout, "%s%s[   OK   ]%s %s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET, msg);
}
