#include "gui.h"

GtkWindow *WINDOW;

static void activate(GtkApplication *app, gpointer user_data) {
    GUI_PATH("activate");

    LVERBOSE("Building UI...");

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "src/gui/builder.ui", NULL);

    GObject *window = gtk_builder_get_object(builder, "window");
    WINDOW = GTK_WINDOW(window);
    gtk_window_set_application(GTK_WINDOW(window), app);

    gtk_window_set_title(GTK_WINDOW(window), "CRSS Master");
    //gtk_window_set_default_size(GTK_WINDOW(window), 450, 450);

    gtk_widget_set_visible(GTK_WIDGET(window), true);

    g_object_unref(builder);

    LVERBOSE("Finished building UI!");
}

void terminateGUI() {
    gtk_window_close(WINDOW);
}

int runGUI(char *fnpath, int argc, char **argv) {
    FUNCPATH("initGUI");

    LINFO("Initializing GUI!");

    GtkApplication *app;
    int status;

    app = gtk_application_new("de.redsynth.crss", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    free(fnpath);
    return status;
}
