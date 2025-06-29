#ifndef WINDOW_H
#define WINDOW_H

#include "utils.h"
#include "gui/text.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

GLFWimage* load_icon(const char* fnpath, const char* path);

GLFWwindow* initGUI(const char* parent_path);

void window_tick(const char* parentpath, GLFWwindow* window);

void window_terminate(const char* parentpath);

int windowMain(const char* parentpath);

void window_draw(const char* parentpath, GLFWwindow* window);

void draw_diamond(const char* parentpath, GLFWwindow* window, int x, int y, int scale);

#endif // WINDOW_H