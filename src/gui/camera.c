#include "camera.h"

void init_camera(CrssCamera *cam) {
    cam->tx = 0;
    cam->ty = 0;
    cam->tz = 0;
    cam->azimuth = 30.0;
    cam->elevation = 7.0;
    cam->distance = 10.0;
    cam->fov = 60.0;
    cam->near = 0.1;
    cam->far = 1000.0;
    cam->aspect = 1.0;
    cam->sensitivity = 0.2;
}

void get_camera_pos_and_target(CrssCamera *cam, vec3 pos, vec3 tgt) {
    float cos_elev = cosf(glm_rad(cam->elevation));
    float sin_elev = sinf(glm_rad(cam->elevation));
    float sin_azim = sinf(glm_rad(cam->azimuth));
    float cos_azim = cosf(glm_rad(cam->azimuth));

    vec3 offset = {
        cam->distance * cos_elev * sin_azim,
        cam->distance * sin_elev,
        cam->distance * cos_elev * cos_azim
    };

    tgt[0] = (float)cam->tx;
    tgt[1] = (float)cam->ty;
    tgt[2] = (float)cam->tz;
    glm_vec3_add(tgt, offset, pos);
}

void get_view_matrix(CrssCamera *cam, mat4 dest) {
    vec3 cam_pos, cam_target;
    get_camera_pos_and_target(cam, cam_pos, cam_target);
    glm_lookat(cam_pos, cam_target, (vec3){0, 1, 0}, dest);
}

void get_projection_matrix(CrssCamera *cam, mat4 dest) {
    glm_perspective(
        glm_rad(cam->fov),
        cam->aspect,
        cam->near, cam->far,
        dest
    );
}

void camera_orbit(CrssCamera *cam, float dx, float dy) {
    cam->azimuth -= dx * cam->sensitivity;
    cam->elevation += dy * cam->sensitivity;
    if (cam->elevation > 89.0) cam->elevation = 89.0;
    if (cam->elevation < -89.0) cam->elevation = -89.0;
}
