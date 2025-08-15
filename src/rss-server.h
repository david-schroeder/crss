#ifndef RSS_SERVER_H
#define RSS_SERVER_H

#include "version.h"
#include "utils.h"
#include "settings.h"
#include "internals.h"
#include "resources.h"

#include "core/core.h"
#include "gui/gui.h"
#include "logger/logger.h"
#include "logger/help.h"
#include "network/network.h"


struct arguments {
    int debug;
    int port;
    char *address;
    char *name;
    char *longname;
    int gui;
};

/*
Parses program arguments and sets global setting variables.

NOTE: This should be marked as deprecated and replaced with handling
to integrate more closely with GTK's argument system.

@param argc Argument count
@param argv String argument list
*/
void parse_args(int argc, char* argv[]);

/*
Ctrl+C Interrupt handler

@param dummy System interrupt code
*/
void intHandler(int dummy);

/*
Global program entry point.

Initializes program and inproc sockets, launches subsystem threads,
and runs command console in main thread until exited.

The subsystems that are launched are:
- GUI thread, if GUI is enabled or selected via command-line arguments
- Command broker thread, which each subsystem can dispatch commands to
- Master network thread
- Master core thread
(Where "master" implies the launch of further threads by the master)
*/
int main(int argc, char* argv[]);

#endif // RSS_SERVER_H
