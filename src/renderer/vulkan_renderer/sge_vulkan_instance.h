//
// Created by Geisthardt on 07.03.2025.
//

#ifndef SGE_VULKAN_INSTANCE_H
#define SGE_VULKAN_INSTANCE_H

#include "../sge_render.h"

SGE_RESULT sge_vulkan_instance_create(sge_render *render, sge_render_settings *render_settings);

SGE_RESULT sge_vulkan_instance_destroy(sge_render *render);

#endif //SGE_VULKAN_INSTANCE_H
