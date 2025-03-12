//
// Created by Geisthardt on 10.03.2025.
//

#ifndef SGE_VULKAN_SHADER_H
#define SGE_VULKAN_SHADER_H

#include "../sge_render.h"

VkShaderModule sge_vulkan_shader_load(sge_render *render, const char *shader_name);

#endif //SGE_VULKAN_SHADER_H
