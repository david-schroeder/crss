#include "resources.h"

#define RST "\033[0m"
#define NS "\n "
#define NL NS RST
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define ITALIC "\033[3m"
#define BOLDWHITE "\033[0;1m"
#define BOLDGREEN "\033[32;1m"
#define BOLDRED "\033[31;1m"

const char *RESOURCE_HELP_GENERAL = "\n"
    NS BOLD "Command Help for %s %s"
    NL DIM "Type 'help <command>' for more information on any specific command, if available" NL
    NS ITALIC "Command Categories"
    NL "  RedSynth" DIM "...................." BOLDGREEN "help redsynth"
    NL "    Graph optimization" DIM "........" BOLDGREEN "help graph"
    NL "  Minecraft Server" DIM "............" BOLDGREEN "help core"
    NL "    Networking" DIM "................" BOLDGREEN "help network"
    NL "    Schematics" DIM "................" BOLDGREEN "help schems"
    NL "    Plot Management" DIM "..........." BOLDGREEN "help plot"
    NL "  GUI" DIM "........................." BOLDGREEN "help gui"
    NS;

const char *RESOURCE_SPLASH = "\n"
    NS "                          ┌────────────────────────────────────┐"
    NS "                          │ " BOLDRED "  _____ " RST "   _____   ______  ______  │"
    NS "                          │ " BOLDRED " /  _  \\ " RST " |  _  \\ /  ___/ /  ___/  │"
    NS "                          │ " BOLDRED " | | |_| " RST " | |_| | | |___  | |___   │"
    NS "                          │ " BOLDRED " | |  _  " RST " |    _/ \\___  \\ \\___  \\  │"
    NS "                          │ " BOLDRED " | |_| | " RST " | |\\ \\   ___| |  ___| |  │"
    NS "                          │ " BOLDRED " \\_____/ " RST " |_| \\_\\ /_____/ /_____/  │"
    NL "                          │                                    │"
    NS "                          └────────────────────────────────────┘"
    NS
    NL "[" DIM "An Application for optimizing and compiling Minecraft redstone circuits to SystemVerilog" RST "]"
    NS
    NL "                                         ────────"
    NS;

const char *RESOURCE_ABOUT = "\n"
    "%s"
    NL BOLD "Version"
    NL "%s is currently running on version \033[31m%s."
    NS
    NL BOLD "Copyright"
    NL "© %s David Schröder. All rights reserved."
    NS
    NL BOLD "License"
    NL "%s is licensed under \033[31mCC BY-NC-SA 4.0."
    NS
    NL BOLD "Contact information"
    NL "Website: " RST DIM "https://www.schroederdavid.de/"
    NL "E-Mail: " RST DIM "post@schroederdavid.de"
    NL "Report Bugs: " RST DIM "post@schroederdavid.de"
    NS;

const char *DEFAULT_STATUS_JSON_RESPONSE = "{"
    "    \"version\": {"
    "        \"name\": \"1.18.2\","
    "        \"protocol\": 758"
    "    },"
    "    \"players\": {"
    "        \"max\": 9999,"
    "        \"online\": 0,"
    "        \"sample\": []"
    "    },"
    "    \"description\": {"
    "        \"text\": \"C Redstone Speedup Server\""
    "    },"
    "    \"favicon\": \"data:image/png;base64,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\","
    "    \"enforcesSecureChat\": false"
    "}";
