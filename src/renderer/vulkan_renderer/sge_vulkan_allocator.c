//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_allocator.h"
#include "../../core/memory_control.h"
#include "../../core/logging.h"


void* VKAPI_CALL sge_allocator(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        return allocate_memory(size, MEMORY_TAG_VULKAN);
}

void VKAPI_CALL sge_free(void *pUserData, void *pMemory) {
        return free_memory(pMemory, MEMORY_TAG_VULKAN);
}

void* VKAPI_CALL sge_reallocator(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        return reallocate_memory(pOriginal, size, MEMORY_TAG_VULKAN);
}


SGE_RESULT sge_vulkan_allocator_create(sge_render *render, sge_render_settings *render_settings) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)(render->api_context);

        if (!render_settings->vulkan.use_sge_allocator) {
                vk_context->sge_allocator = 0;
                return SGE_SUCCESS;
        }
        VkAllocationCallbacks *callbacks = allocate_memory(sizeof(VkAllocationCallbacks), MEMORY_TAG_VULKAN);
        if (callbacks == NULL) { log_event(LOG_LEVEL_FATAL, "Failed to allocate memory for sge allocator"); return false; }

        callbacks->pUserData = NULL;
        callbacks->pfnAllocation = sge_allocator;
        callbacks->pfnFree = sge_free;
        callbacks->pfnReallocation = sge_reallocator;
        callbacks->pfnInternalAllocation = NULL;
        callbacks->pfnInternalFree = NULL;

        vk_context->sge_allocator = callbacks;

        return SGE_SUCCESS;
}