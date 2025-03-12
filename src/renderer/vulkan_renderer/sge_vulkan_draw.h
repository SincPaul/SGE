//
// Created by Geisthardt on 10.03.2025.
//

#ifndef SGE_VULKAN_DRAW_H
#define SGE_VULKAN_DRAW_H

#include "../sge_render.h"

SGE_RESULT sge_vulkan_draw_frame(sge_render *render);
SGE_RESULT sge_vulkan_begin_frame(sge_render *render);
SGE_RESULT sge_vulkan_end_frame(sge_render *render);
#endif //SGE_VULKAN_DRAW_H
