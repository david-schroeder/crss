#ifndef SETTINGS_H
#define SETTINGS_H

#include "version.h"

extern char* SOFTWARE_NAME;
extern char* VERSION_STRING;

extern int MAX_LOG_FMT_LEN;
extern int MAX_FN_PATH_LEN;
extern const char* FNP_DELIM;
extern int LOG_LEVEL;

extern char* SERVER_IP;
extern int SERVER_PORT;
extern int WITH_GUI;

/*
Resets all settings to their default state.
@params none
@returns null
*/
void reset_all_settings();

#endif // SETTINGS_H
