#include "settings.h"

char* SOFTWARE_NAME;
char* VERSION_STRING;

int MAX_LOG_FMT_LEN;
int MAX_FN_PATH_LEN;
const char* FNP_DELIM;
int LOG_LEVEL;

char* SERVER_IP;
int SERVER_PORT;
int WITH_GUI;

void reset_all_settings() {
    SOFTWARE_NAME = "CRSS";
    VERSION_STRING = get_version_string();

    MAX_LOG_FMT_LEN = 256;
    MAX_FN_PATH_LEN = 224;
    FNP_DELIM = ".";
    LOG_LEVEL = 0;

    SERVER_IP = "127.0.0.1";
    SERVER_PORT = 25565;
    WITH_GUI = 1;
}
