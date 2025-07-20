#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
#include <pthread.h>

#include "version.h"

extern char *SOFTWARE_NAME;
extern char *LONG_SOFTWARE_NAME;
extern char *VERSION_STRING;
extern char *SOFTWARE_YEAR;

extern int MAX_LOG_FMT_LEN;
extern int MAX_FN_PATH_LEN;
extern int MAX_CMD_LEN;
extern const char *FNP_DELIM;
extern int LOG_LEVEL;

/*
Simple log.
When `LOG_SIMPLE = true`, console logs are printed to `stdout` and optionally a logfile.
The main program initiates a more complex logging handler which makes use of
ZMQ sockets to propagate messages to various targets, including the GUI console,
various connected network entities as well as the standard `stdout` and logfile
targets. This system is used when `LOG_SIMPLE = false`.
*/
extern bool LOG_SIMPLE;

extern bool IN_TERMINAL_MODE;
extern char *PROMPT_STRING;

extern char *SERVER_IP;
extern int SERVER_PORT;
extern int WITH_GUI;

/*
Resets all settings to their default state.
@return null
*/
void reset_all_settings();

/*
Toggles `LOG_SIMPLE`.

@return New value of `LOG_SIMPLE`.
*/
bool switch_logger_protocol(void);

#endif // SETTINGS_H
