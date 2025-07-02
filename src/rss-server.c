#include "rss-server.h"

// Argp options
const char* argp_program_version;
const char* argp_program_bug_address = "<post@schroederdavid.de>";
static char doc[] = "CRSS Implementation by David Schröder\nhttps://git.tu-berlin.de/schroeder_dav1d/";
static struct argp_option options[] = {
    {"debug",    'd',  "LEVEL",    0,              "Debug level for execution"     },
    {"port",     'p',  "PORT",     0,              "Port to host server on"        },
    {"address",  'a',  "ADDRESS",  0,              "Address to host server on"     },
    {"name",     'n',  "NAME",     OPTION_HIDDEN,  "The name to execute by"        },
    {"longname", 'l', "LONGNAME", OPTION_HIDDEN,  "Software name - long version"  },
    {"nogui",    'g',  0,          0,              "Do not open GUI"               },
    {0}
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;

    switch (key)
    {
    case 'd':
        if (!strcmp(arg, "verbose") || !strcmp(arg, "verb")) {
            arguments->debug = log_verbose;
        }
        else if (!strcmp(arg, "debug") || !strcmp(arg, "debg")) {
            arguments->debug = log_debug;
        }
        else if (!strcmp(arg, "info")) {
            arguments->debug = log_info;
        }
        else if (!strcmp(arg, "warn") || !strcmp(arg, "warning")) {
            arguments->debug = log_warn;
        }
        else if (!strcmp(arg, "fatal") || !strcmp(arg, "fail")) {
            arguments->debug = log_fatal;
        }
        break;
    case 'p':
        arguments->port = atoi(arg);
        break;
    case 'a':
        arguments->address = arg;
        break;
    case 'n':
        arguments->name = arg;
        break;
    case 'g':
        arguments->gui = 0;
        break;
    
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };

void parse_args(int argc, char* argv[]) {
    reset_all_settings();
    argp_program_version = VERSION_STRING; // I hate this
    // Initialize
    struct arguments arguments;
    arguments.name = SOFTWARE_NAME;
    arguments.longname = LONG_SOFTWARE_NAME;
    arguments.debug = log_info;
    arguments.address = SERVER_IP;
    arguments.port = SERVER_PORT;
    arguments.gui = 1;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Update settings
    SOFTWARE_NAME = arguments.name;
    LONG_SOFTWARE_NAME = arguments.longname;
    LOG_LEVEL = arguments.debug;
    SERVER_IP = arguments.address;
    SERVER_PORT = arguments.port;
    WITH_GUI = arguments.gui;
}


// Ctrl+c stuff
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    printf("\r");
    console_log(log_debug, "main.intHandler", "%d", dummy);
    keepRunning = 0;
    if (WITH_GUI) {
        terminateGUI();
    }
}

int return_code;

// all for one fancy line of text...
#define THREAD_WRAPPER(name, compound) \
    const char *init_fp = (const char *)fp; \
    const char *fnpath = name "_bootstrap"; \
    LVERBOSE("Launching " name " subsystem..."); \
    fnpath = init_fp; \
    compound \
    return NULL;

#define DECLARE_THREAD_WRAPPER(name, compound) \
    static void *name ## _wrapper(void *fp) { \
        THREAD_WRAPPER(#name, compound); \
    }

#define LAUNCH_WRAPPED_THREAD(name) \
    pthread_t name ## _thread; \
    pthread_create(&name ## _thread, NULL, name ## _wrapper, (void *)fnpath)

DECLARE_THREAD_WRAPPER(logger, {
    // dispatch to logger thread here
});

DECLARE_THREAD_WRAPPER(gui, {
    return_code = runGUI((char*)fnpath, 0, NULL);
    LINFO("GUI exited with code %d", return_code);
});

int main(int argc, char* argv[]) {
    const char *fnpath = "main";

    // Obtain SOFTWARE_NAME, LONG_SOFTWARE_NAME, LOG_LEVEL, SERVER_IP, SERVER_PORT and VERSION_STRING
    parse_args(argc, argv); // Also initialise settings

    char *start_time = get_time_string();
    LINFO("Launching %s at %s!", LONG_SOFTWARE_NAME, start_time);

    signal(SIGINT, intHandler);

    LINFO("=======================================================================");
    LINFO("Software version %s on host %s:%d", VERSION_STRING, SERVER_IP, SERVER_PORT);

    LDEBUG("Configuration: IP %s, PORT %d, NAME %s, DBG_LVL %d, GUI %d", SERVER_IP, SERVER_PORT, SOFTWARE_NAME, \
                LOG_LEVEL, WITH_GUI);

    crss_initialize(fnpath);
    
    LINFO("Dispatching worker threads...");
    LDEBUG("Launching GUI Thread!");

    return_code = 0;

    // logger thread available via `pthread_t logger_thread`;
    LAUNCH_WRAPPED_THREAD(logger);
    LAUNCH_WRAPPED_THREAD(gui);

    LINFO("%s version %s successfully loaded and started!", SOFTWARE_NAME, VERSION_STRING);

    LINFO("Press ENTER to terminate program.");
    char in;
    scanf("%c", &in);

    switch (return_code) {
        case 0: LINFO("Terminating normally!"); break;
        default: LFATAL("Terminating with errors!");
    }

    // Cleanup and exit
    free(VERSION_STRING);
    return return_code;
}
