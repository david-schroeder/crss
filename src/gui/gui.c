#include "gui.h"

/////////////////////
// CRSS APP WINDOW //
/////////////////////

struct _CrssAppWindow {
    GtkApplicationWindow parent;
    GtkWidget *console_text_view;
    GtkWidget *console_command_entry;
    GtkWidget *graph_area; // GLArea
};
G_DEFINE_TYPE(CrssAppWindow, crss_app_window, GTK_TYPE_APPLICATION_WINDOW);

// Forward declaration
void issue_console_command_callback(GtkEntry *entry, CrssAppWindow *win);

static void crss_app_window_init(CrssAppWindow *win) {
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void crss_app_window_class_init(CrssAppWindowClass *cls) {
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(cls), "/de/redsynth/crss/window.ui");
    #define ATTACH_ELEMENT(name) gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(cls), CrssAppWindow, name)
    #define BIND_CALLBACK(name) gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(cls), name)
    ATTACH_ELEMENT(console_text_view);
    ATTACH_ELEMENT(console_command_entry);
    ATTACH_ELEMENT(graph_area);
    BIND_CALLBACK(issue_console_command_callback);
}

CrssAppWindow *crss_app_window_new(CrssApp *app) {
    return g_object_new(CRSS_APP_WINDOW_TYPE, "application", app, NULL);
}

void crss_app_window_open(CrssAppWindow *win, GFile *file) {}

//////////////
// CRSS APP //
//////////////

struct _CrssApp {
    GtkApplication parent;
};
G_DEFINE_TYPE(CrssApp, crss_app, GTK_TYPE_APPLICATION);

static void crss_app_init(CrssApp *app) {}

CrssAppWindow *WINDOW;

static void crss_app_activate(GApplication *app) {
    GUI_PATH("activate");
    LVERBOSE("Constructing window...");
    CrssAppWindow *win;
    win = crss_app_window_new(CRSS_APP(app));
    WINDOW = win;

    if (!win->console_text_view || !win->console_command_entry || !win->graph_area) {
        LFATAL("Failed to construct GUI!");
        exit(1);
    }

    gtk_window_present(GTK_WINDOW(win));
}

static void crss_app_class_init(CrssAppClass *cls) {
    G_APPLICATION_CLASS(cls)->activate = crss_app_activate;
}

CrssApp *crss_app_new(void) {
    return g_object_new(CRSS_APP_TYPE, "application-id", "de.redsynth.crss",
                        "flags", G_APPLICATION_DEFAULT_FLAGS, NULL);
}

///////////////
// CALLBACKS //
///////////////

void issue_console_command_callback(GtkEntry *entry, CrssAppWindow *win) {
    GUI_PATH("console.cmd_callback");

    GtkEntryBuffer *cmdBuf = gtk_entry_get_buffer(entry);

    // get buffer contents + length
    const gchar *cmd = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(cmdBuf));
    guint cmd_len = gtk_entry_buffer_get_length(GTK_ENTRY_BUFFER(cmdBuf));

    if (cmd_len == 0) return;

    // process command...
    // TODO: emit ZMQ packet here
    LDEBUG("Issued console command '%s'!", cmd);

    GtkTextBuffer *console_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->console_text_view));
    GtkTextIter console_end_iter;
    gtk_text_buffer_get_end_iter(console_buf, &console_end_iter);
    gtk_text_buffer_insert(console_buf, &console_end_iter, cmd, -1);
    gtk_text_iter_forward_to_end(&console_end_iter);
    gtk_text_buffer_insert(console_buf, &console_end_iter, "\n", 1);

    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(win->console_text_view), &console_end_iter, 0.25, false, 0.0, 0.0);

    // clear buffer
    gtk_entry_buffer_delete_text(GTK_ENTRY_BUFFER(cmdBuf), 0, -1);
}

////////////////////
// MAIN FUNCTIONS //
////////////////////

void terminateGUI() {
    gtk_window_close(GTK_WINDOW(WINDOW));
}

int runGUI(char *fnpath, int argc, char **argv) {
    FUNCPATH("initGUI");

    LINFO("Initializing GUI!");

    LVERBOSE("Setting up sockets...");

    LVERBOSE("Starting GTK app...");

    int status = g_application_run(G_APPLICATION(crss_app_new()), argc, argv);
    free(fnpath);

    return status;
}
