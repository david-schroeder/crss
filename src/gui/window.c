#include "gui/window.h"

// ¯\_(ツ)_/¯
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int log_callbacks = 0;

static GLFWimage* font_atlas;
static unsigned int font_texid;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (log_callbacks) {
        if (log_debug >= LOG_LEVEL) {
            printf("\033[F");
        }
        console_log(log_debug, "main.gui.key_callback", "Key callback with key %d, scancode %d, action %d and mods %d          ", key, scancode, action, mods);
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWimage* load_icon(const char* parentpath, const char* path) {
    char* fnpath = get_fn_path(parentpath, "load_icon");

    int width, height, channels;
    unsigned char* image_data = stbi_load(path, &width, &height, &channels, 0);
    if (!image_data) {
        LWARN("Failed to load icon!");
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

GLFWwindow* initGUI(const char* parentpath) {
    char* fnpath = get_fn_path(parentpath, "gui.init");
    LVERBOSE("Initialising GUI!");

    if (!glfwInit()) {
        LFATAL("Failed to initialise GLFW!");
        return NULL;
    }

    LVERBOSE("Creating Window!");
    GLFWwindow* window = glfwCreateWindow(800, 600, SOFTWARE_NAME, NULL, NULL);
    if (!window) {
        LFATAL("Failed to create window!");
        glfwTerminate();
        return NULL;
    }

    LVERBOSE("Loading image assets!");
    GLFWimage* icon = load_icon(fnpath, "server/icon.png");
    if (icon) {
        // Set the icon for the window
        glfwSetWindowIcon(window, 1, icon);

        // Free the icon image data
        stbi_image_free(icon->pixels);
        free(icon);
    }

    GLFWimage* font = load_glyph_atlas(fnpath, "src/gui/glyph_atlas.png");

    unsigned int texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, font->width, font->height, 0, GL_RGB, GL_UNSIGNED_BYTE, font->pixels);

    font_atlas = font;
    font_texid = texid;

    LVERBOSE("Initialising GLFW!");

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.4f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    free(fnpath);

    LVERBOSE("GUI initialised!");

    return window;
}

void window_tick(const char* parentpath, GLFWwindow* window) {
    char* fnpath = get_fn_path(parentpath, "gui.tick");
    //render
    window_draw(fnpath, window);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
    free(fnpath);
}

void window_terminate(const char* parentpath) {
    char* fnpath = get_fn_path(parentpath, "gui.terminate");
    
    LVERBOSE("Terminating GLFW window!");
    glfwTerminate();

    stbi_image_free(font_atlas->pixels);
    free(font_atlas);

    free(fnpath);
}

void window_draw(const char* parentpath, GLFWwindow* window) {
    // Set up the viewport (could be done once if the window size is fixed)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Clear the buffer
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up the orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1); // left, right, bottom, top, near, far

    // Draw your 2D shapes here using OpenGL drawing functions (e.g., glBegin, glVertex2f, glEnd)
    // Or, if you prefer modern OpenGL, you can use shaders, VBOs, and VAOs
    draw_diamond(parentpath, window, 200, 300, 50);

    render_text(parentpath, font_atlas, "blah blah", 220, 320, 10, 10);
}

void draw_diamond(const char* parentpath, GLFWwindow* window, int x, int y, int scale) {
    int x1, x2, x3, x4, y1, y2, y3, y4;
    x1 = x;
    x2 = x+scale;
    x3 = x;
    x4 = x-scale;
    y1 = y-scale*0.5f;
    y2 = y;
    y3 = y+scale*0.5f;
    y4 = y;

    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.4f, 0.4f);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glVertex2f(x4, y4);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.8f, 0.8f, 0.8f);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glVertex2f(x3, y3);
    glVertex2f(x4, y4);
    glVertex2f(x4, y4);
    glVertex2f(x1, y1);
    glEnd();
}
