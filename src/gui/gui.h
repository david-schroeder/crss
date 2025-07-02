#ifndef GUI_H
#define GUI_H

#include "utils.h"

#include "gtk/gtk.h"

#define CRSS_APP_TYPE (crss_app_get_type())
// no clue how this works but eh *shrug*
G_DECLARE_FINAL_TYPE(CrssApp, crss_app, CRSS, APP, GtkApplication)

CrssApp *crss_app_new(void);

#define CRSS_APP_WINDOW_TYPE (crss_app_window_get_type())
G_DECLARE_FINAL_TYPE(CrssAppWindow, crss_app_window, CRSS_APP, APP_WINDOW, GtkApplicationWindow)

CrssAppWindow *crss_app_window_new(CrssApp *app);
void crss_app_window_open(CrssAppWindow *win, GFile *file);

#define GUI_PATH(name) const char *fnpath = "gui." name

int runGUI(char *fnpath, int argc, char **argv);

void terminateGUI();

#endif // GUI_H
