//
// Created by Geisthardt on 10.03.2025.
//

#include "sge_vulkan_shader.h"
#include "../../core/memory_control.h"
#include "../../core/logging.h"
#include "../../core/os_specific/os_utils.h"
#include "../../utils/steinfile.h"

#include <stdio.h>

char* get_shader_path(const char* shader_name) {
        char *base_dir = get_cwd();
        if (base_dir == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to get base dir");
        }

        char *shader_path = allocate_memory(512, MEMORY_TAG_INPUT);
        if (!shader_path) {
                log_event(LOG_LEVEL_FATAL, "failed to allocated memory");
        }

        sprintf(shader_path, "%s/../src/renderer/shaders/%s", base_dir, shader_name);
        free_memory(base_dir, MEMORY_TAG_INPUT);
        return shader_path;
}

VkShaderModule sge_vulkan_shader_load(sge_render *render, const char *shader_name) {
        sge_vulkan_context *vk_context = render->api_context;
        char *full_filepath = get_shader_path(shader_name);
        size_t codeSize;
        uint32_t *code = read_file_as_binary(full_filepath, &codeSize);

        free_memory(full_filepath, MEMORY_TAG_INPUT);

        VkShaderModuleCreateInfo createInfo = {0};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = codeSize;
        createInfo.pCode    = code;

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(vk_context->device, &createInfo, vk_context->sge_allocator, &shaderModule) != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create shader module from file: %s", full_filepath);
                return NULL;
        }

        free_memory(code, MEMORY_TAG_INPUT);
        return shaderModule;
}

