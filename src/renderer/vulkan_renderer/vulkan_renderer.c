//
// Created by Geisthardt on 07.03.2025.
//

#include "vulkan_renderer.h"



#include "../../core/logging.h"


sge_renderer_interface sge_vulkan_interface = {
        .begin_frame = sge_vulkan_begin_frame,
        .draw = sge_vulkan_draw_frame,
        .end_frame = sge_vulkan_end_frame,
        .initialize = sge_vulkan_initialize,
        .shutdown = sge_vulkan_shutdown,
        .set_pipeline = sge_set_pipeline,
        .update_uniform = sge_vulkan_update_uniform_buffer,
        .create_buffer = sge_vulkan_buffer_create,
        .allocate_buffer_memory = sge_vulkan_allocate_buffer,
        .create_descriptor_pool = sge_vulkan_descriptor_pool_create,
        .allocate_descriptor_set = sge_vulkan_descriptor_set,
        .update_descriptor_set = sge_vulkan_descriptor_update,
        .create_renderable_resources = sge_vulkan_create_renderable_resources
};




SGE_RESULT sge_vulkan_initialize(sge_render *render, sge_render_settings *render_settings) {
        log_event(LOG_LEVEL_INFO, "STARTING VULKAN INITIALIZATION");
        SGE_RESULT allocator_result = sge_vulkan_allocator_create(render, render_settings);
        if (allocator_result != SGE_SUCCESS) {
                return allocator_result;
        }

        SGE_RESULT instance_creation_result = sge_vulkan_instance_create(render, render_settings);
        if (instance_creation_result != SGE_SUCCESS) {
                return instance_creation_result;
        }
        log_event(LOG_LEVEL_INFO, "created vulkan instance");

        SGE_RESULT surface_creation_result = sge_vulkan_surface_create(render);
        if (surface_creation_result != SGE_SUCCESS) {
                return surface_creation_result;
        }
        log_event(LOG_LEVEL_INFO, "created surface");

        SGE_RESULT physical_device_result = sge_vulkan_physical_device_select(render);
        if (physical_device_result != SGE_SUCCESS) {
                return physical_device_result;
        }
        log_event(LOG_LEVEL_INFO, "selected physical device");

        SGE_RESULT logical_device_result = sge_vulkan_logical_device_create(render);
        if (logical_device_result != SGE_SUCCESS) {
                return logical_device_result;
        }
        log_event(LOG_LEVEL_INFO, "created logical device");

        SGE_RESULT swapchain_result = sge_vulkan_swapchain_create(render);
        if (swapchain_result != SGE_SUCCESS) {
                return swapchain_result;
        }
        log_event(LOG_LEVEL_INFO, "created swapchain");

        SGE_RESULT command_pool_result = sge_vulkan_command_pool_create(render);
        if (command_pool_result != SGE_SUCCESS) {
                return command_pool_result;
        }
        log_event(LOG_LEVEL_INFO, "craeted command pool");

        SGE_RESULT command_buffer_result = sge_vulkan_command_buffers_allocate(render);
        if (command_buffer_result != SGE_SUCCESS) {
                return command_buffer_result;
        }
        log_event(LOG_LEVEL_INFO, "created command buffers");

        if (!render_settings->vulkan.use_dynamic_rendering) {
                //render pass
                //framebuffer
                log_event(LOG_LEVEL_FATAL, "ENABLE DYNAMIC RENDERING");
        }

        SGE_RESULT sync_objects_result = sge_vulkan_sync_objects_create(render);
        if (sync_objects_result != SGE_SUCCESS) {
                return sync_objects_result;
        }
        log_event(LOG_LEVEL_INFO, "created sync objects");

        SGE_RESULT descriptor_pool_result = sge_vulkan_descriptor_pool_create_old(render);
        if (descriptor_pool_result != SGE_SUCCESS) {
                return descriptor_pool_result;
        }
        log_event(LOG_LEVEL_INFO, "created descriptor pool");

        SGE_RESULT uniform_descriptor_result = sge_vulkan_uniform_descriptor_create(render);
        if (uniform_descriptor_result != SGE_SUCCESS) {
                return uniform_descriptor_result;
        }
        log_event(LOG_LEVEL_INFO, "created uniform descriptors");

        SGE_RESULT pipeline_result = sge_vulkan_pipeline_create(render);
        if (pipeline_result != SGE_SUCCESS) {
                return pipeline_result;
        }
        log_event(LOG_LEVEL_INFO, "created vulkan pipeline");



        log_event(LOG_LEVEL_INFO, "FINISHED VULKAN INITIALIZATION");




        return SGE_SUCCESS;
}
SGE_RESULT sge_vulkan_shutdown(sge_render *render) {
        return SGE_SUCCESS;
}

//SGE_RESULT sge_vulkan_begin_frame(sge_render *render) {
//
//        return SGE_SUCCESS;
//}

//SGE_RESULT sge_vulkan_draw_frame(sge_render *render, sge_mesh *mesh) {
//        return SGE_SUCCESS;
//}
//SGE_RESULT sge_vulkan_end_frame(sge_render *render) {
//        return SGE_SUCCESS;
//}
SGE_RESULT sge_set_pipeline(sge_render *render, sge_pipeline *pipeline) {
        return SGE_SUCCESS;
}