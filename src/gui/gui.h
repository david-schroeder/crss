#ifndef GUI_H
#define GUI_H

#include "utils.h"

#include "gtk/gtk.h"

#define GUI_PATH(name) const char *fnpath = "gui." name

int runGUI(char *fnpath, int argc, char **argv);

void terminateGUI();

#endif // GUI_H
