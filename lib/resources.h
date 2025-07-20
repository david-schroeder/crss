#ifndef RESOURCES_H
#define RESOURCES_H

/*
Collection of resources, specifically long strings such as help messages.
*/

/*
Message for general command help.

Format arguments: Software name, version string
*/
extern const char *RESOURCE_HELP_GENERAL;

void init_resources(void);

#endif // RESOURCES_H
