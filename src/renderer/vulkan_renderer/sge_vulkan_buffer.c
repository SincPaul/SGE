//
// Created by Geisthardt on 12.03.2025.
//

#include "sge_vulkan_buffer.h"

#include "../../core/logging.h"
#include "../../core/memory_control.h"


SGE_RESULT sge_vulkan_uniform_buffer_create(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;

        for (int i = 0; i < 3; ++i) {
                VkBufferCreateInfo ubo_buffer_create_info = {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                        .pNext = NULL,
                        .flags = 0,
                        .size = sizeof(sge_uniform_buffer_object),
                        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 1,
                        .pQueueFamilyIndices = &vk_context->graphics_queue_family_index,
                };

                VkResult ubo_buffer_create_result = vkCreateBuffer(vk_context->device, &ubo_buffer_create_info, vk_context->sge_allocator, &vk_context->uniform_buffer[i]);
                if (ubo_buffer_create_result != VK_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed creating uniform buffer");
                        return SGE_ERROR;
                }

                VkMemoryRequirements memory_requirements;
                vkGetBufferMemoryRequirements(vk_context->device, vk_context->uniform_buffer[i], &memory_requirements);

                VkMemoryAllocateInfo allocate_info = {
                        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                        .pNext = NULL,
                        .allocationSize = memory_requirements.size,
                        .memoryTypeIndex = find_memory_type(render, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                };

                VkResult alloc_result = vkAllocateMemory(vk_context->device, &allocate_info, vk_context->sge_allocator, &vk_context->uniform_buffer_memory[i]);
                if (alloc_result != VK_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed allocating memory to uniform buffer");
                        return SGE_ERROR;
                }

                vkBindBufferMemory(vk_context->device, vk_context->uniform_buffer[i], vk_context->uniform_buffer_memory[i], 0);

        }


        return SGE_SUCCESS;
}
