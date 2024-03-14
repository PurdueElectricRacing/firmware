#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdbool.h>

extern void _log_str(char* data);
extern char log_buffer[];

#define log_printf(...) do { \
    sprintf(log_buffer, __VA_ARGS__); \
    _log_str(log_buffer);             \
} while(false)
// #define log_printf(...) sprintf(log_buffer, __VA_ARGS__)


#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[37m"

#define __mu_log_color(color, ...) do { \
    log_printf("%s", color);       \
	log_printf(__VA_ARGS__);       \
    log_printf("%s", ANSI_RESET);  \
} while(false)

#ifdef DEBUG_LOG

#    define log_msg(...) log_printf(__VA_ARGS__)
#    define log_int(n) (log_printf("%s == %d\n", (#n), (n)))
#    define log_str(s) (log_printf("%s == %s\n", (#s), (s)))
#    define log_char(c) (log_printf("%s == '%c'\n", (#c), (c)))
#    define log_addr(addr) (log_printf("%s == %p\n", (#addr), (void*)(addr)))
#    define log_red(...) __mu_log_color(ANSI_RED, __VA_ARGS__)
#    define log_green(...) __mu_log_color(ANSI_GREEN, __VA_ARGS__)
#    define log_yellow(...) __mu_log_color(ANSI_YELLOW, __VA_ARGS__)
#    define log_blue(...) __mu_log_color(ANSI_BLUE, __VA_ARGS__)
#    define log_magenta(...) __mu_log_color(ANSI_MAGENTA, __VA_ARGS__)
#    define log_cyan(...) __mu_log_color(ANSI_CYAN, __VA_ARGS__)

#else

#    define log_msg(...)
#    define log_int(n)
#    define log_str(s)
#    define log_char(c)
#    define log_addr(addr)
#    define log_red(...)
#    define log_green(...)
#    define log_yellow(...)
#    define log_blue(...)
#    define log_magenta(...)
#    define log_cyan(...)

#endif

#endif
