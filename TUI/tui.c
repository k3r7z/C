#include <stdio.h>
#include <stdarg.h>
#include "tui.h"

void tui_print_error(const char *fmt_msg, ...){
    va_list args;
    va_start(args, fmt_msg);
    fprintf(stderr, "%s%s[ FAILED ]%s ", COLOR_BOLD, COLOR_RED, COLOR_RESET);
    vfprintf(stderr, fmt_msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void tui_print_info(const char *fmt_msg, ...){
    va_list args;
    va_start(args, fmt_msg);
    fprintf(stdout, "%s%s[  INFO  ]%s ", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    vfprintf(stdout, fmt_msg, args);
    fprintf(stdout, "\n");
    va_end(args);
}

void tui_print_success(const char *fmt_msg, ...){
    va_list args;
    va_start(args, fmt_msg);
    fprintf(stdout, "%s%s[   OK   ]%s ", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    vfprintf(stdout, fmt_msg, args);
    fprintf(stdout, "\n");
    va_end(args);
}
