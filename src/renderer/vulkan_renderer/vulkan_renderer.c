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
        .update_uniform = sge_vulkan_update_uniform_buffer
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

        SGE_RESULT surface_creation_result = sge_vulkan_surface_create(render);
        if (surface_creation_result != SGE_SUCCESS) {
                return surface_creation_result;
        }

        SGE_RESULT physical_device_result = sge_vulkan_physical_device_select(render);
        if (physical_device_result != SGE_SUCCESS) {
                return physical_device_result;
        }

        SGE_RESULT logical_device_result = sge_vulkan_logical_device_create(render);
        if (logical_device_result != SGE_SUCCESS) {
                return logical_device_result;
        }

        SGE_RESULT swapchain_result = sge_vulkan_swapchain_create(render);
        if (swapchain_result != SGE_SUCCESS) {
                return swapchain_result;
        }

        SGE_RESULT command_pool_result = sge_vulkan_command_pool_create(render);
        if (command_pool_result != SGE_SUCCESS) {
                return command_pool_result;
        }

        SGE_RESULT command_buffer_result = sge_vulkan_command_buffers_allocate(render);
        if (command_buffer_result != SGE_SUCCESS) {
                return command_buffer_result;
        }

        if (!render_settings->vulkan.use_dynamic_rendering) {
                //render pass
                //framebuffer
        }

        SGE_RESULT sync_objects_result = sge_vulkan_sync_objects_create(render);
        if (sync_objects_result != SGE_SUCCESS) {
                return sync_objects_result;
        }

        SGE_RESULT descriptor_pool_result = sge_vulkan_descriptor_pool_create(render);
        if (descriptor_pool_result != SGE_SUCCESS) {
                return descriptor_pool_result;
        }

        SGE_RESULT uniform_descriptor_result = sge_vulkan_uniform_descriptor_create(render);
        if (uniform_descriptor_result != SGE_SUCCESS) {
                return uniform_descriptor_result;
        }

        SGE_RESULT pipeline_result = sge_vulkan_pipeline_create(render);
        if (pipeline_result != SGE_SUCCESS) {
                return pipeline_result;
        }



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