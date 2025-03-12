//
// Created by Geisthardt on 07.03.2025.
//

#ifndef SGE_VULKAN_COMMAND_H
#define SGE_VULKAN_COMMAND_H

#include "../sge_render.h"

SGE_RESULT sge_vulkan_command_pool_create(sge_render *render);
SGE_RESULT sge_vulkan_command_buffers_allocate(sge_render *render);
SGE_RESULT sge_vulkan_command_pool_destroy(sge_render *render);
#endif //SGE_VULKAN_COMMAND_H
