#ifndef TEXT_H
#define TEXT_H

#include "utils.h"

GLFWimage* load_glyph_atlas(const char* parentpath, const char* path);

int render_text(const char* parentpath, GLFWimage* glyph_atlas, const char* text, int x, int y, int xs, int ys);

int render_char(const char* parentpath, GLFWimage* glyph_atlas, const char chr, int x, int y, int xs, int ys);

#endif // TEXT_H
