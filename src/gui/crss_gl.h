#ifndef CRSS_GL_H
#define CRSS_GL_H

#include "gtk/gtk.h"
#include "utils.h"
#include "gui.h"

#include <epoxy/gl.h>

gboolean render_graph_area(GtkGLArea *area, GdkGLContext *ctx, gpointer user_data);

#endif // CRSS_GL_H
