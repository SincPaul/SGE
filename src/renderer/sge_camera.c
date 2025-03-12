//
// Created by Geisthardt on 11.03.2025.
//

#include "sge_camera.h"


SGE_RESULT sge_camera_move_forward(sge_render *render) {
        vec3 movement = {1, 0, 0};
        render->camera.position = sge_vec3_add(render->camera.position, movement);

        return SGE_SUCCESS;
}
SGE_RESULT sge_camera_move_left(sge_render *render) {
        vec3 movement = {0, -1, 0};
        render->camera.position = sge_vec3_add(render->camera.position, movement);

        return SGE_SUCCESS;
}
SGE_RESULT sge_camera_move_right(sge_render *render) {
        vec3 movement = {0, 1, 0};
        render->camera.position = sge_vec3_add(render->camera.position, movement);

        return SGE_SUCCESS;
}
SGE_RESULT sge_camera_move_backwards(sge_render *render) {
        vec3 movement = {-1, 0, 0};
        render->camera.position = sge_vec3_add(render->camera.position, movement);

        return SGE_SUCCESS;
}
SGE_RESULT sge_camera_rotate(sge_render *render);


SGE_RESULT sge_update_uniform_buffer(sge_render *render) {
        if (!render->sge_interface->update_uniform(render)) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}
