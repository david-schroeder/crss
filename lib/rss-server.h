#ifndef RSS_SERVER_H
#define RSS_SERVER_H

#include "version.h"
#include "utils.h"
#include "settings.h"

#include "gui/window.h"


struct arguments {
    int debug;
    int port;
    char* address;
    char* name;
    int gui;
};

void parse_args(int argc, char* argv[]);

void intHandler(int dummy);

int main(int argc, char* argv[]);

#endif // RSS_SERVER_H
