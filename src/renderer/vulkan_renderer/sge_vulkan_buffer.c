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

SGE_RESULT sge_vulkan_buffer_create(sge_render *render, void **buffer_ptr) {
        sge_vulkan_context *vk_context = render->api_context;

        VkBufferCreateInfo buffer_create_info = {0};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = NULL;
        buffer_create_info.flags = 0;
        buffer_create_info.size = sizeof(sge_uniform_buffer_object);
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &vk_context->graphics_queue_family_index;

        VkBuffer buffer;
        VkResult result = vkCreateBuffer(vk_context->device, &buffer_create_info, vk_context->sge_allocator, &buffer);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to create uniform buffer");
                return SGE_ERROR;
        }
        *buffer_ptr = (void *)buffer;
        return SGE_SUCCESS;
}


SGE_RESULT sge_vulkan_allocate_buffer(sge_render *render, void **memory_ptr, void *buffer) {
        sge_vulkan_context *vk_context = render->api_context;
        VkBuffer vk_buffer = buffer;

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(vk_context->device, vk_buffer, &mem_requirements);

        VkMemoryAllocateInfo alloc_info = {0};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(render,
                                                      mem_requirements.memoryTypeBits,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VkDeviceMemory memory;
        VkResult result = vkAllocateMemory(vk_context->device, &alloc_info, vk_context->sge_allocator, &memory);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate memory for uniform buffer");
                return SGE_ERROR;
        }

        vkBindBufferMemory(vk_context->device, vk_buffer, memory, 0);
        *memory_ptr = (void *)memory;
        return SGE_SUCCESS;
}
