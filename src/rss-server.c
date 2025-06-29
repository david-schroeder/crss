#include "rss-server.h"

// Argp options
const char* argp_program_version;
const char* argp_program_bug_address = "<post@schroederdavid.de>";
static char doc[] = "CRSS Implementation by David Schröder\nhttps://git.tu-berlin.de/schroeder_dav1d/";
static struct argp_option options[] = {
    {"debug",   'd', "LEVEL",   0,              "Debug level for execution"     },
    {"port",    'p', "PORT",    0,              "Port to host server on"        },
    {"address", 'a', "ADDRESS", 0,              "Address to host server on"     },
    {"name",    'n', "NAME",    OPTION_HIDDEN,  "The name to execute by"        },
    {"nogui",   'g', 0,         0,              "Do not open GUI"               },
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
    arguments.debug = log_info;
    arguments.address = SERVER_IP;
    arguments.port = SERVER_PORT;
    arguments.gui = 1;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Update settings
    SOFTWARE_NAME = arguments.name;
    LOG_LEVEL = arguments.debug;
    SERVER_IP = arguments.address;
    SERVER_PORT = arguments.port;
    WITH_GUI = arguments.gui;
}


// Ctrl+c stuff
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    printf("\r");
    console_log(log_debug, "main.intHandler", "%d\n", dummy);
    keepRunning = 0;
}


// Running variables
GLFWwindow* window;

int main(int argc, char* argv[]) {

    const char* fnpath = "main"; // Function path

    signal(SIGINT, intHandler);

    // Obtain SOFTWARE_NAME, LOG_LEVEL, SERVER_IP, SERVER_PORT and VERSION_STRING
    parse_args(argc, argv); // Also initialise settings

    LVERBOSE("Vocalizing...");
    LDEBUG("Squashing the bugs!");
    LINFO("Ready to start!");
    LWARN("Look up, your crown is falling!");
    LFATAL("Oopsie :[");

    LINFO("=======================================================================");
    LINFO("Running Server version %s on host %s:%d", VERSION_STRING, SERVER_IP, SERVER_PORT);

    LDEBUG("Configuration: IP %s, PORT %d, NAME %s, DBG_LVL %d, GUI %d", SERVER_IP, SERVER_PORT, SOFTWARE_NAME, \
                LOG_LEVEL, WITH_GUI);

    if (WITH_GUI) {
        window = initGUI(fnpath);

        if (window == NULL) {
            free(VERSION_STRING);
            return 0;
        }
    }

    while (keepRunning) {
        if (WITH_GUI) {
            if (glfwWindowShouldClose(window)) {
                // Close window
                keepRunning = 0;
                break;
            }
            window_tick(fnpath, window);
        }
    }

    if (WITH_GUI) {
        window_terminate(fnpath);
    }

    LINFO("Terminating normally!");

    // Cleanup and exit
    free(VERSION_STRING);
    return 0;
}
