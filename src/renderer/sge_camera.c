//
// Created by Geisthardt on 11.03.2025.
//

#include "sge_camera.h"

#include <stdio.h>

extern size_t g_delta_time;

SGE_RESULT sge_move(sge_render *render, sge_region *move_region, vec3 direction, sge_movement_settings movement_settings) {
        if (!g_delta_time) {
                g_delta_time = 1.0f;
        }
        if (render == NULL || move_region == NULL) {
                return SGE_ERROR;
        }

        uint32_t region_index = move_region->region_index;
        //printf("REGION INDEX: %i", region_index);


        float delta_time = g_delta_time / 1000.0f;
        if (delta_time > 0.05) {
                delta_time = 0.05;
        }
        //printf("%f\n", delta_time);
        vec3 movement = sge_vec3_scale(direction, movement_settings.delta_speed * delta_time);

        if (movement_settings.mode == ROTATE_PITCH_AND_YAW) {
                movement = sge_vec3_rotate_yaw_pitch(movement, render->regions[region_index]->camera->rotation);
        } else if (movement_settings.mode == ROTATE_PITCH_ONLY) {
                movement = sge_vec3_rotate_pitch(movement, render->regions[region_index]->camera->rotation);
        } else if (movement_settings.mode == ROTATE_YAW_ONLY) {
                movement = sge_vec3_rotate_yaw(movement, render->regions[region_index]->camera->rotation);
        }

        render->regions[region_index]->camera->position = sge_vec3_add(render->regions[region_index]->camera->position, movement);
        return SGE_SUCCESS;
}


SGE_RESULT sge_camera_move_forward(sge_render *render, sge_region *move_region, sge_movement_settings movement_settings) {
        const vec3 movement = {0, 0, -1};
        SGE_RESULT move_result = sge_move(render, move_region, movement, movement_settings);

        return move_result;
}
SGE_RESULT sge_camera_move_left(sge_render *render, sge_region *move_region, sge_movement_settings movement_settings) {
        vec3 movement = {-1.0, 0, 0};
        SGE_RESULT move_result = sge_move(render, move_region, movement, movement_settings);

        return move_result;
}
SGE_RESULT sge_camera_move_right(sge_render *render, sge_region *move_region, sge_movement_settings movement_settings) {
        vec3 movement = {1.0, 0, 0};
        SGE_RESULT move_result = sge_move(render, move_region, movement, movement_settings);

        return move_result;
}
SGE_RESULT sge_camera_move_backwards(sge_render *render, sge_region *move_region, sge_movement_settings movement_settings) {
        vec3 movement = {0, 0, 1};
        SGE_RESULT move_result = sge_move(render, move_region, movement, movement_settings);

        return move_result;
}
SGE_RESULT sge_camera_move_up(sge_render *render, sge_region *move_region, sge_movement_settings movement_settings) {
        vec3 movement = {0, -1.0, 0};
        SGE_RESULT move_result = sge_move(render, move_region, movement, movement_settings);

        return move_result;
}
SGE_RESULT sge_camera_move_down(sge_render *render, sge_region *move_region, sge_movement_settings movement_settings) {
        vec3 movement = {0, 1.0, 0};
        SGE_RESULT move_result = sge_move(render, move_region, movement, movement_settings);

        return move_result;
}


SGE_RESULT sge_camera_rotate(sge_render *render, sge_region *move_region, sge_mouse_movement_settings movement_settings) {

        if (movement_settings.mouse_delta_x == 0 && movement_settings.mouse_delta_y == 0) {
                return SGE_SUCCESS;
        }

        uint32_t region_index = move_region->region_index;

        if (movement_settings.flags & SGE_MOUSE_INVERT_Y) {
                movement_settings.mouse_delta_y = -movement_settings.mouse_delta_y;
        }
        if (movement_settings.flags & SGE_MOUSE_INVERT_X) {
                movement_settings.mouse_delta_x = -movement_settings.mouse_delta_x;
        }

        movement_settings.mouse_delta_y = -movement_settings.mouse_delta_y;


        if (!movement_settings.sensitivity) {
                movement_settings.sensitivity = 1;
        }

        //printf("MOUSE DELTA X: %f, Y: %f\n", movement_settings.mouse_delta_x, movement_settings.mouse_delta_y);
        movement_settings.mouse_delta_x *= movement_settings.sensitivity;
        movement_settings.mouse_delta_y *= movement_settings.sensitivity;

        float angle_x = movement_settings.mouse_delta_x / render->window->width * 360.0f;
        float angle_y = movement_settings.mouse_delta_y / render->window->height * 360.0f;

        //printf("ANGLE X: %f, ANGLE Y: %f\n", angle_x, angle_y);

        render->regions[region_index]->camera->rotation.y += angle_x;
        render->regions[region_index]->camera->rotation.x += angle_y;

        return SGE_SUCCESS;
}

SGE_RESULT sge_camera_rotate_x(sge_render *render, sge_region *move_region, float angle) {
        render->regions[move_region->region_index]->camera->rotation.x += angle;
        return SGE_SUCCESS;
}
SGE_RESULT sge_camera_rotate_y(sge_render *render, sge_region *move_region, float angle) {
        render->regions[move_region->region_index]->camera->rotation.y += angle;
        return SGE_SUCCESS;
}
SGE_RESULT sge_camera_rotate_z(sge_render *render, sge_region *move_region, float angle) {
        render->regions[move_region->region_index]->camera->rotation.z += angle;
        return SGE_SUCCESS;
}

SGE_RESULT sge_camera_lock_mouse(sge_render *render) {
        hide_mouse(render);
}

SGE_RESULT sge_camera_unlock_mouse(sge_render *render) {
        show_mouse(render);
}

SGE_RESULT sge_update_uniform_buffer(sge_render *render, sge_region *region) {
        if (!render->sge_interface->update_uniform(render, region)) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}
