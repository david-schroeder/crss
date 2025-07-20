#ifndef GUI_H
#define GUI_H

#include "utils.h"
#include "internals.h"

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

/*
Blocking wrapper function for GTK Application runner.

Initializes GTK Window, sets up inproc ZMQ sockets and logging,
and runs main window loop until closed or terminated via `terminate_gui()`.

@param argc GTK Application argument count
@param argv GTK Application argument string list
@return Exit code of GTK Application
*/
int run_gui(int argc, char **argv);

/*
Close GTK Application.

Performs cleanup for various GUI internals and exits the GTK application.
*/
void terminate_gui();

#endif // GUI_H
