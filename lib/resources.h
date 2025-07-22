#ifndef RESOURCES_H
#define RESOURCES_H

/*
Collection of resources, specifically long strings such as help messages.
*/

/*
Message for general command help.

Format arguments:
- Software name
- Version string
*/
extern const char *RESOURCE_HELP_GENERAL;

/*
Fancy logo.

Extremely cool.
*/
extern const char *RESOURCE_SPLASH;

/*
About message.

Roughly amounts to name and version info, copyright, licensing and contact information.

Format arguments:
- Splash Resource
- Long software name
- Version String
- (Copyright) Year
- Software name
*/
extern const char *RESOURCE_ABOUT;

#endif // RESOURCES_H
