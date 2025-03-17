//
// Created by Geisthardt on 12.03.2025.
//

#ifndef SGE_VULKAN_BUFFER_H
#define SGE_VULKAN_BUFFER_H

#include "../sge_render.h"

SGE_RESULT sge_vulkan_uniform_buffer_create(sge_render *render);
SGE_RESULT sge_vulkan_buffer_create(sge_render *render, void **buffer_ptr);
SGE_RESULT sge_vulkan_allocate_buffer(sge_render *render, void **memory_ptr, void *buffer);


#endif //SGE_VULKAN_BUFFER_H
