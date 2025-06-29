#include "gui/text.h"

#include "stb_image.h"

#define MAX_RENDER_STRLEN 128

GLFWimage* load_glyph_atlas(const char* parentpath, const char* path) {
    char* fnpath = get_fn_path(parentpath, "load_glyph_atlas");

    int width, height, channels;
    unsigned char* image_data = stbi_load(path, &width, &height, &channels, 0);
    if (!image_data) {
        LWARN("Failed to load font glyph atlas!");
        free(fnpath);
        return NULL;
    }

    GLFWimage* image = (GLFWimage*)malloc(sizeof(GLFWimage));
    image->width = width;
    image->height = height;
    image->pixels = image_data;

    free(fnpath);

    return image;
}

int render_char(const char* parentpath, GLFWimage* glyph_atlas, const char chr, int x, int y, int xs, int ys) {
    int callback = 1;
    if (callback) {
        if (log_debug >= LOG_LEVEL) {
            printf("\033[F");
        }
        console_log(log_debug, "main.gui.text.render_char", "chr: %c X: %d Y: %d Xs: %d Ys: %d", chr, x, y, xs, ys);
    }
    return 0;
}

int render_text(const char* parentpath, GLFWimage* glyph_atlas, const char* text, int x, int y, int xs, int ys) {
    for (int i = 0; i<strnlen(text, MAX_RENDER_STRLEN); i++) {
        render_char(parentpath, glyph_atlas, text[i], x, y, xs, ys);
        x += xs;
    }
    return 0;
}
