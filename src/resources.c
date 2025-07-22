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
    NL "    Plot Management" DIM "..........." BOLDGREEN "help plots"
    NL "  GUI" DIM "........................." BOLDGREEN "help gui"
    NS
    NL ITALIC "General Commands" NL DIM
    "+---------+-------+----------------------------+"
    NS "|" BOLDWHITE " Command " RST DIM "|" BOLDWHITE " Alias " RST DIM "|" BOLDWHITE " Description                " RST DIM "|"
    NS "+---------+-------+----------------------------+"
    NS "|" RST " help    " DIM "|" RST "  ---  " DIM "|" RST " Display this help screen   " DIM "|"
    NS "|" RST " quit    " DIM "|" RST " exit  " DIM "|" RST " Quit the application       " DIM "|"
    NS "|" RST " debug   " DIM "|" RST "  ---  " DIM "|" RST " Set the log level          " DIM "|"
    NS "+---------+-------+----------------------------+"
    NS "";

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
