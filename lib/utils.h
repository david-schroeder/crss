#ifndef UTILS_H
#define UTILS_H

#include "settings.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <signal.h>
#include <unistd.h>

#include <GLFW/glfw3.h>

#define LVERBOSE(...) console_log(log_verbose, fnpath, __VA_ARGS__)
#define LDEBUG(...) console_log(log_debug, fnpath, __VA_ARGS__)
#define LINFO(...) console_log(log_info, fnpath, __VA_ARGS__)
#define LWARN(...) console_log(log_warn, fnpath, __VA_ARGS__)
#define LFATAL(...) console_log(log_fatal, fnpath, __VA_ARGS__)

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
Set the terminal color style to any style and color
@param int style_specifier - the style to be used
@param int color_specifier - the color to be used
@returns null
*/
void set_style(enum style_specifier style_specifier, enum color_specifier color_specifier);

/*
Resets the terminal color style
@param none
@returns null
*/
void reset_style();

/*
Set a string's color style to any style and color
@param char* dest - the string to be modified
@param int style_specifier - the style to be used
@param int color_specifier - the color to be used
@returns null
*/
void s_set_style(char* dest, enum style_specifier style_specifier, enum color_specifier color_specifier);

/*
Resets a string's color style
@param char* dest - the string to be reset
@returns null
*/
void s_reset_style(char* dest);

/*
Gets time string in the format HH:MM:SS
@params none
@returns char* containing a result string; should be freed
*/
char* get_time_string();

/*
Optionally logs a formatted message, depending on <level>
@param level - the minimum log level for the message to be shown
@param format - the format string to print
@return null
*/
void console_log(enum log_level level, const char* func_location, const char* format, ...);

/*
Returns a child function path like main.gui.draw
@param char* parent_path - the parent path e.g. main.gui
@param char* fn_name - the child path e.g. draw
@returns char* containing full fn_path e.g. main.gui.draw
*/
char* get_fn_path(const char* parent_path, const char* fn_name);

#endif // UTILS_H
