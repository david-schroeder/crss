#include "version.h"

char* get_version_string() {
    char* s = (char*)calloc(64, sizeof(char));
    if (s == NULL) {
        return NULL;
    }
    if (IS_SNAPSHOT) {
        snprintf(s, 64, "v%d.%d.%d-snapshot-%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO, SNAPSHOT_VER);
    }
    else {
        snprintf(s, 64, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    }
    return s;
}
