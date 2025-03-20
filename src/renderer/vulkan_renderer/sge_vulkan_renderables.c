//
// Created by Geisthardt on 19.03.2025.
//

#include "sge_vulkan_renderables.h"

#include "../../core/logging.h"
#include "../../core/memory_control.h"

SGE_RESULT sge_vulkan_create_renderable_resources(sge_render *render, sge_renderable *renderable) {
        if (render == NULL ||renderable == NULL || renderable->mesh == NULL) {
                log_event(LOG_LEVEL_ERROR, "tried to creating renderable without initializing render or a valid renderable");
                return SGE_ERROR;
        }

        sge_vulkan_context *vk_context = render->api_context;
        sge_mesh *mesh = renderable->mesh;

        if (!mesh->vertex_buffer.data) {
                log_event(LOG_LEVEL_ERROR, "no vertex buiffer data passed for renderable creation");
                return SGE_ERROR;
        }

        VkBufferCreateInfo vertex_buffer_create_info = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = mesh->vertex_buffer.size,
                .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VkBuffer vertex_buffer;
        VkResult vertex_buffer_result = vkCreateBuffer(vk_context->device, &vertex_buffer_create_info, vk_context->sge_allocator, &vertex_buffer);
        if (vertex_buffer_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_ERROR, "Failed to create vertex buffer");
                return SGE_ERROR;
        }

        VkMemoryRequirements vertex_mem_reqs;
    vkGetBufferMemoryRequirements(vk_context->device, vertex_buffer, &vertex_mem_reqs);

    // Allocate memory
    VkMemoryAllocateInfo vertex_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = vertex_mem_reqs.size,
        .memoryTypeIndex = find_memory_type(render, vertex_mem_reqs.memoryTypeBits,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    VkDeviceMemory vertex_memory;
    if (vkAllocateMemory(vk_context->device, &vertex_alloc_info,
                        vk_context->sge_allocator, &vertex_memory) != VK_SUCCESS) {
        vkDestroyBuffer(vk_context->device, vertex_buffer, vk_context->sge_allocator);
        log_event(LOG_LEVEL_ERROR, "Failed to allocate vertex buffer memory");
        return SGE_ERROR;
    }

    // Bind buffer to memory
    if (vkBindBufferMemory(vk_context->device, vertex_buffer, vertex_memory, 0) != VK_SUCCESS) {
        vkFreeMemory(vk_context->device, vertex_memory, vk_context->sge_allocator);
        vkDestroyBuffer(vk_context->device, vertex_buffer, vk_context->sge_allocator);
        log_event(LOG_LEVEL_ERROR, "Failed to bind vertex buffer memory");
        return SGE_ERROR;
    }

    // Map memory and copy data
    void *mapped_data;
    if (vkMapMemory(vk_context->device, vertex_memory, 0,
                  mesh->vertex_buffer.size, 0, &mapped_data) != VK_SUCCESS) {
        vkFreeMemory(vk_context->device, vertex_memory, vk_context->sge_allocator);
        vkDestroyBuffer(vk_context->device, vertex_buffer, vk_context->sge_allocator);
        log_event(LOG_LEVEL_ERROR, "Failed to map vertex buffer memory");
        return SGE_ERROR;
    }

    // Copy data
    memcpy(mapped_data, mesh->vertex_buffer.data, mesh->vertex_buffer.size);

    // Unmap
    vkUnmapMemory(vk_context->device, vertex_memory);

    // Store the VkBuffer in the api_handle field
    mesh->vertex_buffer.api_handle = vertex_buffer;


        return SGE_SUCCESS;
}
