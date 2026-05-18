#include "crss_gl.h"

gboolean render_graph_area(GtkGLArea *area, GdkGLContext *ctx, gpointer user_data) {

    CrssAppWindow *win = user_data;

    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
}
