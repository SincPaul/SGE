//
// Created by Geisthardt on 11.03.2025.
//

#ifndef SGE_CAMERA_H
#define SGE_CAMERA_H

#include "../utils/sge_math.h"

typedef struct sge_camera {
        vec3 position; //x,   y,     z
        vec3 rotation; //yaw, pitch, roll
        float fov;
        float speed;
} sge_camera;

#include "sge_render.h"


typedef struct sge_uniform_buffer_object {
        m4 model;
        m4 view;
        m4 proj;
} sge_uniform_buffer_object;


SGE_RESULT sge_camera_move_forward(sge_render *render);
SGE_RESULT sge_camera_move_left(sge_render *render);
SGE_RESULT sge_camera_move_right(sge_render *render);
SGE_RESULT sge_camera_move_backwards(sge_render *render);
SGE_RESULT sge_camera_move_down(sge_render *render);
SGE_RESULT sge_camera_move_up(sge_render *render);
SGE_RESULT sge_camera_rotate(sge_render *render);
SGE_RESULT sge_camera_rotate_x(sge_render *render, float angle);
SGE_RESULT sge_camera_rotate_y(sge_render *render, float angle);
SGE_RESULT sge_camera_rotate_z(sge_render *render, float angle);

SGE_RESULT sge_update_uniform_buffer(sge_render *render);


#endif //SGE_CAMERA_H
