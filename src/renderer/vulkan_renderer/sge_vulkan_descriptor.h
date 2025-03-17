//
// Created by Geisthardt on 12.03.2025.
//

#ifndef SGE_VULKAN_DESCRIPTOR_H
#define SGE_VULKAN_DESCRIPTOR_H

#include "../sge_render.h"


SGE_RESULT sge_vulkan_descriptor_pool_create_old(sge_render *render);
SGE_RESULT sge_vulkan_descriptor_pool_create(sge_render *render, void *pool_ptr);
SGE_RESULT sge_vulkan_descriptor_set(sge_render *render, void *descriptor_ptr, void *layout_ptr, void *descriptor_pool);
SGE_RESULT sge_vulkan_uniform_descriptor_create(sge_render *render);
SGE_RESULT sge_vulkan_descriptor_update(sge_render *render, sge_uniform_buffer_type *buffer);

#endif //SGE_VULKAN_DESCRIPTOR_H
