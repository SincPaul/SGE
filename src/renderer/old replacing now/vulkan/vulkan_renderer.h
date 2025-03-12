//
// Created by Geisthardt on 03.03.2025.
//

#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <stdint.h>
#include <windows.h>
#include <vulkan/vulkan_core.h>

#include "../../core/os_specific/sge_window.h"

typedef struct swapchain_infos {
        VkImage                         *sc_images;
        VkImageView                     *sc_views;
        uint32_t                        sc_img_count;

        VkSurfaceFormatKHR              surface;
        VkSurfaceCapabilitiesKHR        surface_capabilities;
        VkFramebuffer                   framebuffer;
} swapchain_infos;

typedef enum sge_render_mode {
        SGE_RENDER_EASY,
        SGE_RENDER_ADVANCED
} sge_render_mode;



typedef struct sync_objects {
        VkSemaphore                     image_available_semaphore[3], render_finished_semaphore[3];
        VkFence                         in_flight_fence[3];

        uint32_t                        current_frame;
} sync_objects;

typedef struct vulkan_context {
        VkInstance                      instance;
        VkSurfaceKHR                    surface;
        VkPhysicalDevice                physical_device;
        VkDevice                        device;
        VkSwapchainKHR                  swapchain;

        swapchain_infos                 sc;

        sync_objects                    so;

        VkCommandPool                   command_pool;
        VkCommandBuffer                 command_buffer[3];
        uint32_t                        command_buffer_index;
        VkRenderPass                    render_pass;

        VkQueue                         graphics_queue;
        VkQueue                         present_queue;
        VkQueue                         transfer_queue;

        VkPipeline                      pipeline;

        VkAllocationCallbacks           *sge_allocator;


} vulkan_context;

int initialize_vk(vulkan_context *vk_context ,sge_window *window, const char *application_name, uint32_t application_version);

bool create_vk_instance(const char *application_name, uint32_t application_version, vulkan_context *vk_context);

bool create_surface(sge_window *window, vulkan_context *vk_context);

bool select_physical_device(vulkan_context *vk_context);

VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t *graphics_queue_index, vulkan_context *vk_context);

VkQueue get_graphics_queue(VkDevice device, uint32_t graphics_family_queue_index);

VkQueue get_present_queue(VkDevice device, uint32_t graphics_family_queue_index);

VkQueue get_transfer_queue(VkDevice device, uint32_t graphics_family_queue_index);

VkSwapchainKHR create_swapchain(VkDevice device, VkSurfaceKHR surface, VkPhysicalDevice physical_device, vulkan_context *vk_context);

VkRenderPass create_render_pass(VkDevice device, vulkan_context *vk_context);

int initialize_vk_instance(HWND hwnd, const char *application_name, uint32_t application_version);

bool get_sge_allocator(vulkan_context *vk_context);

VkCommandPool create_command_pool(VkDevice device, uint32_t graphics_queue_family_index, vulkan_context *vk_context);

bool allocate_command_pools(VkDevice device, VkCommandPool command_pool, vulkan_context *vk_context);

VkFramebuffer create_framebuffer(VkDevice device, vulkan_context *vk_context);

void create_sync_objects(VkDevice device, vulkan_context *vk_context);

VkPipeline create_graphics_pipeline(VkDevice device, vulkan_context *vk_context);

VkSemaphore create_semaphore(VkDevice device, vulkan_context *vk_context);

VkFence create_fence(VkDevice device, vulkan_context *vk_context);

bool begin_command_buffer_rec(VkCommandBuffer command_buffer, vulkan_context *vk_context);

bool submit_command_buffer_queue(VkCommandBuffer command_buffer,VkQueue queue, vulkan_context *vk_context);

bool end_command_buffer_rec(VkCommandBuffer command_buffer, vulkan_context *vk_context);

bool draw_frame(vulkan_context *vk_context);

bool handle_resize_out_of_date(vulkan_context *vk_context);

bool destroy_swapchain(VkDevice device, VkSwapchainKHR swapchain, vulkan_context *vk_context);

bool shutdown_vulkan(vulkan_context *vk_context); //todo

#endif //VULKAN_RENDERER_H
