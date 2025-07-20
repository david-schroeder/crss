#include "resources.h"

const char *RESOURCE_HELP_GENERAL;

void init_resources(void) {
    RESOURCE_HELP_GENERAL = ""
    "\n\n\033[1m%s Command Help for Version %s\033[0m"
    "\n\033[2mType 'help <command>' for more information on any specific command, if available\033[0m\n"
    "\n\033[3mCommand Categories\033[0m"
    "\n\033[0m  RedSynth\033[2m....................\033[32;1mhelp redsynth"
    "\n\033[0m    Graph optimization\033[2m........\033[32;1mhelp graph"
    "\n\033[0m  Minecraft Server\033[2m............\033[32;1mhelp core"
    "\n\033[0m    Networking\033[2m................\033[32;1mhelp network"
    "\n\033[0m    Schematics\033[2m................\033[32;1mhelp schems"
    "\n\033[0m    Plot Management\033[2m...........\033[32;1mhelp plots"
    "\n\033[0m  GUI\033[2m.........................\033[32;1mhelp gui"
    "\n\033[0m"
    "\n\033[3mGeneral Commands\033[2m"
    "\n+---------+-------+----------------------------+"
    "\n|\033[0;1m Command \033[2m|\033[0;1m Alias \033[2m|\033[0;1m Description                \033[2m|"
    "\n+---------+-------+----------------------------+"
    "\n|\033[0m help    \033[2m|\033[0m  ---  \033[2m|\033[0m Display this help screen   \033[2m|"
    "\n|\033[0m quit    \033[2m|\033[0m exit  \033[2m|\033[0m Quit the application       \033[2m|"
    "\n|\033[0m debug   \033[2m|\033[0m  ---  \033[2m|\033[0m Set the log level          \033[2m|"
    "\n+---------+-------+----------------------------+"
    "\n\033[0m";
}
