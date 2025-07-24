#include "crss_gl.h"

gboolean render_graph_area(GtkGLArea *area, GdkGLContext *ctx) {

    glClearColor(0.3, 0.1, 0.6, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
}
