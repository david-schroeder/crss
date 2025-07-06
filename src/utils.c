#include "utils.h"

void set_style(enum style_specifier style_specifier, enum color_specifier color_specifier) {
    printf("\033[%d;%dm", style_specifier, color_specifier);
}

void reset_style() {
    set_style(style_regular, color_regular);
}

void s_set_style(char* dest, enum style_specifier style_specifier, enum color_specifier color_specifier) {
    char* tmp = malloc(8*sizeof(char));
    sprintf(tmp, "\033[%d;%dm", style_specifier, color_specifier);
    strcat(dest, tmp);
    free(tmp);
}

void s_reset_style(char* dest) {
    s_set_style(dest, style_regular, color_regular);
}

char* get_time_string() {
    time_t now;
    time(&now);
    struct tm* local = localtime(&now);
    char* ret = malloc(9*sizeof(char)); // HH:MM:SS\0 is 9 characters
    sprintf(ret, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
    return ret;
}

// NOTE: think about what messages are sent to GUI console / ZMQ / multiple channels / etc...
// TODO: rewrite
void console_log(enum log_level level, const char* func_location, const char* format, ...) {

    if (level < LOG_LEVEL) { return; }

    char* formatted_string = calloc(MAX_LOG_FMT_LEN, sizeof(char));
    char* tmp = calloc(MAX_LOG_FMT_LEN, sizeof(char));

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(tmp, MAX_LOG_FMT_LEN, format, argptr);
    va_end(argptr);

    // [%02d:%02d:%02d] [%s::%s::%s] This is a message!
    // [15:08:34] [CRSS::FATAL::main.network.handler.758.login] Client disconnected

    // [
    s_reset_style(formatted_string);
    strcat(formatted_string, "[");

    // 15:08:34] [
    s_set_style(formatted_string, style_regular, color_cyan);
    char* time_string = get_time_string();
    strcat(formatted_string, time_string);
    s_reset_style(formatted_string);
    strcat(formatted_string, "] [");

    // CRSS::
    s_set_style(formatted_string, style_regular, color_green);
    strcat(formatted_string, SOFTWARE_NAME);
    s_reset_style(formatted_string);
    strcat(formatted_string, "::");

    switch (level) {
    case log_verbose:
        s_set_style(formatted_string, style_regular, color_purple);
        strcat(formatted_string, "VERB");
        break;
    
    case log_debug:
        s_set_style(formatted_string, style_dark, color_red);
        strcat(formatted_string, "DEBG");
        break;
    
    case log_info:
        s_set_style(formatted_string, style_regular, color_white);
        strcat(formatted_string, "INFO");
        break;
    
    case log_warn:
        s_set_style(formatted_string, style_regular, color_yellow);
        strcat(formatted_string, "WARN");
        break;
    
    case log_fatal:
        s_set_style(formatted_string, style_background, color_red);
        strcat(formatted_string, "FAIL");
        break;
    
    default:
        break;
    }

    s_reset_style(formatted_string);
    strcat(formatted_string, "::");

    s_set_style(formatted_string, style_regular, color_blue);
    strcat(formatted_string, func_location);
    s_reset_style(formatted_string);
    strcat(formatted_string, "] ");

    switch (level) {
    case log_warn:
        s_set_style(formatted_string, style_regular, color_yellow);
        break;
    
    case log_fatal:
        s_set_style(formatted_string, style_background, color_red);
        break;
    
    default:
        s_reset_style(formatted_string);
        break;
    }

    strcat(formatted_string, tmp);
    s_reset_style(formatted_string);
    strcat(formatted_string, "\n");

    printf("%s", formatted_string);
    fflush(stdout);

    free(formatted_string);
    free(tmp);
    free(time_string);
}

char* get_fn_path(const char* parent_path, const char* fn_name) {
    char* s = calloc(MAX_FN_PATH_LEN, sizeof(char));
    strcat(s, parent_path);
    strcat(s, FNP_DELIM);
    strcat(s, fn_name);
    return s;
}
