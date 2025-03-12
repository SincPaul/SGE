//
// Created by Geisthardt on 07.03.2025.
//

#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <stdint.h>


typedef struct vulkan_app_info {
        char            *application_name;
        uint32_t        application_version;
} vulkan_app_info;

#include <vulkan/vulkan_core.h>
#include "../sge_render.h"
#include "sge_vulkan_instance.h"
#include "sge_vulkan_surface.h"
#include "sge_vulkan_device.h"
#include "sge_vulkan_swapchain.h"
#include "sge_vulkan_command.h"
#include "sge_vulkan_sync.h"
#include "sge_vulkan_allocator.h"
#include <vulkan/vulkan_win32.h>
#include "sge_vulkan_buffer.h"
#include "sge_vulkan_draw.h"
#include "sge_vulkan_pipeline.h"
#include "sge_vulkan_descriptor.h"
#include "sge_vulkan_uniform.h"


typedef struct swapchain_infos {
        VkImage                         *sc_images;
        VkImageView                     *sc_views;
        uint32_t                        sc_img_count;

        VkSurfaceFormatKHR              surface_format;
        VkSurfaceCapabilitiesKHR        surface_capabilities;
        VkFramebuffer                   framebuffer;
} swapchain_infos;

typedef enum sge_render_mode {
        SGE_RENDER_EASY,
        SGE_RENDER_ADVANCED
} sge_render_mode;



typedef struct sync_objects {
        VkSemaphore                     *image_available_semaphores, *render_finished_semaphores;
        VkFence                         *in_flight_fences;
        uint32_t                        current_frame;
        uint32_t                        max_frames_in_flight;
} sync_objects;

typedef struct sge_vulkan_context {
        VkInstance                      instance;
        VkSurfaceKHR                    surface;
        VkPhysicalDevice                physical_device;
        VkPhysicalDeviceProperties      physical_device_properties;
        VkDevice                        device;
        VkSwapchainKHR                  swapchain;

        swapchain_infos                 sc;

        sync_objects                    so;
        VkCommandPool                   command_pool;
        VkCommandBuffer                 *command_buffer;
        uint32_t                        command_buffer_count;
        uint32_t                        command_buffer_index;
        VkRenderPass                    render_pass;
        uint32_t                        graphics_queue_family_index;

        VkQueue                         graphics_queue;
        VkQueue                         present_queue;
        VkQueue                         transfer_queue;

        VkPipeline                      pipeline;
        VkPipelineLayout                pipeline_layout;

        VkAllocationCallbacks           *sge_allocator;

        VkDescriptorPool                descriptor_pool;
        VkDescriptorSet                 descriptor_set[3];
        VkDescriptorSetLayout           descriptor_set_layout;
        VkBuffer                        uniform_buffer[3];
        VkDeviceMemory                  uniform_buffer_memory[3];
} sge_vulkan_context;




SGE_RESULT sge_vulkan_initialize(sge_render *render, sge_render_settings *render_settings);
SGE_RESULT sge_vulkan_shutdown(sge_render *render);
//SGE_RESULT sge_vulkan_begin_frame(sge_render *render);
//SGE_RESULT sge_vulkan_draw_frame(sge_render *render, sge_mesh *mesh);
//SGE_RESULT sge_vulkan_end_frame(sge_render *render);
SGE_RESULT sge_set_pipeline(sge_render *render, sge_pipeline *pipeline);



extern sge_renderer_interface sge_vulkan_interface;

#endif //VULKAN_RENDERER_H
