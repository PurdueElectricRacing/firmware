#ifndef __TEST_HELPER_H__
#define __TEST_HELPER_H__
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define _T_ANSI_RED   "\x1b[31m"
#define _T_ANSI_GREEN "\x1b[32m"
#define _T_ANSI_RESET "\x1b[37m"

// Do not print colors if redirecting to file
#define __t_log_color_error(_color, ...) \
    do { \
        if (isatty(STDERR_FILENO)) { \
            fprintf(stderr, _color); \
        } \
        fprintf(stderr, __VA_ARGS__); \
        if (isatty(STDERR_FILENO)) { \
            fprintf(stderr, _T_ANSI_RESET); \
        } \
    } while (false);

#define t_start() int __t_line_first_fail = 0
#define t_check(_condition, ...) \
    do { \
        if (!(_condition)) { \
            fprintf(stderr, __VA_ARGS__); \
            fputc('\n', stderr); \
            if (__t_line_first_fail == 0) { \
                __t_line_first_fail = __LINE__; \
            } \
        } \
    } while (false);
#define t_end() return __t_line_first_fail

#define t_run(_function) \
    do { \
        int __t_line = _function(); \
        if (__t_line == 0) { \
            __t_log_color_error(_T_ANSI_GREEN, \
                                "Test passed: %s\n", \
                                (#_function)); \
        } else { \
            __t_log_color_error(_T_ANSI_RED, \
                                "Test failed: %s at line %d\n", \
                                (#_function), \
                                __t_line); \
        } \
    } while (false);

#define t_check_strings_eq(s1, s2) t_check(strcmp((s1), (s2)) == 0)

#endif
