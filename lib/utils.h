#ifndef UTILS_H
#define UTILS_H

#include "settings.h"
#include "internals.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

/*
Log verbose message.

Assumes `fnpath` is defined and contains the current function identifier.
*/
#define LVERBOSE(...) console_log(log_verbose, fnpath, __VA_ARGS__)

/*
Log debug message.

Assumes `fnpath` is defined and contains the current function identifier.
*/
#define LDEBUG(...) console_log(log_debug, fnpath, __VA_ARGS__)

/*
Log info message.

Assumes `fnpath` is defined and contains the current function identifier.
*/
#define LINFO(...) console_log(log_info, fnpath, __VA_ARGS__)

/*
Log warning message.

Assumes `fnpath` is defined and contains the current function identifier.
*/
#define LWARN(...) console_log(log_warn, fnpath, __VA_ARGS__)

/*
Log fatal message.

Assumes `fnpath` is defined and contains the current function identifier.
*/
#define LFATAL(...) console_log(log_fatal, fnpath, __VA_ARGS__)

#if __STDC_VERSION__ < 199901L
    #define DLVERBOSE(...) console_log_direct(log_verbose, fnpath, __VA_ARGS__)
    #define DLDEBUG(...) console_log_direct(log_debug, fnpath, __VA_ARGS__)
    #define DLINFO(...) console_log_direct(log_info, fnpath, __VA_ARGS__)
    #define DLWARN(...) console_log_direct(log_warn, fnpath, __VA_ARGS__)
    #define DLFATAL(...) console_log_direct(log_fatal, fnpath, __VA_ARGS__)
#else
    #define DLVERBOSE(...) console_log_direct(log_verbose, __func__, __VA_ARGS__)
    #define DLDEBUG(...) console_log_direct(log_debug, __func__, __VA_ARGS__)
    #define DLINFO(...) console_log_direct(log_info, __func__, __VA_ARGS__)
    #define DLWARN(...) console_log_direct(log_warn, __func__, __VA_ARGS__)
    #define DLFATAL(...) console_log_direct(log_fatal, __func__, __VA_ARGS__)
#endif

enum log_level {
    log_verbose=0,
    log_debug=1,
    log_info=2,
    log_warn=3,
    log_fatal=4
};

enum style_specifier {
    style_regular=0,
    style_bold=1,
    style_dark=2,
    style_italic=3,
    style_underlined=4,
    style_blinking=5,
    style_background=7,
    style_invisible=8,
    style_strikethrough=9,
    style_double_under=21
};

enum color_specifier {
    color_regular=0,
    color_black=30,
    color_red=31,
    color_green=32,
    color_yellow=33,
    color_blue=34,
    color_purple=35,
    color_cyan=36,
    color_white=37
};

/*
Set the terminal color and style
@param style_specifier Style to be used
@param color_specifier Color to be used
*/
void set_style(enum style_specifier style_specifier, enum color_specifier color_specifier);

/*
Resets terminal color style
*/
void reset_style();

/*
Set a string's color and style
@param dest String to be modified
@param style_specifier Style to be used
@param color_specifier Color to be used
*/
void s_set_style(char* dest, enum style_specifier style_specifier, enum color_specifier color_specifier);

/*
Resets a string's color style
@param dest The string to be reset
*/
void s_reset_style(char* dest);

/*
Gets current time as string in the format HH:MM:SS

@return String containing current time; must be freed
*/
char* get_time_string();

/*
# CRSS Logger

Optionally logs a formatted message, depending on <level>.
@param level - the minimum log level for the message to be shown
@param format - the format string to print
*/
void console_log(enum log_level level, const char* func_location, const char* format, ...);

void console_log_direct(enum log_level level, const char* func_location, const char* format, ...);

/*
Get Callee function identifier.

@param parent_path Parent function identifier
@param fn_name Callee identifier
@return Full callee identifier string. Must be freed.
*/
char* get_fn_path(const char* parent_path, const char* fn_name);

#define FUNCPATH(funcname) fnpath = get_fn_path(fnpath, funcname)

#endif // UTILS_H
