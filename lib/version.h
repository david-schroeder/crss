#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#define VERSION_MAJOR   0
#define VERSION_MINOR   0
#define VERSION_MICRO   0
#define IS_SNAPSHOT     1
#define SNAPSHOT_VER    1

/*
Generates a version string such as "v1.2.3" using the version infos
#defined above.
@return a char* to a string which should be freed with free(), NULL on calloc fail
*/
char* get_version_string();

#endif // VERSION_H
