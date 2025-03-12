//
// Created by Geisthardt on 12.03.2025.
//

#include "sge_vulkan_uniform.h"

#include <math.h>
#include <stdio.h>

#include "../../core/memory_control.h"
void sge_m4_set_perspective(m4 mat, float fov_y, float aspect, float near, float far);

SGE_RESULT sge_vulkan_update_uniform_buffer(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;
        sge_camera *camera = &render->camera;

        sge_uniform_buffer_object ubo;

        printf("Camera pos: x: %f, y: %f, z: %f\n", camera->position.x, camera->position.y, camera->position.z);


        sge_m4_set_identity(ubo.model);
        m4 view_matrix;
        sge_m4_set_identity(view_matrix);
        vec3 negative_cam_pos = {
                -camera->position.x,
                -camera->position.y,
                -camera->position.z
        };
        sge_m4_set_translate(view_matrix, negative_cam_pos);
        //sge_m4_print(view_matrix);
        sge_m4_transpose(view_matrix);
        //sge_m4_print(view_matrix);
        copy_memory(ubo.view, view_matrix, sizeof(m4), 0, 0);
        sge_m4_set_perspective(ubo.proj, 45.0f * 3.14159f / 180.0f, (float)vk_context->sc.surface_capabilities.currentExtent.width / vk_context->sc.surface_capabilities.currentExtent.height, 0.1f, 100.0f);
        sge_m4_transpose(ubo.proj);
        printf("PROJECTION MATRIX\n");
        sge_m4_print(ubo.proj);
        printf("VIEW MATRIX\n");
        sge_m4_print(ubo.view);
        printf("MODEL MATRIX\n");
        sge_m4_print(ubo.model);
        void *data;
        vkMapMemory(vk_context->device, vk_context->uniform_buffer_memory[vk_context->so.current_frame], 0, sizeof(ubo), 0, &data);
        copy_memory(data, &ubo, sizeof(ubo), 0, 0);
        vkUnmapMemory(vk_context->device,vk_context->uniform_buffer_memory[vk_context->so.current_frame]);

        printf("Updated view matrix: %f %f %f %f\n", ubo.view[0][0], ubo.view[0][1], ubo.view[0][2], ubo.view[0][3]);


        return SGE_SUCCESS;
}
void sge_m4_set_perspective(m4 mat, float fov_y, float aspect, float near_f, float far_f) {
        float tan_half_fovy = tanf(fov_y / 2.0f);
        sge_m4_set_identity(mat);
        mat[0][0] = 1.0f / (aspect * tan_half_fovy);
        mat[1][1] = 1.0f / tan_half_fovy;
        mat[2][2] = -(far_f + near_f) / (far_f - near_f);
        mat[2][3] = -(2.0f * far_f * near_f) / (far_f - near_f);
        mat[3][2] = -1.0f;
        mat[3][3] = 0.0f;
}