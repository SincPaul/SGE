//
// Created by Geisthardt on 12.03.2025.
//

#include "sge_vulkan_descriptor.h"

#include "../../core/logging.h"
#include "../../core/memory_control.h"


SGE_RESULT sge_vulkan_descriptor_pool_create_old(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;

        VkDescriptorPoolSize pool_size = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
        };

        VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .maxSets = 3, //todo dynamic
                .poolSizeCount = 1,
                .pPoolSizes = &pool_size,
        };

        VkResult descriptor_pool_result = vkCreateDescriptorPool(vk_context->device, &descriptor_pool_create_info, vk_context->sge_allocator, &vk_context->descriptor_pool);
        if (descriptor_pool_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create descriptor pool");
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_uniform_descriptor_create(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;

        sge_vulkan_uniform_buffer_create(render);

        for (int i = 0; i < 3; ++i) {
                VkDescriptorSetLayoutBinding ubo_layout_binding = {
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                        .pImmutableSamplers = NULL,
                };

                VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
                        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                        .pNext = NULL,
                        .flags = 0,
                        .bindingCount = 1,
                        .pBindings = &ubo_layout_binding,
                };

                VkResult descriptor_layout_result = vkCreateDescriptorSetLayout(vk_context->device, &descriptor_set_layout_create_info, vk_context->sge_allocator, &vk_context->descriptor_set_layout);
                if (descriptor_layout_result != VK_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed creating descriptor layout");
                        return SGE_ERROR;
                }

                VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
                        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                        .pNext = NULL,
                        .descriptorPool = vk_context->descriptor_pool,
                        .descriptorSetCount = 1,
                        .pSetLayouts = &vk_context->descriptor_set_layout,
                };

                VkResult allocation_result = vkAllocateDescriptorSets(vk_context->device, &descriptor_set_allocate_info, &vk_context->descriptor_set[i]);
                if (allocation_result != VK_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed to allocate descriptor set");
                        return SGE_ERROR;
                }

                VkDescriptorBufferInfo buffer_info = {
                        .buffer = vk_context->uniform_buffer[i],
                        .offset = 0,
                        .range = sizeof(sge_uniform_buffer_object)
                };

                VkWriteDescriptorSet write_descriptor_set = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = NULL,
                        .dstSet = vk_context->descriptor_set[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = 0,
                        .pBufferInfo = &buffer_info,
                        .pTexelBufferView = 0,
                };

               vkUpdateDescriptorSets(vk_context->device, 1, &write_descriptor_set, 0, NULL);
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_descriptor_pool_create(sge_render *render, void *pool_ptr) {
        sge_vulkan_context *vk_context = render->api_context;

        VkDescriptorPoolSize pool_size = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 3, //todo dynamic
            };

        VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .maxSets = 3, // todo dynamic
                .poolSizeCount = 1,
                .pPoolSizes = &pool_size,
            };

        VkResult result = vkCreateDescriptorPool(vk_context->device, &descriptor_pool_create_info, vk_context->sge_allocator, pool_ptr);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create descriptor pool");
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}


SGE_RESULT sge_vulkan_descriptor_set(sge_render *render, void *descriptor_ptr, void *layout_ptr, void *descriptor_pool) {
        sge_vulkan_context *vk_context = render->api_context;

        VkDescriptorSetLayoutBinding ubo_layout_binding = {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL,
            };

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .bindingCount = 1,
                .pBindings = &ubo_layout_binding,
            };

        VkResult layout_result = vkCreateDescriptorSetLayout(vk_context->device, &descriptor_set_layout_create_info, vk_context->sge_allocator, layout_ptr);
        if (layout_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed creating descriptor layout");
                return SGE_ERROR;
        }

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = NULL,
                .descriptorPool = descriptor_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = layout_ptr,
            };

        VkResult allocation_result = vkAllocateDescriptorSets(vk_context->device, &descriptor_set_allocate_info, descriptor_ptr);
        if (allocation_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to allocate descriptor set");
                return SGE_ERROR;
        }


        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_descriptor_update(sge_render *render, sge_uniform_buffer_type *buffer) {
        sge_vulkan_context *vk_context = render->api_context;

        VkDescriptorBufferInfo buffer_info = {
                .buffer = buffer->buffer,
                .offset = 0,
                .range = sizeof(sge_uniform_buffer_object)
        };

        VkWriteDescriptorSet write_descriptor_set = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = buffer->descriptor,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = 0,
                .pBufferInfo = &buffer_info,
                .pTexelBufferView = 0,
        };

        vkUpdateDescriptorSets(vk_context->device, 1, &write_descriptor_set, 0, NULL);


        return SGE_SUCCESS;
}