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

bool IN_TERMINAL_MODE;
char *PROMPT_STRING;

char *SERVER_IP;
int SERVER_PORT;
int WITH_GUI;

void reset_all_settings() {
    SOFTWARE_NAME = "CRSS";
    LONG_SOFTWARE_NAME = "C Redstone Speedup Server";
    VERSION_STRING = get_version_string();

    MAX_LOG_FMT_LEN = 4096;
    MAX_FN_PATH_LEN = 480;
    MAX_CMD_LEN = 256;
    FNP_DELIM = ".";
    LOG_LEVEL = 0;
    LOG_SIMPLE = true;
    IN_TERMINAL_MODE = false;
    PROMPT_STRING = malloc(64);
    sprintf(PROMPT_STRING, " [\033[32m%s\033[0m] \033[2;3m[Try 'help' or 'about'...]\033[0m > ", SOFTWARE_NAME);

    SERVER_IP = "127.0.0.1";
    SERVER_PORT = 25565;
    WITH_GUI = 1;
}

bool switch_logger_protocol(void) {
    LOG_SIMPLE = !LOG_SIMPLE;
    return LOG_SIMPLE;
}
