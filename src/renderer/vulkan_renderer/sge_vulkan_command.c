//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_command.h"
#include "../../core/logging.h"
#include "../../core/memory_control.h"


SGE_RESULT sge_vulkan_command_pool_create(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context *)render->api_context;

        VkCommandPoolCreateInfo pool_info = {0};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = vk_context->graphics_queue_family_index;

        if (vkCreateCommandPool(vk_context->device, &pool_info, vk_context->sge_allocator, &vk_context->command_pool) != VK_SUCCESS) {
                return SGE_ERROR;
        }
        return SGE_SUCCESS;
}


SGE_RESULT sge_vulkan_command_buffers_allocate(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context *)render->api_context;

        vk_context->command_buffer_count = 3; //todo

        const uint32_t command_buffer_count = vk_context->command_buffer_count;

        VkCommandBufferAllocateInfo alloc_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = vk_context->command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = command_buffer_count,
                .pNext = 0,

        };

        vk_context->command_buffer = allocate_memory(sizeof(vk_context->command_buffer) * command_buffer_count, MEMORY_TAG_VULKAN);
        if (vk_context->command_buffer == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate command buffer");
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        VkResult allocate_result = vkAllocateCommandBuffers(vk_context->device, &alloc_info, vk_context->command_buffer);
        if (allocate_result != VK_SUCCESS) {
                free_memory(vk_context->command_buffer, MEMORY_TAG_VULKAN);
                return SGE_ERROR;
        }
        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_command_pool_destroy(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context *)render->api_context;

    if (vk_context->command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(vk_context->device, vk_context->command_pool, vk_context->sge_allocator);
        vk_context->command_pool = VK_NULL_HANDLE;
    }
    if (vk_context->command_buffer) {
        free_memory(vk_context->command_buffer, MEMORY_TAG_VULKAN);
        vk_context->command_buffer = NULL;
    }
    vk_context->command_buffer_count = 0;
    return SGE_SUCCESS;
}