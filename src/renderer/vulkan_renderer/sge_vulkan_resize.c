//
// Created by Geisthardt on 10.03.2025.
//

#include "sge_vulkan_resize.h"

#include "../../core/logging.h"

SGE_RESULT handle_resize_out_of_date(sge_render *render) {
        if (sge_vulkan_swapchain_destroy(render) != SGE_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed destroying swapchain");
                return SGE_ERROR;
        }
        if (sge_vulkan_swapchain_create(render) != SGE_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed creating swapchain");
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}
