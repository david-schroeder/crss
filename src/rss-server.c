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
    console_log(log_debug, "main.intHandler", "Goodbye!", dummy);
    keepRunning = 0;
    dispatch_command("exit");
}

int return_code;

DECLARE_THREAD_WRAPPER(logger, {
    // dispatch to logger thread here
    // that is, this code will be run in a new thread
    int rc = run_logger(2);
    LDEBUG("Logger exited with code %d", rc);
});

DECLARE_THREAD_WRAPPER(gui, {
    return_code = run_gui(0, NULL);
    LDEBUG("GUI exited with code %d", return_code);
});

int main(int argc, char* argv[]) {
    const char *fnpath = "main";

    reset_all_settings();

    // Obtain SOFTWARE_NAME, LONG_SOFTWARE_NAME, LOG_LEVEL, SERVER_IP, SERVER_PORT and VERSION_STRING
    parse_args(argc, argv);

    char *start_time = get_time_string();
    LINFO("Launching %s at %s!", LONG_SOFTWARE_NAME, start_time);

    signal(SIGINT, intHandler);

    LINFO("=======================================================================");
    LINFO("Software version %s on host %s:%d", VERSION_STRING, SERVER_IP, SERVER_PORT);

    LDEBUG("Configuration: IP %s, PORT %d, NAME %s, DBG_LVL %d, GUI %d", SERVER_IP, SERVER_PORT, SOFTWARE_NAME, \
                LOG_LEVEL, WITH_GUI);

    LVERBOSE("Initializing!");

    init_resources();

    // create ZMQ context
    crss_initialize(fnpath);

    LINFO("Dispatching worker threads...");

    LDEBUG("Launching Logger Thread!");
    // logger thread available via `pthread_t logger_thread`;
    LAUNCH_WRAPPED_THREAD(logger);

    LDEBUG("Launching GUI Thread!");

    return_code = 0;

    LAUNCH_WRAPPED_THREAD(gui);
    
    CONNECT_TO_CMD_BROADCAST();
    SUBSCRIBE_TO_CMD("exit");
    SUBSCRIBE_TO_CMD("quit");
    SUBSCRIBE_TO_CMD("help");
    SUBSCRIBE_TO_CMD("debug");
    SUBSCRIBE_TO_CMD("about");
    logger_notify("Terminal");

    while (LOG_SIMPLE && keepRunning) usleep(1000); // Wait for logger to activate

    LINFO("%s version %s successfully loaded and started!", SOFTWARE_NAME, VERSION_STRING);

    IN_TERMINAL_MODE = true;
    LINFO("Type 'exit' to terminate program, 'about' for information about the project or 'help' for a list of commands.");
    char buf[MAX_CMD_LEN];
    buf[MAX_CMD_LEN-1] = '\0';
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    RUN_CMD_HANDLER({
        usleep(1000);
        int numRead = read(0, buf, MAX_CMD_LEN-1);
        if (numRead > 0) {
            buf[numRead-1] = '\0'; // overwrite \n with \0
            dispatch_command(buf);
        }
    }, {
        HANDLE_COMMAND("exit", {
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("quit", {
            EXIT_CMD_HANDLER();
        })
        else HANDLE_COMMAND("help", {
            LINFO(RESOURCE_HELP_GENERAL, SOFTWARE_NAME, VERSION_STRING);
        })
        HANDLE_COMMAND("debug verbose"     , { LOG_LEVEL = log_verbose; })
        else HANDLE_COMMAND("debug verb"   , { LOG_LEVEL = log_verbose; })
        else HANDLE_COMMAND("debug v"      , { LOG_LEVEL = log_verbose; })
        else HANDLE_COMMAND("debug debug"  , { LOG_LEVEL = log_debug; })
        else HANDLE_COMMAND("debug debg"   , { LOG_LEVEL = log_debug; })
        else HANDLE_COMMAND("debug d"      , { LOG_LEVEL = log_debug; })
        else HANDLE_COMMAND("debug info"   , { LOG_LEVEL = log_info; })
        else HANDLE_COMMAND("debug i"      , { LOG_LEVEL = log_info; })
        else HANDLE_COMMAND("debug warning", { LOG_LEVEL = log_warn; })
        else HANDLE_COMMAND("debug warn"   , { LOG_LEVEL = log_warn; })
        else HANDLE_COMMAND("debug w"      , { LOG_LEVEL = log_warn; })
        else HANDLE_COMMAND("debug fatal"  , { LOG_LEVEL = log_fatal; })
        else HANDLE_COMMAND("debug fail"   , { LOG_LEVEL = log_fatal; })
        else HANDLE_COMMAND("debug f"      , { LOG_LEVEL = log_fatal; })
        else HANDLE_COMMAND("debug", {
            if (strlen(RECEIVED_CMD) > 6) {
                LFATAL("debug: '%s' is not a valid log level!", &RECEIVED_CMD[6]);
            } else {
                LFATAL("debug: Please provide a log level.");
            }
        })
    })

    IN_TERMINAL_MODE = false;

    CMD_HANDLER_CLEANUP();

    DLINFO("Joining worker threads...");

    JOIN_WRAPPED_THREAD(gui);
    DLDEBUG("Joined GUI thread!");
    JOIN_WRAPPED_THREAD(logger);
    DLDEBUG("Joined Logger thread!");

    LINFO("All threads joined!");


    switch (return_code) {
        case 0: LINFO("Terminating normally!"); break;
        default: LFATAL("Terminating with errors!");
    }

    // Cleanup and exit
    free(VERSION_STRING);
    zmq_ctx_term(crss_zmq_ctx());

    return return_code;
}
