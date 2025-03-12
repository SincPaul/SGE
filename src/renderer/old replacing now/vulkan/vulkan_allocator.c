//
// Created by Geisthardt on 04.03.2025.
//

#include "vulkan_renderer.h"
#include "../../core/logging.h"
#include "../../core/memory_control.h"


//The type of pfnAllocation is:
//// Provided by VK_VERSION_1_0
//typedef void* (VKAPI_PTR *PFN_vkAllocationFunction)(
//void* pUserData,
//size_t size,
//size_t alignment,
//VkSystemAllocationScope allocationScope);

void* VKAPI_CALL sge_allocator(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        return allocate_memory(size, MEMORY_TAG_VULKAN);
}

void VKAPI_CALL sge_free(void *pUserData, void *pMemory) {
        return free_memory(pMemory, MEMORY_TAG_VULKAN);
}

void* VKAPI_CALL sge_reallocator(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        return reallocate_memory(pOriginal, size, MEMORY_TAG_VULKAN);
}


bool get_sge_allocator(vulkan_context *vk_context) {
        VkAllocationCallbacks *callbacks = allocate_memory(sizeof(VkAllocationCallbacks), MEMORY_TAG_VULKAN);
        if (callbacks == NULL) { log_event(LOG_LEVEL_FATAL, "Failed to allocate memory for sge allocator"); return false; }

        callbacks->pUserData = NULL;
        callbacks->pfnAllocation = sge_allocator;
        callbacks->pfnFree = sge_free;
        callbacks->pfnReallocation = sge_reallocator;
        callbacks->pfnInternalAllocation = NULL;
        callbacks->pfnInternalFree = NULL;

        vk_context->sge_allocator = callbacks;

        return true;
}
