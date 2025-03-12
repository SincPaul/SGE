//
// Created by Geisthardt on 12.03.2025.
//

#include "sge_vulkan_uniform.h"

#include <stdio.h>

#include "../../core/memory_control.h"


SGE_RESULT sge_vulkan_update_uniform_buffer(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;
        sge_camera *camera = &render->camera;

        sge_uniform_buffer_object ubo;

        printf("Camera pos: x: %f, y: %f, z: %f\n", camera->position.x, camera->position.y, camera->position.z);


        m4 identity_matrix;
        sge_m4_set_identity(identity_matrix);
        copy_memory(ubo.model, identity_matrix, sizeof(m4), 0, 0);
        m4 view_matrix;
        sge_m4_set_identity(view_matrix);
        sge_m4_set_translate(view_matrix, camera->position);
        copy_memory(ubo.view, view_matrix, sizeof(m4), 0, 0);
        m4 projection_matrix;
        sge_m4_set_identity(projection_matrix);
        copy_memory(ubo.proj ,projection_matrix, sizeof(m4), 0, 0);

        void *data;
        vkMapMemory(vk_context->device, vk_context->uniform_buffer_memory, 0, sizeof(ubo), 0, &data);
        copy_memory(data, &ubo, sizeof(ubo), 0, 0);
        vkUnmapMemory(vk_context->device,vk_context->uniform_buffer_memory);

        printf("Updated view matrix: %f %f %f %f\n", ubo.view[0][0], ubo.view[0][1], ubo.view[0][2], ubo.view[0][3]);

        return SGE_SUCCESS;
}
