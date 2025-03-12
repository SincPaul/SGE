//
// Created by Geisthardt on 07.03.2025.
//

#ifndef SGE_VULKAN_DEVICE_H
#define SGE_VULKAN_DEVICE_H

#include "../sge_render.h"

SGE_RESULT sge_vulkan_physical_device_select(sge_render *render);
SGE_RESULT sge_vulkan_physical_device_remove(sge_render *render);

SGE_RESULT sge_vulkan_logical_device_create(sge_render *render);
SGE_RESULT sge_vulkan_logical_device_destroy(sge_render *render);

#endif //SGE_VULKAN_DEVICE_H
