#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"

#include <cglm/cglm.h>

typedef struct {
    uint16_t tx, ty, tz;
    float azimuth;
    float elevation;
    float distance;
    float aspect;
    float fov;
    float near;
    float far;
    float sensitivity;
} CrssCamera;

void init_camera(CrssCamera *cam);

void get_camera_pos(CrssCamera *cam, vec3 tgt);

void get_camera_pos_and_target(CrssCamera *cam, vec3 pos, vec3 tgt);

void get_view_matrix(CrssCamera *cam, mat4 dest);

void get_projection_matrix(CrssCamera *cam, mat4 dest);

void camera_orbit(CrssCamera *cam, float dx, float dy);

#endif // CAMERA_H
