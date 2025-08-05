#include "gui.h"
#include "crss_gl.h"

/////////////////////
// CRSS APP WINDOW //
/////////////////////

struct _CrssAppWindow {
    GtkApplicationWindow parent;
    GtkWidget *console_text_view;
    GtkWidget *console_command_entry;
    GtkWidget *console_text_wrap_button;
    GtkWidget *graph_area; // GLArea
};
G_DEFINE_TYPE(CrssAppWindow, crss_app_window, GTK_TYPE_APPLICATION_WINDOW);

// Forward declarations
void issue_console_command_callback(GtkEntry *entry, CrssAppWindow *win);
void toggle_console_wrap(GtkCheckButton *button, CrssAppWindow *win);

static void crss_app_window_init(CrssAppWindow *win) {
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void crss_app_window_class_init(CrssAppWindowClass *cls) {
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(cls), "/de/redsynth/crss/window.ui");
    #define ATTACH_ELEMENT(name) gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(cls), CrssAppWindow, name)
    #define BIND_CALLBACK(name) gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(cls), name)
    ATTACH_ELEMENT(console_text_view);
    ATTACH_ELEMENT(console_command_entry);
    ATTACH_ELEMENT(console_text_wrap_button);
    ATTACH_ELEMENT(graph_area);
    BIND_CALLBACK(issue_console_command_callback);
    BIND_CALLBACK(render_graph_area);
    BIND_CALLBACK(toggle_console_wrap);
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

CrssAppWindow *WINDOW;
CrssApp *APP;

static void crss_app_init(CrssApp *app) {
    APP = app;
}

static void crss_app_activate(GApplication *app) {
    GUI_PATH("activate");
    LVERBOSE("Constructing window...");
    CrssAppWindow *win;
    win = crss_app_window_new(CRSS_APP(app));
    WINDOW = win;

    if (!win->console_text_view || !win->console_command_entry || !win->graph_area) {
        LFATAL("Failed to construct GUI!");
        dispatch_command("exit");
    } else {
        gtk_window_present(GTK_WINDOW(win));
    }
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
    GUI_PATH("console.callback");

    GtkEntryBuffer *cmdBuf = gtk_entry_get_buffer(entry);

    // get buffer contents + length
    const gchar *cmd = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(cmdBuf));
    guint cmd_len = gtk_entry_buffer_get_length(GTK_ENTRY_BUFFER(cmdBuf));

    if (cmd_len == 0) return;

    GtkTextBuffer *console_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->console_text_view));
    GtkTextIter console_end_iter;
    gtk_text_buffer_get_end_iter(console_buf, &console_end_iter);
    gtk_text_buffer_insert(console_buf, &console_end_iter, cmd, -1);
    gtk_text_iter_forward_to_end(&console_end_iter);
    gtk_text_buffer_insert(console_buf, &console_end_iter, "\n", 1);

    // process command...
    LDEBUG("GUI issued console command '%s'!", cmd);
    dispatch_command((char *)cmd);

    // clear buffer
    gtk_entry_buffer_delete_text(GTK_ENTRY_BUFFER(cmdBuf), 0, -1);
}

void toggle_console_wrap(GtkCheckButton *button, CrssAppWindow *win) {
    GUI_PATH("console.callback");

    gboolean do_wrap = gtk_check_button_get_active(button);
    if (do_wrap) LVERBOSE("Toggled console text wrap on!");
    else LVERBOSE("Toggled console text wrap off!");

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(win->console_text_view), do_wrap ? GTK_WRAP_WORD : GTK_WRAP_NONE);
}

///////////////////
// CONFIGURATION //
///////////////////

static bool _GUI_FORMATTED_LOG = false;

////////////////////
// MAIN FUNCTIONS //
////////////////////

static gboolean __terminate_gui_inner() {
    DLDEBUG("Quitting GTK Application!");
    gtk_widget_unrealize(GTK_WIDGET(WINDOW->graph_area));
    gtk_window_close(GTK_WINDOW(WINDOW));
    g_application_quit(G_APPLICATION(APP));
    return G_SOURCE_REMOVE;
}

void terminate_gui() {
    DLVERBOSE("Scheduling GTK Application Quit!");
    g_idle_add((GSourceFunc)__terminate_gui_inner, NULL);
}

static char *strip_format_chars(char *data) {
    size_t nchars = strlen(data);
    char *stripped = malloc(nchars+1);
    size_t sptr = 0; // stripped pointer
    bool wait_for_m = false; // A format escape sequence ends in m (e.g. \033[37;1m)
    for (int i = 0; i < nchars; i++) {
        char chr = data[i];
        if (chr == '\033') wait_for_m = true;
        if (!wait_for_m) {
            stripped[sptr++] = chr;
        } else if (chr == 'm') wait_for_m = false;
    }
    stripped[sptr] = '\0';
    free(data);
    return stripped;
}

static gboolean gui_console_add(char *message) {
    if (!_GUI_FORMATTED_LOG) {
        message = strip_format_chars(message);
    }

    GtkTextBuffer *console_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(WINDOW->console_text_view));
    
    GtkTextIter console_end_iter;
    gtk_text_buffer_get_end_iter(console_buf, &console_end_iter);

    gtk_text_buffer_insert(console_buf, &console_end_iter, &message[4], -1);
    
    gtk_text_iter_forward_to_end(&console_end_iter);
    //gtk_text_buffer_insert(console_buf, &console_end_iter, "\n", 1);
    
    GtkTextMark *mark = gtk_text_buffer_create_mark(console_buf, NULL, &console_end_iter, false);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(WINDOW->console_text_view), mark);

    free(message);
    return G_SOURCE_REMOVE;
}

static void gui_cmd_handler(char *fnpath) {
    FUNCPATH("cmd_handler");

    LVERBOSE("Setting up sockets...");

    CONNECT_TO_CMD_BROADCAST();
    SUBSCRIBE_TO_CMD("gui");
    SUBSCRIBE_TO_CMD("exit");
    SUBSCRIBE_TO_CMD("quit");
    SUBSCRIBE_TO_CMD("log");

    LVERBOSE("Notifying Logger...");
    logger_notify("GUI");

    LVERBOSE("Waiting for window to be created...");
    while (!WINDOW);

    LVERBOSE("Entering command park loop...");
    RUN_CMD_HANDLER({}, {
        HANDLE_COMMAND("exit", {
            terminate_gui();
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("quit", {
            terminate_gui();
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("log ", {
            char *msg = mystrdup(RECEIVED_CMD);
            g_idle_add((GSourceFunc)gui_console_add, msg);
        })
    })

    CMD_HANDLER_CLEANUP();

    free(fnpath);
}

DECLARE_THREAD_WRAPPER(gui_cmd_handler, {gui_cmd_handler((char *)fnpath);})

int run_gui(int argc, char **argv) {
    const char *fnpath = "gui.init";
    LDEBUG("Initializing GUI!");

    LAUNCH_WRAPPED_THREAD(gui_cmd_handler);

    LVERBOSE("Starting GTK app...");
    CrssApp *app = crss_app_new();
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    WITH_GUI = 0;
    LVERBOSE("Exited GTK app!");

    JOIN_WRAPPED_THREAD(gui_cmd_handler);
    LVERBOSE("Command handler thread joined!");

    return status;
}
