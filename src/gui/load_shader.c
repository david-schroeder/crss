#include "load_shader.h"

/*
 * Loads a file into a heap-allocated, null-terminated buffer.
 *
 * @param path Path to file
 *
 * @return In-memory string containing file contents
 *   (must be freed by caller) or NULL on failure
 */
char *load_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, (size_t)size, f);
    fclose(f);

    if (read_size != (size_t)size) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0'; // null-terminate

    return buffer;
}
