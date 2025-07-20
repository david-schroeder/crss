#include "settings.h"

char *SOFTWARE_NAME;
char *LONG_SOFTWARE_NAME;
char *VERSION_STRING;

int MAX_LOG_FMT_LEN;
int MAX_FN_PATH_LEN;
int MAX_CMD_LEN;
const char* FNP_DELIM;
int LOG_LEVEL;
bool LOG_SIMPLE;

char *SERVER_IP;
int SERVER_PORT;
int WITH_GUI;

void reset_all_settings() {
    SOFTWARE_NAME = "CRSS";
    LONG_SOFTWARE_NAME = "C Redstone Speedup Server";
    VERSION_STRING = get_version_string();

    MAX_LOG_FMT_LEN = 256;
    MAX_FN_PATH_LEN = 224;
    MAX_CMD_LEN = 256;
    FNP_DELIM = ".";
    LOG_LEVEL = 0;
    LOG_SIMPLE = true;

    SERVER_IP = "127.0.0.1";
    SERVER_PORT = 25565;
    WITH_GUI = 1;
}

bool switch_logger_protocol(void) {
    LOG_SIMPLE = !LOG_SIMPLE;
    return LOG_SIMPLE;
}
