//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_sync.h"
#include "../../core/memory_control.h"

SGE_RESULT sge_vulkan_sync_objects_create(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context *)render->api_context;

        vk_context->so.max_frames_in_flight = 3;
        uint32_t max_frames_in_flight = vk_context->so.max_frames_in_flight;

        vk_context->so.image_available_semaphores = allocate_memory(max_frames_in_flight * sizeof(VkSemaphore), MEMORY_TAG_VULKAN);
        vk_context->so.render_finished_semaphores = allocate_memory(max_frames_in_flight * sizeof(VkSemaphore), MEMORY_TAG_VULKAN);
        vk_context->so.in_flight_fences = allocate_memory(max_frames_in_flight * sizeof(VkFence), MEMORY_TAG_VULKAN);

        if (!vk_context->so.image_available_semaphores || !vk_context->so.render_finished_semaphores || !vk_context->so.in_flight_fences) {
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        for (uint32_t i = 0; i < max_frames_in_flight; i++) {
                VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

                if (vkCreateSemaphore(vk_context->device, &semaphore_info, vk_context->sge_allocator, &vk_context->so.image_available_semaphores[i]) != VK_SUCCESS ||
                        vkCreateSemaphore(vk_context->device, &semaphore_info, vk_context->sge_allocator, &vk_context->so.render_finished_semaphores[i]) != VK_SUCCESS ||
                        vkCreateFence(vk_context->device, &fence_info, vk_context->sge_allocator, &vk_context->so.in_flight_fences[i]) != VK_SUCCESS) {
                                return SGE_ERROR;
                        }
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_sync_objects_destroy(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)(render->api_context);

        for (uint32_t i = 0; i < vk_context->so.max_frames_in_flight; i++) {
                if (vk_context->so.image_available_semaphores[i] != VK_NULL_HANDLE) {
                        vkDestroySemaphore(vk_context->device, vk_context->so.image_available_semaphores[i], vk_context->sge_allocator);
                }
                if (vk_context->so.render_finished_semaphores[i] != VK_NULL_HANDLE) {
                        vkDestroySemaphore(vk_context->device, vk_context->so.render_finished_semaphores[i], vk_context->sge_allocator);
                }
                if (vk_context->so.in_flight_fences[i] != VK_NULL_HANDLE) {
                        vkDestroyFence(vk_context->device, vk_context->so.in_flight_fences[i], vk_context->sge_allocator);
                }
        }
        free_memory(vk_context->so.image_available_semaphores, MEMORY_TAG_VULKAN);
        free_memory(vk_context->so.render_finished_semaphores, MEMORY_TAG_VULKAN);
        free_memory(vk_context->so.in_flight_fences, MEMORY_TAG_VULKAN);

        vk_context->so.image_available_semaphores = NULL;
        vk_context->so.render_finished_semaphores = NULL;
        vk_context->so.in_flight_fences = NULL;
        vk_context->so.max_frames_in_flight = 0;
        return SGE_SUCCESS;
}
