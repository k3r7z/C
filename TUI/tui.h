#ifndef TUI_H
#define TUI_H
#define SYM_CHECK     "\xE2\x9C\x94"
#define SYM_CROSS     "\xE2\x9C\x98"
#define SYM_WARN      "\xE2\x9A\xA0"
#define SYM_INFO      "\xE2\x84\xB9"
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

void tui_print_error(const char *fmt_msg, ...);
void tui_print_info(const char *fmt_msg, ...);
void tui_print_success(const char *fmt_msg, ...);

#endif
